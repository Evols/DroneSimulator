"""
Post-stall processing using Viterna extrapolation.

This module replaces unreliable xfoil post-stall values with 
Viterna-extrapolated values and extends the AOA range to ±90 degrees
with coarser resolution in post-stall regions.
"""

from typing import Optional
import math

from aero_table.application.xfoil_pol_parser import ComputedXfoilRow, ComputedXfoilValue
from aero_table.application.viterna import (
    detect_stall_point, 
    apply_viterna_extrapolation,
    StallPoint
)


def generate_viterna_aoa_array(
    negative_stall_deg: float,
    positive_stall_deg: float,
    xfoil_aoas: list[float]
) -> list[float]:
    """
    Generate AOA array with variable resolution:
    - Keep xfoil resolution from min xfoil AOA to stall points
    - Use 1° resolution from stall to stall+30%
    - Use 4° resolution from stall+30% to ±90°
    
    Args:
        negative_stall_deg: Negative stall angle (e.g., -12°)
        positive_stall_deg: Positive stall angle (e.g., 15°)
        xfoil_aoas: Original xfoil AOA array
        
    Returns:
        Combined AOA array with variable resolution covering -90 to +90
    """
    aoa_set = set()
    
    # Add all xfoil AOAs within the stall boundaries
    for aoa in xfoil_aoas:
        if negative_stall_deg <= aoa <= positive_stall_deg:
            aoa_set.add(round(aoa, 6))  # Round to avoid floating point issues
    
    # Negative side post-stall extrapolation
    # From negative stall to stall - 30% of stall angle (1° steps)
    negative_transition_angle = negative_stall_deg - abs(negative_stall_deg) * 0.3
    
    aoa = negative_stall_deg - 1.0
    while aoa >= negative_transition_angle:
        aoa_set.add(round(aoa, 6))
        aoa -= 1.0
    
    # From transition to -90° (4° steps)
    aoa = math.floor(negative_transition_angle / 4) * 4  # Start at nearest multiple of 4 below transition
    while aoa >= -90:
        aoa_set.add(round(aoa, 6))
        aoa -= 4.0
    
    # Ensure we include -90
    aoa_set.add(-90.0)
    
    # Positive side post-stall extrapolation
    # From positive stall to stall + 30% of stall angle (1° steps)
    positive_transition_angle = positive_stall_deg + abs(positive_stall_deg) * 0.3
    
    aoa = positive_stall_deg + 1.0
    while aoa <= positive_transition_angle:
        aoa_set.add(round(aoa, 6))
        aoa += 1.0
    
    # From transition to +90° (4° steps)
    aoa = math.ceil(positive_transition_angle / 4) * 4  # Start at nearest multiple of 4 above transition
    while aoa <= 90:
        aoa_set.add(round(aoa, 6))
        aoa += 4.0
    
    # Ensure we include +90
    aoa_set.add(90.0)
    
    # Convert to sorted list
    result = sorted(list(aoa_set))
    
    return result


def apply_viterna_to_row(
    row: ComputedXfoilRow,
    cd_max: float = 1.8
) -> ComputedXfoilRow:
    """
    Apply Viterna extrapolation to replace and extend post-stall values.
    
    This function:
    1. Detects positive and negative stall points from xfoil data
    2. Generates new AOA array with variable resolution extending to ±90°
    3. Uses xfoil data where available (within stall boundaries)
    4. Applies Viterna extrapolation for post-stall regions
    
    Args:
        row: Row with xfoil data (uniform resolution from xfoil sweep)
        cd_max: Maximum drag coefficient for Viterna model
        
    Returns:
        Row with extended AOA range (-90 to +90) and variable resolution
    """
    # Use the xfoil AOA list
    xfoil_aoa_list = row.angles_of_attack
    
    # Detect stall points
    positive_stall = detect_stall_point(row.values_for_aoas, xfoil_aoa_list, search_positive=True)
    negative_stall = detect_stall_point(row.values_for_aoas, xfoil_aoa_list, search_positive=False)
    
    if positive_stall is None or negative_stall is None:
        print(f"  Warning: Could not detect stall points for Re={row.reynolds_number:.0f}")
        return row
    
    print(f"  Re={row.reynolds_number:.0f}: "
          f"Positive stall at {positive_stall.aoa_deg:.1f}°, "
          f"Negative stall at {negative_stall.aoa_deg:.1f}°")
    
    # Generate new AOA array with variable resolution (-90 to +90)
    viterna_extended_aoas = generate_viterna_aoa_array(
        negative_stall_deg=negative_stall.aoa_deg,
        positive_stall_deg=positive_stall.aoa_deg,
        xfoil_aoas=xfoil_aoa_list
    )
    
    print(f"  AOA points: {len(xfoil_aoa_list)} (xfoil) -> {len(viterna_extended_aoas)} (with Viterna to ±90°)")
    
    # Create xfoil data lookup dictionary for efficient access
    xfoil_data_map = {}
    for aoa, value in zip(xfoil_aoa_list, row.values_for_aoas):
        if value is not None:
            xfoil_data_map[round(aoa, 6)] = value
    
    # Create new values list with variable resolution
    new_values: list[Optional[ComputedXfoilValue]] = []
    
    for aoa in viterna_extended_aoas:
        aoa_key = round(aoa, 6)
        
        # Determine if we're post-stall
        is_post_stall = (aoa > positive_stall.aoa_deg) or (aoa < negative_stall.aoa_deg)
        
        if is_post_stall:
            # Use Viterna extrapolation
            viterna_value = apply_viterna_extrapolation(
                aoa_deg=aoa,
                positive_stall=positive_stall,
                negative_stall=negative_stall,
                cd_max=cd_max
            )
            new_values.append(viterna_value)
        else:
            # Try to use xfoil data if available
            if aoa_key in xfoil_data_map:
                new_values.append(xfoil_data_map[aoa_key])
            else:
                # This shouldn't happen often, but keep None if no data
                new_values.append(None)
    
    return ComputedXfoilRow(
        reynolds_number=row.reynolds_number,
        angles_of_attack=viterna_extended_aoas,  # Variable resolution AOA list
        values_for_aoas=new_values,
        xfoil_aoa_stepper=row.xfoil_aoa_stepper  # Keep reference to original xfoil config
    )


def count_viterna_replacements(original_row: ComputedXfoilRow, viterna_row: ComputedXfoilRow) -> tuple[int, int]:
    """
    Count how many values were replaced by Viterna.
    
    Returns:
        Tuple of (xfoil_values_replaced, none_values_filled)
    """
    xfoil_replaced = 0
    none_filled = 0
    
    for orig, vit in zip(original_row.values_for_aoas, viterna_row.values_for_aoas):
        # If original had xfoil data but now has different viterna data
        if orig is not None and vit is not None and orig != vit:
            xfoil_replaced += 1
        # If original was None but now has viterna data
        elif orig is None and vit is not None:
            none_filled += 1
    
    return xfoil_replaced, none_filled


def apply_viterna_to_all_rows(
    rows: list[ComputedXfoilRow],
    cd_max: float = 1.8
) -> list[ComputedXfoilRow]:
    """
    Apply Viterna extrapolation to all rows with unified AOA array.
    
    All Reynolds numbers will use the same AOA array for consistent CSV format.
    Uses the most conservative stall angles (widest range) across all Reynolds numbers.
    
    Args:
        rows: List of rows with xfoil data
        cd_max: Maximum drag coefficient
        
    Returns:
        List of rows with Viterna applied (all with same AOA array)
    """
    print("\n=== Applying Viterna Post-Stall Extrapolation ===")
    
    # First pass: detect all stall points to find the most conservative angles
    most_negative_stall = 0.0
    most_positive_stall = 0.0
    
    for row in rows:
        xfoil_aoa_list = row.angles_of_attack
        positive_stall = detect_stall_point(row.values_for_aoas, xfoil_aoa_list, search_positive=True)
        negative_stall = detect_stall_point(row.values_for_aoas, xfoil_aoa_list, search_positive=False)
        
        if positive_stall and negative_stall:
            # Track the most extreme stall angles (widest range needed)
            if negative_stall.aoa_deg < most_negative_stall:
                most_negative_stall = negative_stall.aoa_deg
            if positive_stall.aoa_deg > most_positive_stall:
                most_positive_stall = positive_stall.aoa_deg
    
    # Get any row's xfoil AOAs as reference (they should all be the same from xfoil)
    reference_xfoil_aoas = rows[0].angles_of_attack if rows else []
    
    # Generate unified AOA array using most conservative stall angles
    unified_aoa_array = generate_viterna_aoa_array(
        negative_stall_deg=most_negative_stall,
        positive_stall_deg=most_positive_stall,
        xfoil_aoas=reference_xfoil_aoas
    )
    
    print(f"  Unified AOA array: {len(unified_aoa_array)} points from -90° to +90°")
    print(f"  Using stall boundaries: {most_negative_stall:.1f}° to {most_positive_stall:.1f}°")
    
    # Second pass: apply Viterna with unified AOA array
    viterna_rows = []
    total_xfoil_replaced = 0
    total_none_filled = 0
    
    for row in rows:
        viterna_row = apply_viterna_to_row_with_unified_aoa(row, unified_aoa_array, cd_max)
        viterna_rows.append(viterna_row)
        
        xfoil_replaced, none_filled = count_viterna_replacements(row, viterna_row)
        total_xfoil_replaced += xfoil_replaced
        total_none_filled += none_filled
    
    print(f"\n=== Viterna Summary ===")
    print(f"Total xfoil post-stall values replaced: {total_xfoil_replaced}")
    print(f"Total missing values filled: {total_none_filled}")
    print(f"Total values affected: {total_xfoil_replaced + total_none_filled}")
    
    return viterna_rows


def apply_viterna_to_row_with_unified_aoa(
    row: ComputedXfoilRow,
    unified_aoa_array: list[float],
    cd_max: float = 1.8
) -> ComputedXfoilRow:
    """
    Apply Viterna extrapolation using a pre-computed unified AOA array.
    
    Args:
        row: Row with xfoil data
        unified_aoa_array: Pre-computed AOA array to use for all Reynolds numbers
        cd_max: Maximum drag coefficient
        
    Returns:
        Row with extended AOA range using the unified array
    """
    xfoil_aoa_list = row.angles_of_attack
    
    # Detect stall points for this specific Reynolds number
    positive_stall = detect_stall_point(row.values_for_aoas, xfoil_aoa_list, search_positive=True)
    negative_stall = detect_stall_point(row.values_for_aoas, xfoil_aoa_list, search_positive=False)
    
    if positive_stall is None or negative_stall is None:
        print(f"  Warning: Could not detect stall points for Re={row.reynolds_number:.0f}")
        # Return with unified AOA array but filled with Nones
        return ComputedXfoilRow(
            reynolds_number=row.reynolds_number,
            angles_of_attack=unified_aoa_array,
            values_for_aoas=[None] * len(unified_aoa_array),
            xfoil_aoa_stepper=row.xfoil_aoa_stepper
        )
    
    print(f"  Re={row.reynolds_number:.0f}: "
          f"Positive stall at {positive_stall.aoa_deg:.1f}°, "
          f"Negative stall at {negative_stall.aoa_deg:.1f}°")
    
    # Create xfoil data lookup dictionary
    xfoil_data_map = {}
    for aoa, value in zip(xfoil_aoa_list, row.values_for_aoas):
        if value is not None:
            xfoil_data_map[round(aoa, 6)] = value
    
    # Create new values list using unified AOA array
    new_values: list[Optional[ComputedXfoilValue]] = []
    
    for aoa in unified_aoa_array:
        aoa_key = round(aoa, 6)
        
        # Determine if we're post-stall for this specific Reynolds number
        is_post_stall = (aoa > positive_stall.aoa_deg) or (aoa < negative_stall.aoa_deg)
        
        if is_post_stall:
            # Use Viterna extrapolation
            viterna_value = apply_viterna_extrapolation(
                aoa_deg=aoa,
                positive_stall=positive_stall,
                negative_stall=negative_stall,
                cd_max=cd_max
            )
            new_values.append(viterna_value)
        else:
            # Try to use xfoil data if available
            if aoa_key in xfoil_data_map:
                new_values.append(xfoil_data_map[aoa_key])
            else:
                # Use Viterna even in pre-stall if we don't have xfoil data at this AOA
                viterna_value = apply_viterna_extrapolation(
                    aoa_deg=aoa,
                    positive_stall=positive_stall,
                    negative_stall=negative_stall,
                    cd_max=cd_max
                )
                new_values.append(viterna_value)
    
    return ComputedXfoilRow(
        reynolds_number=row.reynolds_number,
        angles_of_attack=unified_aoa_array,
        values_for_aoas=new_values,
        xfoil_aoa_stepper=row.xfoil_aoa_stepper
    )

