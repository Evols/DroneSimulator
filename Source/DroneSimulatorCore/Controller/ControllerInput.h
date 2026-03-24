#pragma once

#include "CoreMinimal.h"

#include "ControllerInput.generated.h"


USTRUCT(BlueprintType)
struct DRONESIMULATORCORE_API FDronePlayerInput
{
    GENERATED_BODY()

public:
    /**
     * Throttle is in 0..1 range
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    double throttle = 0.0;

    /**
     * Yaw is the rotation on the Z axis, in range -1..1
     * Positive yaw makes forward go right to be consistent with Unreal's rotators
     * It is the opposite to Euler and Tait–Bryan angles
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    double yaw = 0.0;

    /**
     * Pitch is the rotation on the Y axis, in range -1..1
     * Positive pitch makes forward go up to be consistent with Unreal's rotators
     * It is the same as Euler angles and Tait–Bryan angles
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    double pitch = 0.0;

    /**
     * Yaw is the rotation on the X axis, in range -1..1
     * Positive roll makes right go down, to be consistent with Unreal's rotators
     * It is the same as Euler angles and Tait–Bryan angles
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    double roll = 0.0;

public:

    static FDronePlayerInput zero();

};
