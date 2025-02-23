/** Модуль с шейдерами */

#pragma once

#include "Modules/ModuleManager.h"

class FMeshPainterShaderCoreModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
