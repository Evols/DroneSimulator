import os
import signal
import subprocess
import sys
from typing import Optional
from pathlib import Path

from aero_table.ports.xfoil_file_generator import XFoilFileGenerator
from aero_table.ports.xfoil_runner import XfoilPolFile
from aero_table.application.aoa_stepper import AoaStepper

from .pol_file import parse_pol_file


def run_xfoil(generator: XFoilFileGenerator, reynolds_number: float, aoa_stepper: AoaStepper, output_folder: Path, base_file_name: str) -> Optional[XfoilPolFile]:
    os.makedirs(output_folder, exist_ok=True)
    
    # Create temp subdirectory for input/output files
    temp_folder = output_folder / "temp"
    os.makedirs(temp_folder, exist_ok=True)

    input_file_path = temp_folder / f"{base_file_name}_input.txt"
    pol_file_name = f"{base_file_name}_output.pol"

    # Generate input file content with current Reynolds number
    xfoil_file_content = generator.generate_file_content(pol_file_name, reynolds_number, aoa_stepper)

    with open(input_file_path, "w") as xfoil_input_file:
        xfoil_input_file.write(xfoil_file_content)

    try:
        # Run xfoil with the generated input file
        timeout = 30
        if sys.platform == "win32":
            # Run in a new process group so we can kill all children if needed
            xfoil_command = "xfoil"

            proc = subprocess.Popen(
                f'"{xfoil_command}" < "{input_file_path}"',
                shell=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                cwd=temp_folder,
                creationflags=subprocess.CREATE_NEW_PROCESS_GROUP,
            )
        else:
            # On Unix, start a new session for the same reason
            xfoil_command = "xfoil"
            proc = subprocess.Popen(
                f"{xfoil_command} < {input_file_path}",
                shell=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                cwd=temp_folder,
                preexec_fn=os.setsid,
            )

        # Wait for completion with timeout
        stdout, stderr = proc.communicate(timeout=timeout)

    except subprocess.TimeoutExpired as e:
        print(f"Xfoil timed out: {type(e).__name__}")
        if sys.platform == "win32":
            subprocess.call(
                ["taskkill", "/F", "/T", "/PID", str(proc.pid)],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
        else:
            os.killpg(os.getpgid(proc.pid), signal.SIGTERM)
        return None

    except (subprocess.TimeoutExpired, subprocess.SubprocessError) as e:
        print(f"Process failed or timed out: {type(e).__name__}")
        return None

    pol_file_path = temp_folder / pol_file_name

    # Check if the pol file was created
    if not os.path.isfile(pol_file_path):
        print(f"Pol file was not created: {pol_file_path}")
        if stdout:
            print(f"STDOUT:\n{stdout.decode(errors='replace')}")
        if stderr:
            print(f"STDERR:\n{stderr.decode(errors='replace')}")
        return None

    # Parse and validate the pol file
    pol_data = parse_pol_file(pol_file_path)
    if pol_data is None:
        print(f"Failed to parse pol file: {pol_file_path}")
        return None

    return pol_data
