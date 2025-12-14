#include "DroneSimulatorGame/Gameplay/DronePawn.h"
#include "DroneSimulatorGame/Gameplay/DroneMovementComponent.h"


ADronePawn::ADronePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	movement_component = CreateDefaultSubobject<UDroneMovementComponent>("MovementComponent");
}

UPawnMovementComponent* ADronePawn::GetMovementComponent() const
{
	return this->movement_component;
}

void ADronePawn::enqueue_flight_record(const TArray<FFlightRecordPawnEvent>& events)
{
	if (!enable_recording)
	{
		return;
	}

	FScopeLock Lock(&this->temp_flight_record_mutex);
	temp_flight_record.Append(events);
}

TArray<FFlightRecordPawnEvent> ADronePawn::consume_flight_record()
{
	FScopeLock Lock(&this->temp_flight_record_mutex);
	auto temp_flight_record_copy = temp_flight_record;
	temp_flight_record = {};
	return temp_flight_record_copy;
}
