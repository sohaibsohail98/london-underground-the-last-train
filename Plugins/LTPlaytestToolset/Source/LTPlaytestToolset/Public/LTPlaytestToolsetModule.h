#pragma once

#include "Modules/ModuleInterface.h"

class FLTPlaytestToolsetModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
