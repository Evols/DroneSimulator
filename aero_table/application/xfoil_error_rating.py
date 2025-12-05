from statistics import median

from aero_table.application.xfoil_pol_parser import ComputedXfoilRow


def get_positive_stall_aoa(row: ComputedXfoilRow):
    """
    Find the first local maximum in lift coefficient (positive stall).
    This is the first peak where CL starts decreasing after increasing,
    which represents the actual stall, not a secondary peak at high AoA.
    """
    angles = row.angles_of_attack
    
    # Build a list of valid (index, CL) pairs
    valid_data = []
    for index, value in enumerate(row.values_for_aoas):
        if value is not None:
            valid_data.append((index, angles[index], value.lift_coefficient))

    if len(valid_data) < 3:
        return None

    # Sort by angle of attack to ensure we process in order
    valid_data.sort(key=lambda x: x[1])
    
    # Find the first local maximum (where CL increases then decreases)
    # We look for positive angles of attack where we have a peak
    for i in range(1, len(valid_data) - 1):
        prev_cl = valid_data[i - 1][2]
        curr_cl = valid_data[i][2]
        next_cl = valid_data[i + 1][2]
        curr_aoa = valid_data[i][1]
        
        # Check if this is a local maximum and at positive AoA
        if curr_aoa > 0 and prev_cl < curr_cl and curr_cl > next_cl:
            return curr_aoa
    
    # Fallback: if no local maximum found, return global maximum for positive AoA
    positive_data = [(aoa, cl) for idx, aoa, cl in valid_data if aoa > 0]
    if positive_data:
        return max(positive_data, key=lambda x: x[1])[0]
    
    return None

def get_negative_stall_aoa(row: ComputedXfoilRow):
    """
    Find the first local minimum in lift coefficient (negative stall).
    This is the first trough where CL starts increasing after decreasing,
    which represents the actual negative stall, not a secondary trough at very low AoA.
    """
    angles = row.angles_of_attack
    
    # Build a list of valid (index, AoA, CL) pairs
    valid_data = []
    for index, value in enumerate(row.values_for_aoas):
        if value is not None:
            valid_data.append((index, angles[index], value.lift_coefficient))
    
    if len(valid_data) < 3:
        return None
    
    # Sort by angle of attack (ascending order)
    valid_data.sort(key=lambda x: x[1])
    
    # Find the first local minimum (where CL decreases then increases)
    # We look for negative angles of attack where we have a trough
    for i in range(1, len(valid_data) - 1):
        prev_cl = valid_data[i - 1][2]
        curr_cl = valid_data[i][2]
        next_cl = valid_data[i + 1][2]
        curr_aoa = valid_data[i][1]
        
        # Check if this is a local minimum and at negative AoA
        if curr_aoa < 0 and prev_cl > curr_cl and curr_cl < next_cl:
            return curr_aoa
    
    # Fallback: if no local minimum found, return global minimum for negative AoA
    negative_data = [(aoa, cl) for idx, aoa, cl in valid_data if aoa < 0]
    if negative_data:
        return min(negative_data, key=lambda x: x[1])[0]
    
    return None

def pick_best_candidate(rows: list[ComputedXfoilRow]) -> ComputedXfoilRow | None:
    """
    Pick the row whose positive and negative stall angles are closest to the median values.
    This helps select the most representative aerodynamic data from multiple runs.
    """

    if len(rows) == 0:
        return None
    
    if len(rows) == 1:
        return rows[0]

    positive_stalls = [get_positive_stall_aoa(row) for row in rows]
    negative_stalls = [get_negative_stall_aoa(row) for row in rows]

    median_positive_stall_aoa = median(positive_stalls)
    median_negative_stall_aoa = median(negative_stalls)

    # Find the row with stall points closest to the median
    best_row: ComputedXfoilRow | None = None
    best_error = float('inf')
    
    for row in rows:
        error = rate_row_error(row, median_positive_stall_aoa, median_negative_stall_aoa)

        if error < best_error:
            best_error = error
            best_row = row

    return best_row


def rate_row_error(row: ComputedXfoilRow, median_positive_stall_aoa: float, median_negative_stall_aoa: float) -> float:
    row_positive_stall_aoa = get_positive_stall_aoa(row)
    row_negative_stall_aoa = get_negative_stall_aoa(row)

    if row_positive_stall_aoa is None or row_negative_stall_aoa is None:
        return float('inf')

    positive_stall_relative_error = abs((row_positive_stall_aoa - median_positive_stall_aoa) / median_positive_stall_aoa)
    negative_stall_relative_error = abs((row_negative_stall_aoa - median_negative_stall_aoa) / median_negative_stall_aoa)

    all_aoa = row.angles_of_attack

    positive_stall_idx = [idx for idx, aoa in enumerate(all_aoa) if abs(aoa - row_positive_stall_aoa) < 0.01][0]
    negative_stall_idx = [idx for idx, aoa in enumerate(all_aoa) if abs(aoa - row_negative_stall_aoa) < 0.01][0]

    # The core idea is that can fill a point if there are at least 2 valid points before and after it

    none_error_sum = 0.0
    none_checkable_array = row.values_for_aoas[negative_stall_idx:positive_stall_idx + 1]
    for idx, value in enumerate(none_checkable_array):
        if value is not None:
            continue

        if idx < 2:
            return float('inf')

        prev_idx = idx - 1
        prev_prev_idx = idx - 1
        if none_checkable_array[prev_idx] is None or none_checkable_array[prev_prev_idx] is None:
            return float('inf')

        if idx >= len(none_checkable_array) - 2:
            return float('inf')

        next_idx = idx + 1
        next_next_idx = idx + 2
        if none_checkable_array[next_idx] is None or none_checkable_array[next_next_idx] is None:
            return float('inf')

        # We can fill a single point missing, but we prefer a row that has as few missing points as possible
        none_error_sum += 1.0

    none_error_sum = none_error_sum / len(none_checkable_array)

    return 40.0 * none_error_sum + 1.0 * (positive_stall_relative_error ** 2 + negative_stall_relative_error ** 2)
