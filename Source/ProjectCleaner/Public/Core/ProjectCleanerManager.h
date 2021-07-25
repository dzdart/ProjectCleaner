﻿// Copyright 2021. Ashot Barkhudaryan. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ProjectCleanerManager
{
public:
	ProjectCleanerManager();

	/**
	 * @brief Updates data containers
	 */
	void Update();

	/** Data Container getters **/
	const TArray<FAssetData>& GetAllAssets() const;
	const TSet<FName>& GetCorruptedAssets() const;
	const TSet<FName>& GetNonEngineFiles() const;
private:
	
	/**
	 * @brief Empties all data containers
	 */
	void Clean();

	/** Data Containers **/
	
	/**
	* @brief All assets in "/Game" folder
	*/
	TArray<FAssetData> AllAssets;

	/**
	* @brief Corrupted assets (assets with engine extension, but not available in AssetRegistry)
	*/
	TSet<FName> CorruptedAssets;

	/**
	* @brief All non engine files (not .uasset or .umap files)
	*/
	TSet<FName> NonEngineFiles;

	class FAssetRegistryModule* AssetRegistry;
};
