#pragma once

#include "DroneSimulatorGame/Gameplay/Recording/FlightRecord.h"
#include "Runtime/Engine/Classes/GameFramework/Pawn.h"

#include "DronePawn.generated.h"


class UDroneController;
class UInputAction;
class UDroneMovementComponent;
class UDroneMovementController;
class UInputMappingContext;


USTRUCT(BlueprintType)
struct FFlightRecordPawnEvent
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	double event_time = 0.0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FFlightRecordEventData event_tata;

	FFlightRecordPawnEvent() = default;

	FFlightRecordPawnEvent(double in_event_time, const FFlightRecordEventData& in_event_data)
		: event_time(in_event_time), event_tata(in_event_data)
	{
	}
};

class UDroneInputSettings;


UCLASS(meta=(BlueprintSpawnableComponent))
class DRONESIMULATORGAME_API ADronePawn : public APawn
{
	GENERATED_BODY()

public:

	ADronePawn();

public:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true", DisplayName="Movement component"))
	TObjectPtr<UDroneMovementComponent> movement_component;

	virtual UPawnMovementComponent* GetMovementComponent() const override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool enable_recording = true;
	
	FCriticalSection temp_flight_record_mutex;
	TArray<FFlightRecordPawnEvent> temp_flight_record;

	UFUNCTION()
	void enqueue_flight_record(const TArray<FFlightRecordPawnEvent>& events);

	UFUNCTION()
	[[nodiscard]] TArray<FFlightRecordPawnEvent> consume_flight_record();
};
