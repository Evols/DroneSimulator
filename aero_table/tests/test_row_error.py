import unittest
import os
import shutil

from aero_table.adapters.folder import FolderAdapter
from aero_table.application.aoa_stepper import AoaStepper
from aero_table.application.compute_table import compute_candidate
from aero_table.application.csv import init_csv_table, write_csv_table_row
from aero_table.application.xfoil_row_fixer import try_to_fix_row
from aero_table.ports import XFoilFileGenerator


class Naca2412Generator(XFoilFileGenerator):
    def generate_file_content(self, pol_file_name: str, reynolds_number: float, aoa_stepper: AoaStepper) -> str:
        content = f"""
NACA 2412
PANE

PLOP
G

OPER
VISC {reynolds_number}
VPAR
N 5
ITER 200

PACC
{pol_file_name}

ASEQ {aoa_stepper.min} {aoa_stepper.max} {aoa_stepper.step}
PACC

QUIT
"""
        return content


class TestRowEror(unittest.TestCase):
    def test_typical_stall_point(self):
        generator = Naca2412Generator()

        xfoil_aoa_stepper = AoaStepper(min=-20, max=50, step=0.5)

        folder_name = "test_row_error"
        output_folder_path = FolderAdapter().get_base_saved_path().joinpath(folder_name).resolve()
        shutil.rmtree(output_folder_path, ignore_errors=True)
        os.makedirs(output_folder_path)

        row = compute_candidate(generator, 1_000_001.0, xfoil_aoa_stepper, output_folder_path)
        for v in row.values_for_aoas:
            if v is None:
                continue
            v.lift_coefficient = 0.0 if v.lift_coefficient is None else v.lift_coefficient
            v.drag_coefficient = 0.0 if v.drag_coefficient is None else v.drag_coefficient
            v.moment_coefficient = 0.0 if v.moment_coefficient is None else v.moment_coefficient

        csv_path = output_folder_path / "test_data_points.csv"
        init_csv_table(csv_path, xfoil_aoa_stepper.get_all_angles_of_attack())
        write_csv_table_row(csv_path, row)

        row_fixed = try_to_fix_row(row)
        write_csv_table_row(csv_path, row_fixed)


if __name__ == '__main__':
    unittest.main()
