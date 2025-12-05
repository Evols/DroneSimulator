#include "DroneSimulator/Gameplay/DroneGameState.h"
#include "DroneSimulator/Gameplay/Recording/DroneFlightRecordingManager.h"


void ADroneGameState::BeginPlay()
{
	Super::BeginPlay();

	this->setup_flight_recorder();
}

void ADroneGameState::setup_flight_recorder()
{
	auto* world = this->GetWorld();
	if (world == nullptr)
	{
		return;
	}

	this->flight_recording_manager = world->SpawnActor<ADroneFlightRecordingManager>();
}
