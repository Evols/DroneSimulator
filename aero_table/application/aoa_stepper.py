from dataclasses import dataclass


@dataclass
class AoaStepper:
    """
    Uniform angle-of-attack stepping configuration for xfoil sweeps.
    
    This defines the xfoil run parameters: starting angle, ending angle, and step size.
    Used exclusively for configuring xfoil - NOT for Viterna-extended data.
    """
    min: float
    max: float
    step: float

    def get_expected_count(self) -> int:
        """Calculate the expected number of values."""
        if self.step <= 0:
            raise ValueError("step must be positive")
        # Calculate number of steps including both endpoints
        return int(round((self.max - self.min) / self.step)) + 1

    def get_all_angles_of_attack(self) -> list[float]:
        """Get the list of all angle of attack values for this uniform sweep."""
        aoas = []
        current = self.min
        while abs(current) <= abs(self.max) + 1e-9:  # Small epsilon for floating point comparison
            aoas.append(current)
            current += self.step

        return aoas
