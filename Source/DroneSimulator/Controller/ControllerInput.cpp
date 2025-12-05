#include "DroneSimulator/Controller/ControllerInput.h"


FDronePlayerInput FDronePlayerInput::zero()
{
    return FDronePlayerInput { 0.0, 0.0, 0.0, 0.0 };
}
