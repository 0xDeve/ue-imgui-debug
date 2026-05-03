#pragma once

#include "Modules/ModuleManager.h"

class FImGuiDebugModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
