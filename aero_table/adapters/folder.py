from pathlib import Path

from aero_table.ports.folder import FolderPort


class FolderAdapter(FolderPort):
    def get_base_saved_path(self) -> Path:
        parent_folder = Path(__file__).parent
        return parent_folder.joinpath("../../Saved/AeroTable")
