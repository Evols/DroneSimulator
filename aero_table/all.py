from aero_table.airfoils.naca2412 import run_naca2412
from aero_table.airfoils.naca4410 import run_naca4410
from aero_table.utils.timer import timer

def main():
    run_naca2412()
    run_naca4410()

if __name__ == "__main__":
    with timer("All"):
        main()
