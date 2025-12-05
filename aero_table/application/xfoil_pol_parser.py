from dataclasses import dataclass

from aero_table.application.aoa_stepper import AoaStepper
from aero_table.ports import XfoilPolFile


@dataclass
class ComputedXfoilValue:
    angle_of_attack: float
    lift_coefficient: float
    drag_coefficient: float
    pressure_drag_coefficient: float
    moment_coefficient: float
    top_transition: float
    bottom_transition: float


@dataclass
class ComputedXfoilRow:
    """
    Row of aerodynamic data at a specific Reynolds number.
    
    Can represent either:
    - Pure xfoil data (uniform resolution from xfoil sweep)
    - Viterna-extended data (variable resolution from -90° to +90°)
    
    The angles_of_attack list explicitly defines all AOA points.
    The xfoil_aoa_stepper tracks the original xfoil configuration.
    """
    reynolds_number: float
    angles_of_attack: list[float]  # Explicit AOA array (uniform or variable resolution)
    values_for_aoas: list[ComputedXfoilValue | None]
    xfoil_aoa_stepper: AoaStepper  # Original xfoil sweep configuration


def pol_file_to_row(pol_file: XfoilPolFile, xfoil_aoa_stepper: AoaStepper) -> ComputedXfoilRow:
    angles_of_attack = xfoil_aoa_stepper.get_all_angles_of_attack()
    values_for_aoas: list[None | ComputedXfoilValue] = [None for _ in range(len(angles_of_attack))]

    for index, angle_of_attack in enumerate(angles_of_attack):
        found_data_point = None
        for data_point in pol_file.data_points:
            if abs(data_point.alpha - angle_of_attack) < 1e-9:
                found_data_point = data_point
                break
        if found_data_point is None:
            continue
        values_for_aoas[index] = ComputedXfoilValue(
            angle_of_attack=angle_of_attack,
            lift_coefficient=found_data_point.CL, 
            drag_coefficient=found_data_point.CD,
            pressure_drag_coefficient=found_data_point.CDp,
            moment_coefficient=found_data_point.CM,
            top_transition=found_data_point.Top_Xtr,
            bottom_transition=found_data_point.Bot_Xtr
        )

    return ComputedXfoilRow(
        reynolds_number=pol_file.reynolds_number,
        angles_of_attack=angles_of_attack,
        values_for_aoas=values_for_aoas,
        xfoil_aoa_stepper=xfoil_aoa_stepper
    )
