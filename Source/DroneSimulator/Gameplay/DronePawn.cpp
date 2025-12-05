#include "DroneSimulator/Gameplay/DronePawn.h"
#include "DroneSimulator/Gameplay/DroneMovementComponent.h"

#include "Kismet/GameplayStatics.h"

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
