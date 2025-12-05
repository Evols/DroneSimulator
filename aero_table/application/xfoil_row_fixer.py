from copy import deepcopy

from aero_table.application.xfoil_pol_parser import ComputedXfoilRow, ComputedXfoilValue


def cubic_interpolation(before_2: float, before_1: float, after_1: float, after_2: float) -> float:
    """
    Perform a cubic interpolation between the given points.

    The four values are assumed to be equally spaced samples:
        before_2, before_1, (interpolated), after_1, after_2

    This returns the interpolated value between before_1 and after_1.
    """
    return (-before_2 + 9 * before_1 + 9 * after_1 - after_2) / 16.0


def cubic_interpolation_xfoil(before_2: ComputedXfoilValue, before_1: ComputedXfoilValue, after_1: ComputedXfoilValue, after_2: ComputedXfoilValue) -> ComputedXfoilValue:
    """
    Perform a cube interpolation between the given points.
    """

    return ComputedXfoilValue(
        angle_of_attack=cubic_interpolation(before_2.angle_of_attack, before_1.angle_of_attack, after_1.angle_of_attack, after_2.angle_of_attack),
        lift_coefficient=cubic_interpolation(before_2.lift_coefficient, before_1.lift_coefficient, after_1.lift_coefficient, after_2.lift_coefficient),
        drag_coefficient=cubic_interpolation(before_2.drag_coefficient, before_1.drag_coefficient, after_1.drag_coefficient, after_2.drag_coefficient),
        pressure_drag_coefficient=0.0,
        moment_coefficient=0.0,
        top_transition=0.0,
        bottom_transition=0.0,
    )


def try_to_fix_row(in_row: ComputedXfoilRow) -> ComputedXfoilRow:
    row = deepcopy(in_row)

    for index, value in enumerate(row.values_for_aoas):
        if value is not None:
            continue

        # If there are *two* valid points before and *two* valid points after the current index, use cubic interpolation to fill the value
        if index >= 2 and index <= len(row.values_for_aoas) - 3:
            if row.values_for_aoas[index - 1] is not None and row.values_for_aoas[index - 2] is not None and row.values_for_aoas[index + 1] is not None and row.values_for_aoas[index + 2] is not None:
                # Use cubic interpolation to fill the value
                value = cubic_interpolation_xfoil(row.values_for_aoas[index - 1], row.values_for_aoas[index - 2], row.values_for_aoas[index + 1], row.values_for_aoas[index + 2])
                row.values_for_aoas[index] = value
                continue

    return row
