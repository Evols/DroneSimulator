# Aerodynamic coefficients table generator

This program generates tables of aerodynamic coefficients, for different airfoils.
It is based on Xfoil.

## Usage

The terminal should be in the root of the project.

To generate all tables, run:

```shell
python -m aero_table.all
```

To generate the tables for a specific airfoil, in this instance for NACA 2412, run:

```shell
python -m aero_table.airfoils.naca2412
```

## Testing

To test the program, run:

```shell
python -m unittest discover aero_table
```
