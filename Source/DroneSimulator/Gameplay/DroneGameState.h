#pragma once

#include "Runtime/Engine/Classes/GameFramework/GameStateBase.h"

#include "DroneGameState.generated.h"

class ADroneFlightRecordingManager;

UCLASS()
class ADroneGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

	UFUNCTION()
	void setup_flight_recorder();

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ADroneFlightRecordingManager> flight_recording_manager;

};
