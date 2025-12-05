import csv
from pathlib import Path

from aero_table.application.xfoil_pol_parser import ComputedXfoilRow


def init_csv_table(csv_path: Path, angles_of_attack: list[float]):
    with open(csv_path, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        header = ['Reynolds', 'Coefficient'] + [f'AOA_{aoa}' for aoa in angles_of_attack]
        writer.writerow(header)


def write_csv_table_row(csv_path: Path, row: ComputedXfoilRow):
    values_for_aoas = row.values_for_aoas

    with open(csv_path, 'a', newline='') as csvfile:
        writer = csv.writer(csvfile)

        # Write CL row
        cl_row = [row.reynolds_number, 'CL']
        for aoa_idx in range(len(values_for_aoas)):
            value = values_for_aoas[aoa_idx]
            if value is None:
                cl_row.append('')
            else:
                cl_row.append(value.lift_coefficient)
        writer.writerow(cl_row)

        # Write CD row
        cd_row = [row.reynolds_number, 'CD']
        for aoa_idx in range(len(values_for_aoas)):
            value = values_for_aoas[aoa_idx]
            if value is None:
                cd_row.append('')
            else:
                cd_row.append(value.drag_coefficient)
        writer.writerow(cd_row)

        # Write CM row
        cm_row = [row.reynolds_number, 'CM']
        for aoa_idx in range(len(values_for_aoas)):
            value = values_for_aoas[aoa_idx]
            if value is None:
                cm_row.append('')
            else:
                cm_row.append(value.moment_coefficient)
        writer.writerow(cm_row)


def write_csv_table(rows: list[ComputedXfoilRow], csv_path: Path):
    if len(rows) == 0:
        print("No rows to write to CSV file.")
        return

    first_row = rows[0]
    init_csv_table(csv_path, first_row.angles_of_attack)

    for row in rows:
        write_csv_table_row(csv_path, row)
