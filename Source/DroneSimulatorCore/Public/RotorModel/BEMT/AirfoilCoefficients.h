#pragma once

#include "DroneSimulatorCore/Public/Simulation/Structural.h"

struct FAirfoilCoefficients
{
    double lift;
    double drag;

public:

    FAirfoilCoefficients(double in_lift, double in_drag):
        lift(in_lift), drag(in_drag)
    {
    }

    static FAirfoilCoefficients get_sensible_defaults()
    {
        return FAirfoilCoefficients(0.0, 0.1);
    }
};

namespace simulation_bemt
{
    TOptional<FAirfoilCoefficients> interpolate_airfoil_coefficients(double reynolds, double angle_of_attack, const FDroneAirfoil& airfoil);
}
