#pragma once

#include "DroneSimulator/Gameplay/Recording/FlightRecord.h"
#include "Runtime/Engine/Classes/Engine/DataAsset.h"

#include "FlightRecordAsset.generated.h"


USTRUCT(BlueprintType)
struct FFlightRecordEvent
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FName pawn_name;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	double event_time;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FFlightRecordEventData event_data;

public:

	FFlightRecordEvent() = default;

	FFlightRecordEvent(const FName& in_pawn_name, double in_event_time, const FFlightRecordEventData& in_event_data)
		: pawn_name(in_pawn_name), event_time(in_event_time), event_data(in_event_data)
	{
	}
};

/**
 * Flight record info for a session
 */
UCLASS(BlueprintType)
class DRONESIMULATOR_API UFlightRecordAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FFlightRecordEvent> events;
};
