from abc import ABC, abstractmethod
from pathlib import Path

class FolderPort(ABC):
    @abstractmethod
    def get_base_saved_path(self) -> Path:
        raise NotImplementedError("This method should be implemented by subclasses")
