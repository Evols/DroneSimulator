from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Optional

from aero_table.application.aoa_stepper import AoaStepper


@dataclass
class XfoilDataPoint:
    """Represents a single data point in a polar file."""
    alpha: float  # Angle of attack (degrees)
    CL: float     # Lift coefficient
    CD: float     # Drag coefficient
    CDp: float    # Pressure drag coefficient
    CM: float     # Moment coefficient
    Top_Xtr: float  # Top transition point
    Bot_Xtr: float  # Bottom transition point


@dataclass
class XfoilPolFile:
    """Represents a complete polar file with metadata and data points."""
    airfoil_name: str
    reynolds_number: float
    mach_number: float
    ncrit: float
    xtrf_top: float
    xtrf_bottom: float
    data_points: list[XfoilDataPoint]


class XfoilRunnerPort(ABC):
    @abstractmethod
    def run_xfoil(self, xfoil_file_content: str, reynolds_number: float, aoa_stepper: AoaStepper, output_folder: str) -> Optional[XfoilPolFile]:
        raise NotImplementedError("This method should be implemented by subclasses")
