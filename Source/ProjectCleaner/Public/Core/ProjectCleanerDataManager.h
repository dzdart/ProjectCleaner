﻿// Copyright 2021. Ashot Barkhudaryan. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StructsContainer.h"

struct FAssetData;
class FAssetToolsModule;
class FAssetRegistryModule;
class IPlatformFile;

class ProjectCleanerDataManagerV2
{
public:

	// ctor/dtor
	ProjectCleanerDataManagerV2();
	~ProjectCleanerDataManagerV2();

	void AnalyzeProject();
	void SetSilentMode(const bool SilentMode);
	void SetScanDeveloperContents(const bool bScan);
	void PrintInfo();
	// void SetExcludedPaths();
	// void SetExcludedClasses();
	
	void CleanProject();
	
private:
	
	void FixupRedirectors() const;
	void FindAllAssets();
	void FindInvalidFilesAndAssets();
	void FindIndirectAssets();
	void FindEmptyFolders(const bool bScanDevelopersContent);
	void FindPrimaryAssetClasses();
	void FindAssetsWithExternalReferencers();
	void FindUnusedAssets();
	void FindUsedAssets(TSet<FName>& UsedAssets);
	void FindUsedAssetsDependencies(const TSet<FName>& UsedAssets, TSet<FName>& UsedAssetsDeps) const;
	void FindExcludedAssets(TSet<FName>& UsedAssets);
	void AnalyzeUnusedAssets();
	void GetDeletionAssetsBucket(TArray<FAssetData>& Bucket, const int32 BucketSize);

	void DeleteUnusedAssets();
	void DeleteEmptyFolders();

	/* Check Functions */
	bool IsExcludedByClass(const FAssetData& AssetData) const;
	
	/* Data Containers */
	TArray<FAssetData> AllAssets;
	TArray<FAssetData> UnusedAssets;
	TArray<FAssetData> PrimaryAssets;
	TArray<FAssetData> UserExcludedAssets;
	TArray<FAssetData> AssetsWithExternalRefs;
	TSet<FName> CorruptedAssets;
	TSet<FName> NonEngineFiles;
	TSet<FName> EmptyFolders;
	TSet<FName> PrimaryAssetClasses;
	TSet<FName> ExcludedAssets;
	TMap<FAssetData, FIndirectAsset> IndirectAssets;
	TMap<FAssetData, FUnusedAssetInfo> UnusedAssetsInfos;

	/* Configs */
	bool bSilentMode;
	bool bScanDeveloperContents;
	bool bAutomaticallyDeleteEmptyFolders;
	TSet<FName> ExcludedPaths;
	TSet<FName> ExcludedClasses;

	/* Engine Modules */
	FAssetRegistryModule* AssetRegistry;
	FAssetToolsModule* AssetTools;
	IPlatformFile* PlatformFile;

	/* Constants */
	const FName RelativeRoot;
};

// ==============================
// Refactor
// ==============================
// ==============================
// ==============================
class ProjectCleanerDataManager
{
	friend class FProjectCleanerDataManagerTest;
public:
	
	/**
	 * @brief Returns all assets in given path using AssetRegistry
	 * @param InPath - Relative path. Example "/Game"
	 * @param AllAssets - Asset Container
	 */
	static void GetAllAssetsByPath(const FName& InPath, TArray<FAssetData>& AllAssets);

	/**
	 * @brief Returns all corrupted assets and non engine files in given path
	 * @param InPath - Absolute path. Example "C:/dev/projects/project_name/content/"
	 * @param AllAssets - All assets in path
	 * @param CorruptedAssets - Corrupted assets container
	 * @param NonEngineFiles - Non engine files container
	 */
	static void GetInvalidFilesByPath(
		const FString& InPath,
		const TArray<FAssetData>& AllAssets,
		TSet<FName>& CorruptedAssets,
		TSet<FName>& NonEngineFiles
	);

	/**
	 * @brief Returns all asset package names that used in source code and config files, under Source/, Config/ or Plugins/ Directories
	 * @param InPath - Absolute path. Example "C:/dev/projects/project_name/content/"
	 * @param IndirectlyUsedAssets - Indirect assets container
	 * @param AllAssets - All assets in Project
	 */
	static void GetIndirectAssetsByPath(
		const FString& InPath,
		TMap<FAssetData, FIndirectAsset>& IndirectlyUsedAssets,
		const TArray<FAssetData>& AllAssets
	);

	/**
	 * @brief Returns all empty folders in given path
	 * @param InPath - Absolute path. Example "C:/dev/projects/project_name/content/"
	 * @param EmptyFolders - Empty Folders container
	 * @param bScanDevelopersContent - Flag should we include Developer Content or not
	 */
	static void GetEmptyFolders(const FString& InPath, TSet<FName>& EmptyFolders, const bool bScanDevelopersContent);

	/**
	 * @brief Returns all primary asset classes in project
	 * @param PrimaryAssetClasses - Primary assets classes container
	 */
	static void GetPrimaryAssetClasses(TSet<FName>& PrimaryAssetClasses);

	/**
	 * @brief Return all assets that has external referencers outside "/Game" folder
	 * @param AssetsWithExternalRefs 
	 * @param AllAssets 
	 */
	static void GetAllAssetsWithExternalReferencers(TArray<FAssetData>& AssetsWithExternalRefs, const TArray<FAssetData>& AllAssets);

	/**
	 * @brief Returns class name of given asset data, takes into account blueprint assets
	 * @param AssetData 
	 * @return FName - ClassName
	 */
	static FName GetClassName(const FAssetData& AssetData);

	/**
	 * @brief Checks if given asset is under Megascans Plugin Base folder or not
	 * @param AssetData 
	 * @return bool
	 */
	static bool IsUnderMegascansFolder(const FAssetData& AssetData);

	/**
	 * @brief Checks if given asset is circular or not
	 * @explanation If assets refs exists in deps or vice versa, then its cyclic
	 * @param AssetData
	 * @param Refs
	 * @param Deps
	 * @param CommonAssets
	 * @return bool 
	 */
	static bool IsCircularAsset(const FAssetData& AssetData, TArray<FName>& Refs, TArray<FName>& Deps, TArray<FName>& CommonAssets);

	/**
	 * @brief Checks if given assets is root or not
	 * @explanation Root assets dont have referencers
	 * @param AssetData 
	 * @return bool
	 */
	static bool IsRootAsset(const FAssetData& AssetData);

private:
	/**
	 * @brief Finds all empty folders in given path recursively
	 * @param FolderPath - Absolute path
	 * @param EmptyFolders - Empty Folders container
	 * @return bool
	 */
	static bool FindEmptyFolders(const FString& FolderPath, TSet<FName>& EmptyFolders);

	/**
	 * @brief Checks if given string contains indirectly used assets
	 * @param FileContent - Given string to check, should be file content
	 * @description - Checking is done via regular expression, by search all pattern that start with "/Game"
	 * @return bool
	 */
	static bool HasIndirectlyUsedAssets(const FString& FileContent);
};