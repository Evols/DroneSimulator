#include "DroneSimulator/Simulation/LogDebug.h"

DEFINE_LOG_CATEGORY(LogDroneSimulatorDebug);

constexpr bool log_to_unreal = false;

void FDebugLog::log(const FString& log)
{
    if (log_to_unreal)
    {
        UE_LOG(LogDroneSimulatorDebug, Display, TEXT("%s"), *log);
    }

    this->logs.Add(log);
}

void FDebugLog::append_debug_log(const FDebugLog& other)
{
    this->logs.Append(other.logs);
}
