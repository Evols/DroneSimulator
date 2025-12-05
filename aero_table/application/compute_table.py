import os
import shutil
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor, as_completed

from aero_table.adapters.folder import FolderAdapter
from aero_table.application.csv import init_csv_table, write_csv_table_row
from aero_table.application.xfoil_pol_parser import pol_file_to_row, ComputedXfoilRow
from aero_table.application.xfoil_error_rating import pick_best_candidate
from aero_table.application.post_stall_processing import apply_viterna_to_all_rows
from aero_table.application.xfoil_row_fixer import try_to_fix_row
from aero_table.ports import XFoilFileGenerator
from aero_table.application.reynolds_range import ReynoldsRange
from aero_table.application.aoa_stepper import AoaStepper
from aero_table.adapters.xfoil_runner.run_process import run_xfoil


REYNOLDS_CANDIDATES_ABSOLUTE_COUNT = 10


def build_reynolds_candidates_for_reynolds_number(reynolds_number: float) -> list[float]:
    """
    Build the list of Reynolds numbers to try for a given Reynolds number.
    The list will be centered around the given Reynolds number and will have a length of REYNOLDS_CANDIDATES_ABSOLUTE_COUNT * 2 + 1.
    They are alternatively above and below the given Reynolds number, growing in absolute value.
    """

    if reynolds_number < REYNOLDS_CANDIDATES_ABSOLUTE_COUNT:
        raise ValueError("Reynolds number must be greater than REYNOLDS_CANDIDATES_ABSOLUTE_COUNT")
    if REYNOLDS_CANDIDATES_ABSOLUTE_COUNT <= 0:
        raise ValueError("REYNOLDS_CANDIDATES_ABSOLUTE_COUNT must be positive")

    candidates = []
    for offset in range(1, REYNOLDS_CANDIDATES_ABSOLUTE_COUNT + 1):
        candidates.append(reynolds_number + offset)
        candidates.append(reynolds_number - offset)

    return candidates


def compute_candidate(generator: XFoilFileGenerator, reynolds_number: float, xfoil_aoa_stepper: AoaStepper, output_folder_path: Path) -> ComputedXfoilRow | None:
    positive_file_name = f"{reynolds_number}_positive".replace(".", "_")
    positive_aoa_stepper = AoaStepper(0.0, xfoil_aoa_stepper.max, xfoil_aoa_stepper.step)
    pol_file_positive = run_xfoil(generator, reynolds_number, positive_aoa_stepper, output_folder_path,
                                  positive_file_name)
    if pol_file_positive is None:
        return None

    negative_file_name = f"{reynolds_number}_negative".replace(".", "_")
    negative_aoa_stepper = AoaStepper(0.0, xfoil_aoa_stepper.min, -xfoil_aoa_stepper.step)
    pol_file_negative = run_xfoil(generator, reynolds_number, negative_aoa_stepper, output_folder_path,
                                  negative_file_name)
    if pol_file_negative is None:
        return None

    positive_row = pol_file_to_row(pol_file_positive, positive_aoa_stepper)
    negative_row = pol_file_to_row(pol_file_negative, negative_aoa_stepper)

    final_row_values = negative_row.values_for_aoas[1:][::-1] + positive_row.values_for_aoas
    final_row_aoas = negative_row.angles_of_attack[1:][::-1] + positive_row.angles_of_attack

    return ComputedXfoilRow(
        reynolds_number=reynolds_number,
        angles_of_attack=final_row_aoas,
        values_for_aoas=final_row_values,
        xfoil_aoa_stepper=xfoil_aoa_stepper
    )


def _compute_candidate_wrapper(args: tuple) -> tuple[float, ComputedXfoilRow | None]:
    """
    Wrapper function for parallel execution of compute_candidate.
    Returns a tuple of (reynolds_number, result) to preserve the candidate Reynolds number.
    """
    generator, candidate_reynolds, xfoil_aoa_stepper, output_folder_path = args
    result = compute_candidate(generator, candidate_reynolds, xfoil_aoa_stepper, output_folder_path)
    return candidate_reynolds, result


def compute_best_candidate(generator: XFoilFileGenerator, reynolds_number: float, xfoil_aoa_stepper: AoaStepper, output_folder_path: Path) -> ComputedXfoilRow | None:
    reynolds_number = float(round(reynolds_number))
    candidate_reynolds = build_reynolds_candidates_for_reynolds_number(reynolds_number)

    candidate_rows: list[ComputedXfoilRow] = []

    # Prepare arguments for parallel execution
    # Each candidate computation is independent, so we can run them in parallel
    args_list = [(generator, candidate_reynolds_val, xfoil_aoa_stepper, output_folder_path) 
                 for candidate_reynolds_val in candidate_reynolds]

    # Use ProcessPoolExecutor to compute candidates in parallel
    # max_workers=None uses os.cpu_count() by default
    with ProcessPoolExecutor() as executor:
        # Submit all tasks
        future_to_reynolds = {
            executor.submit(_compute_candidate_wrapper, args): args[1] 
            for args in args_list
        }
        
        # Collect results as they complete
        for future in as_completed(future_to_reynolds):
            try:
                _, candidate_row = future.result()
                if candidate_row is not None:
                    candidate_rows.append(candidate_row)
            except Exception as e:
                reynolds = future_to_reynolds[future]
                print(f"Error computing candidate for Reynolds {reynolds}: {e}")

    best_candidate = pick_best_candidate(candidate_rows)
    if best_candidate is None:
        return None

    best_fixed_candidate = try_to_fix_row(best_candidate)
    return best_fixed_candidate


def extract_quality_info(row_fixed: ComputedXfoilRow) -> tuple[int, int]:
    aoa_values = row_fixed.angles_of_attack
    smallest_aoa = abs(aoa_values[0])
    smallest_aoa_idx = 0

    for aoa_idx, aoa in enumerate(aoa_values):
        aoa_abs = abs(aoa)
        if aoa_abs <= smallest_aoa:
            smallest_aoa = aoa_abs
            smallest_aoa_idx = aoa_idx

    negative_first_bad_row = smallest_aoa_idx
    while negative_first_bad_row >= 1:
        if row_fixed.values_for_aoas[negative_first_bad_row - 1] is None:
            break
        negative_first_bad_row -= 1

    positive_first_bad_row = smallest_aoa_idx
    while positive_first_bad_row < len(row_fixed.values_for_aoas) - 1:
        if row_fixed.values_for_aoas[positive_first_bad_row + 1] is None:
            break
        positive_first_bad_row += 1
    return negative_first_bad_row, positive_first_bad_row


def print_summary_statistics(all_rows: list[ComputedXfoilRow]):
    """Print summary statistics about data completeness between stall boundaries."""
    total_points = 0
    missing_points = 0
    
    # Only count points between negative and positive stall boundaries
    for row in all_rows:
        negative_stall_idx, positive_stall_idx = extract_quality_info(row)
        
        # Count points in the valid range (between stalls)
        for idx in range(negative_stall_idx, positive_stall_idx + 1):
            total_points += 1
            if row.values_for_aoas[idx] is None:
                missing_points += 1
    
    present_points = total_points - missing_points

    print(f"\n=== Final Summary (between stall boundaries) ===")
    print(f"Total data points: {total_points}")
    print(f"Present: {present_points} ({100*present_points/total_points:.1f}%)")
    print(f"Missing: {missing_points} ({100*missing_points/total_points:.1f}%)")


def compute_table(
    generator: XFoilFileGenerator, 
    reynolds_range: ReynoldsRange, 
    xfoil_aoa_stepper: AoaStepper, 
    folder_name: str,
    cd_max: float = 1.8
) -> list[ComputedXfoilRow]:
    output_folder_path = FolderAdapter().get_base_saved_path().joinpath(folder_name).resolve()
    shutil.rmtree(output_folder_path, ignore_errors=True)
    os.makedirs(output_folder_path)

    csv_path = output_folder_path / (folder_name + "_coefficients.csv")
    print(f"CSV will be written to:", csv_path)

    xfoil_rows = []

    for reynolds_number in reynolds_range.get_subvalues():
        reynolds_number = float(int(reynolds_number))

        print("Computing coefficients for Re =", reynolds_number)

        best_candidate = compute_best_candidate(generator, reynolds_number, xfoil_aoa_stepper, output_folder_path)

        if best_candidate is None:
            continue

        xfoil_rows.append(best_candidate)

    # Apply Viterna post-stall extrapolation
    print("\n" + "="*60)
    viterna_rows = apply_viterna_to_all_rows(xfoil_rows, cd_max=cd_max)
    print("="*60)

    init_csv_table(csv_path, viterna_rows[0].angles_of_attack)

    # Write CSV with Viterna-enhanced data
    for idx, row in enumerate(viterna_rows):
        write_csv_table_row(csv_path, row)

    # Print final summary
    print_summary_statistics(viterna_rows)
    print(f"\nFinal CSV report available at: {csv_path}")

    return viterna_rows
