// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAssetManager_DDDSourceModule : public IModuleInterface
{

	
public:

	static inline FAssetManager_DDDSourceModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FAssetManager_DDDSourceModule>("AssetManager_DDDSource");
	}


	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("AssetManager_DDDSource");
	}

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
