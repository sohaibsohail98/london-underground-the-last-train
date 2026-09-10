#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/** Automation tests for LastTrain. Editor-only, never compiled into a shipping
	build. Kept as its own module so test code cannot leak a dependency the
	runtime module does not already have. */
class FLastTrainTestsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};
