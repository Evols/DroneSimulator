"""
Utility to visualize Viterna post-stall extrapolation results.

This script helps verify that Viterna extrapolation is working correctly
by plotting the aerodynamic coefficients before and after extrapolation.
"""

import csv
from pathlib import Path
from typing import Optional


def plot_comparison(csv_path: Path, reynolds_number: Optional[float] = None):
    """
    Plot CL, CD, CM curves from CSV to visualize Viterna extrapolation.
    
    Args:
        csv_path: Path to the all_data_points.csv file
        reynolds_number: Optional Reynolds number to plot. If None, plots first Re.
    
    Note:
        Requires matplotlib. Install with: pip install matplotlib
    """
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("Error: matplotlib not installed. Install with: pip install matplotlib")
        return
    
    # Read CSV data
    with open(csv_path, 'r') as f:
        reader = csv.reader(f)
        header = next(reader)
        
        # Extract AOA values from header (skip 'Reynolds' and 'Coefficient' columns)
        aoa_values = [float(h.replace('AOA_', '')) for h in header[2:]]
        
        # Read data
        data_by_re = {}
        for row in reader:
            re = float(row[0])
            coeff_type = row[1]
            values = [float(v) if v != '' else None for v in row[2:]]
            
            if re not in data_by_re:
                data_by_re[re] = {}
            data_by_re[re][coeff_type] = values
    
    # Select Reynolds number to plot
    if reynolds_number is None:
        reynolds_number = sorted(data_by_re.keys())[0]
    elif reynolds_number not in data_by_re:
        available = sorted(data_by_re.keys())
        print(f"Reynolds number {reynolds_number} not found.")
        print(f"Available Reynolds numbers: {available}")
        return
    
    data = data_by_re[reynolds_number]
    
    # Create figure with subplots
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(12, 10))
    fig.suptitle(f'Aerodynamic Coefficients with Viterna Extrapolation\nRe = {reynolds_number:.0f}', 
                 fontsize=14, fontweight='bold')
    
    # Plot CL
    ax1.plot(aoa_values, data['CL'], 'b-', linewidth=2, label='CL')
    ax1.axhline(y=0, color='k', linestyle='--', alpha=0.3)
    ax1.axvline(x=0, color='k', linestyle='--', alpha=0.3)
    ax1.grid(True, alpha=0.3)
    ax1.set_xlabel('Angle of Attack (degrees)')
    ax1.set_ylabel('Lift Coefficient (CL)')
    ax1.set_title('Lift Coefficient')
    ax1.legend()
    
    # Plot CD
    ax2.plot(aoa_values, data['CD'], 'r-', linewidth=2, label='CD')
    ax2.axhline(y=0, color='k', linestyle='--', alpha=0.3)
    ax2.axvline(x=0, color='k', linestyle='--', alpha=0.3)
    ax2.grid(True, alpha=0.3)
    ax2.set_xlabel('Angle of Attack (degrees)')
    ax2.set_ylabel('Drag Coefficient (CD)')
    ax2.set_title('Drag Coefficient')
    ax2.legend()
    
    # Plot CM
    ax3.plot(aoa_values, data['CM'], 'g-', linewidth=2, label='CM')
    ax3.axhline(y=0, color='k', linestyle='--', alpha=0.3)
    ax3.axvline(x=0, color='k', linestyle='--', alpha=0.3)
    ax3.grid(True, alpha=0.3)
    ax3.set_xlabel('Angle of Attack (degrees)')
    ax3.set_ylabel('Moment Coefficient (CM)')
    ax3.set_title('Moment Coefficient')
    ax3.legend()
    
    plt.tight_layout()
    
    # Save figure
    output_path = csv_path.parent / f'viterna_plot_Re_{reynolds_number:.0f}.png'
    plt.savefig(output_path, dpi=150)
    print(f"Plot saved to: {output_path}")
    
    plt.show()


def print_summary(csv_path: Path):
    """
    Print summary statistics from the CSV file.
    
    Args:
        csv_path: Path to the all_data_points.csv file
    """
    with open(csv_path, 'r') as f:
        reader = csv.reader(f)
        header = next(reader)
        
        reynolds_numbers = set()
        total_points = 0
        filled_points = 0
        
        for row in reader:
            re = float(row[0])
            reynolds_numbers.add(re)
            
            for value in row[2:]:
                total_points += 1
                if value != '':
                    filled_points += 1
        
        coverage = 100 * filled_points / total_points if total_points > 0 else 0
        
        print(f"\n=== CSV Summary ===")
        print(f"CSV file: {csv_path}")
        print(f"Reynolds numbers: {len(reynolds_numbers)}")
        print(f"Total data points: {total_points}")
        print(f"Filled points: {filled_points} ({coverage:.1f}%)")
        print(f"Empty points: {total_points - filled_points} ({100-coverage:.1f}%)")


if __name__ == "__main__":
    import sys
    from aero_table.adapters.folder import FolderAdapter
    
    # Default to NACA 2412
    folder_name = "naca2412"
    if len(sys.argv) > 1:
        folder_name = sys.argv[1]
    
    csv_path = FolderAdapter().get_base_saved_path().joinpath(folder_name).joinpath("all_data_points.csv")
    
    if not csv_path.exists():
        print(f"Error: CSV file not found at {csv_path}")
        print("Run the aero table generation first: python -m aero_table.all")
        sys.exit(1)
    
    print_summary(csv_path)
    
    # Plot first Reynolds number by default
    plot_comparison(csv_path)

