#include "DroneSimulator/Simulation/SimulationWorld.h"

TTuple<double, FVector> USimulationWorld::get_wind_and_air_density() const
{
	return { 1.225, FVector::Zero() };
}
