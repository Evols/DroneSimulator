from aero_table.ports import XFoilFileGenerator
from aero_table.application.aoa_stepper import AoaStepper
from aero_table.application.reynolds_range import ReynoldsRange
from aero_table.application.compute_table import compute_table
from aero_table.utils.timer import timer


class Generator(XFoilFileGenerator):
    def generate_file_content(self, pol_file_name: str, re: float, aoa_stepper: AoaStepper) -> str:
        content = f"""
NACA 4410
PANE

PLOP
G

OPER
VISC {re}
VPAR
N 7
ITER 200

PACC
{pol_file_name}

ASEQ {aoa_stepper.min} {aoa_stepper.max} {aoa_stepper.step}
PACC

QUIT
"""
        return content


def run_naca4410():
    generator = Generator()

    reynolds_range = ReynoldsRange(min=10_000, max=10_000_000, num_subvalues_per_10=10)
    xfoil_aoa_stepper = AoaStepper(min=-15, max=25, step=0.2)

    compute_table(generator, reynolds_range, xfoil_aoa_stepper, "naca4410")


if __name__ == "__main__":
    with timer("NACA 4410"):
        run_naca4410()
