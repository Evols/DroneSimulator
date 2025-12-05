from aero_table.ports import XFoilFileGenerator
from aero_table.application.aoa_stepper import AoaStepper
from aero_table.application.reynolds_range import ReynoldsRange
from aero_table.application.compute_table import compute_table
from aero_table.utils.timer import timer


class Generator(XFoilFileGenerator):
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


def run_naca2412():
    generator = Generator()

    reynolds_range = ReynoldsRange(min=40_000.0, max=6_000_000.0, num_subvalues_per_10=10)
    xfoil_aoa_stepper = AoaStepper(min=-20, max=50, step=0.5)

    compute_table(generator, reynolds_range, xfoil_aoa_stepper, "naca2412")


if __name__ == "__main__":
    with timer("NACA 2412"):
        run_naca2412()
