from abc import ABC, abstractmethod

from aero_table.application.aoa_stepper import AoaStepper


class XFoilFileGenerator(ABC):
    @abstractmethod
    def generate_file_content(self, pol_file_name: str, reynolds_number: float, aoa_stepper: AoaStepper) -> str:
        raise NotImplementedError("This method should be implemented by subclasses")
