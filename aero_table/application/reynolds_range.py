from dataclasses import dataclass
import math

small_value = 0.001

def get_subvalues_single_decade(min_value: float, max_value: float, num_subvalues_per_10: int) -> list[float]:
    """
    Generate logarithmically spaced Reynolds numbers, if min and max are in the same decade.
    Aka min/max are on the same power of 10.
    """
    log_min = math.log10(min_value)
    log_max = math.log10(max_value)

    decade_num_subvalues = round((log_max - log_min) * num_subvalues_per_10)

    result = []
    for i in range(decade_num_subvalues):
        result.append(min_value * 10 ** (i / decade_num_subvalues * (log_max - log_min)))
    result.append(max_value)

    return result


def get_subvalues_first_decade(min_value: float, num_subvalues_per_10: int):
    log_min = math.log10(min_value)
    min_decade_top = math.ceil(log_min + small_value)
    log_delta = min_decade_top - log_min

    first_decade_num_subvalues = max(1, round(num_subvalues_per_10 * log_delta))

    result = []
    for i in range(first_decade_num_subvalues):
        result.append(min_value * 10 ** (i / first_decade_num_subvalues * log_delta))

    return result


def get_subvalues_mid_decade(min_decade_top: int, max_decade_bottom: int, num_subvalues_per_10: int):
    result = []
    for decade_idx in range(max_decade_bottom - min_decade_top):
        decade = min_decade_top + decade_idx
        for i in range(num_subvalues_per_10):
            result.append(10 ** (decade + i / num_subvalues_per_10))

    return result


def get_subvalues_last_decade(max_value: float, num_subvalues_per_10: int):
    log_max = math.log10(max_value)
    max_decade_bottom = math.floor(log_max + small_value)
    log_delta = log_max - max_decade_bottom

    last_decade_num_subvalues = max(1, round(num_subvalues_per_10 * log_delta))

    result = []
    for i in range(last_decade_num_subvalues):
        result.append(10 ** (max_decade_bottom + i / last_decade_num_subvalues * log_delta))

    return result


@dataclass
class ReynoldsRange:
    min: float
    max: float
    num_subvalues_per_10: int

    def get_subvalues(self) -> list[float]:
        """
        Generate logarithmically spaced Reynolds numbers.

        Returns a list of Reynolds numbers where values are almost evenly distributed
        on a logarithmic scale, with num_subvalues_per_10 points per decade
        (power of 10 range).

        min, max are always included in the result.
        Every power of 10 between min and max is included as well.
        Because of this, the "evenly distributed" property is not guaranteed.
        """
        if self.min <= 0 or self.max <= 0:
            raise ValueError("Reynolds numbers must be positive")
        if self.min == self.max:
            return [self.min]
        if self.min > self.max:
            raise ValueError("min must be less than max")
        if self.num_subvalues_per_10 <= 0:
            raise ValueError("num_subvalues_per_10 must be positive")

        # Calculate the range in logarithmic (base 10) space
        log_min = math.log10(self.min)
        log_max = math.log10(self.max)

        min_decade_bottom = math.floor(log_min + small_value)
        max_decade_top = math.ceil(log_max - small_value)

        # If the decades are inconsistent, return an empty list
        if min_decade_bottom > max_decade_top:
            return []

        # If the decades are equal, return a single value
        if min_decade_bottom == max_decade_top:
            return [self.min]

        # If the decades are adjacent, just generate the values in between
        if min_decade_bottom + 1 == max_decade_top:
            return get_subvalues_single_decade(self.min, self.max, self.num_subvalues_per_10)

        # Here we can assume that min_decade_bottom + 1 < max_decade_top

        result = []
        result += get_subvalues_first_decade(self.min, self.num_subvalues_per_10)

        min_decade_top = min_decade_bottom + 1
        max_decade_bottom = max_decade_top - 1
        result += get_subvalues_mid_decade(min_decade_top, max_decade_bottom, self.num_subvalues_per_10)

        result += get_subvalues_last_decade(self.max, self.num_subvalues_per_10)
        if abs(result[-1] - self.max) >= small_value:
            result += [self.max]

        return result
