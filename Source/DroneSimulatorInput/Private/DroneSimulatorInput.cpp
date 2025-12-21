#include "DroneSimulatorInput.h"
#include "DroneInputKeys.h"
#include "Modules/ModuleManager.h"

class FDroneSimulatorInputModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FDroneInputKeys::register_keys();
	}
};

IMPLEMENT_MODULE(FDroneSimulatorInputModule, DroneSimulatorInput);

DEFINE_LOG_CATEGORY(LogDroneSimulatorInput);
