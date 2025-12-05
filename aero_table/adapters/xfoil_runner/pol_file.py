import re
from pathlib import Path
from typing import Optional

from aero_table.ports.xfoil_runner import XfoilPolFile, XfoilDataPoint


def parse_pol_file(file_path: Path) -> Optional[XfoilPolFile]:
    """
    Parse an XFoil polar (.pol) file.

    Args:
        file_path: Path to the .pol file

    Returns:
        PolFile object containing the parsed data, or None if parsing fails
    """
    try:
        with open(file_path, 'r') as f:
            lines = f.readlines()

        # Initialize variables
        airfoil_name = ""
        reynolds_number = 0.0
        mach_number = 0.0
        ncrit = 0.0
        xtrf_top = 0.0
        xtrf_bottom = 0.0
        data_points = []

        # Parse metadata
        for i, line in enumerate(lines):
            # Extract airfoil name (line 4)
            if "Calculated polar for:" in line:
                airfoil_name = line.split("Calculated polar for:")[1].strip()

            # Extract xtrf values
            if "xtrf =" in line:
                match = re.search(r'xtrf\s+=\s+([\d.]+)\s+\(top\)\s+([\d.]+)\s+\(bottom\)', line)
                if match:
                    xtrf_top = float(match.group(1))
                    xtrf_bottom = float(match.group(2))

            # Extract Mach, Re, and Ncrit
            if "Mach =" in line and "Re =" in line:
                # Example: " Mach =   0.000     Re =     0.501 e 6     Ncrit =   7.000"
                mach_match = re.search(r'Mach\s+=\s+([\d.]+)', line)
                re_match = re.search(r'Re\s+=\s+([\d.]+)\s+e\s+(\d+)', line)
                ncrit_match = re.search(r'Ncrit\s+=\s+([\d.]+)', line)

                if mach_match:
                    mach_number = float(mach_match.group(1))
                if re_match:
                    reynolds_number = float(re_match.group(1)) * (10 ** int(re_match.group(2)))
                if ncrit_match:
                    ncrit = float(ncrit_match.group(1))

            # Find the data section (starts after the dashed line)
            if line.strip().startswith("------"):
                # Data starts on the next line
                data_start_idx = i + 1
                break
        else:
            # No data separator found
            return None

        # Parse data points
        for line in lines[data_start_idx:]:
            line = line.strip()
            if not line:  # Skip empty lines
                continue

            # Split by whitespace and try to parse
            parts = line.split()
            if len(parts) >= 7:
                try:
                    alpha = float(parts[0])
                    CL = float(parts[1])
                    CD = float(parts[2])
                    CDp = float(parts[3])
                    CM = float(parts[4])
                    Top_Xtr = float(parts[5])
                    Bot_Xtr = float(parts[6])

                    data_points.append(XfoilDataPoint(
                        alpha=alpha,
                        CL=CL,
                        CD=CD,
                        CDp=CDp,
                        CM=CM,
                        Top_Xtr=Top_Xtr,
                        Bot_Xtr=Bot_Xtr
                    ))
                except ValueError:
                    # Skip lines that can't be parsed as floats
                    continue

        if not data_points:
            return None

        return XfoilPolFile(
            airfoil_name=airfoil_name,
            reynolds_number=reynolds_number,
            mach_number=mach_number,
            ncrit=ncrit,
            xtrf_top=xtrf_top,
            xtrf_bottom=xtrf_bottom,
            data_points=data_points
        )

    except (FileNotFoundError, IOError) as e:
        print(f"Error reading file {file_path}: {e}")
        return None


def get_cl_at_alpha(pol_file: XfoilPolFile, alpha: float) -> Optional[float]:
    """
    Get the lift coefficient (CL) at a specific angle of attack.
    Uses linear interpolation if the exact alpha is not found.

    Args:
        pol_file: The parsed PolFile object
        alpha: Angle of attack in degrees

    Returns:
        CL value, or None if alpha is out of range
    """
    data = pol_file.data_points

    # Check if alpha is in range
    if alpha < data[0].alpha or alpha > data[-1].alpha:
        return None

    # Find exact match or interpolate
    for i in range(len(data) - 1):
        if data[i].alpha <= alpha <= data[i + 1].alpha:
            if data[i].alpha == alpha:
                return data[i].CL
            elif data[i + 1].alpha == alpha:
                return data[i + 1].CL
            else:
                # Linear interpolation
                t = (alpha - data[i].alpha) / (data[i + 1].alpha - data[i].alpha)
                return data[i].CL + t * (data[i + 1].CL - data[i].CL)

    return None


def get_cd_at_alpha(pol_file: XfoilPolFile, alpha: float) -> Optional[float]:
    """
    Get the drag coefficient (CD) at a specific angle of attack.
    Uses linear interpolation if the exact alpha is not found.

    Args:
        pol_file: The parsed PolFile object
        alpha: Angle of attack in degrees

    Returns:
        CD value, or None if alpha is out of range
    """
    data = pol_file.data_points

    # Check if alpha is in range
    if alpha < data[0].alpha or alpha > data[-1].alpha:
        return None

    # Find exact match or interpolate
    for i in range(len(data) - 1):
        if data[i].alpha <= alpha <= data[i + 1].alpha:
            if data[i].alpha == alpha:
                return data[i].CD
            elif data[i + 1].alpha == alpha:
                return data[i + 1].CD
            else:
                # Linear interpolation
                t = (alpha - data[i].alpha) / (data[i + 1].alpha - data[i].alpha)
                return data[i].CD + t * (data[i + 1].CD - data[i].CD)

    return None
