#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DroneFlightRecordingManager.generated.h"

class UFlightRecordAsset;

UCLASS()
class DRONESIMULATORGAME_API ADroneFlightRecordingManager : public AActor
{
	GENERATED_BODY()

public:

	ADroneFlightRecordingManager();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void setup_flight_record_asset();

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UFlightRecordAsset> flight_record_asset;

	UFUNCTION()
	void save_flight_record_asset();

public:

	virtual void Tick(float DeltaTime) override;

protected:

	void record_flight_record_of_pawns();

};
