﻿// Copyright 2021. Ashot Barkhudaryan. All Rights Reserved.

#include "Core/ProjectCleanerUtility.h"
#include "StructsContainer.h"
#include "ProjectCleaner.h"
// Engine Headers
#include "ObjectTools.h"
#include "FileHelpers.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IContentBrowserSingleton.h"
#include "Engine/AssetManager.h"
#include "Engine/MapBuildDataRegistry.h"
#include "UObject/ObjectRedirector.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "Editor/ContentBrowser/Public/ContentBrowserModule.h"

int64 ProjectCleanerUtility::GetTotalSize(const TArray<FAssetData>& Assets)
{
	int64 Size = 0;
	for (const auto& Asset : Assets)
	{
		FAssetRegistryModule& AssetRegistry = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
		const auto AssetPackageData = AssetRegistry.Get().GetAssetPackageData(Asset.PackageName);
		if (!AssetPackageData) continue;
		Size += AssetPackageData->DiskSize;
	}

	return Size;
}

FString ProjectCleanerUtility::ConvertAbsolutePathToInternal(const FString& InPath)
{
	FString Path = InPath;
	FPaths::NormalizeFilename(Path);
	const FString ProjectContentDirAbsPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	return ConvertPathInternal(ProjectContentDirAbsPath, FString{ "/Game/" }, Path);
}

FString ProjectCleanerUtility::ConvertInternalToAbsolutePath(const FString& InPath)
{
	FString Path = InPath;
	FPaths::NormalizeFilename(Path);
	const FString ProjectContentDirAbsPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	return ConvertPathInternal(FString{ "/Game/" }, ProjectContentDirAbsPath, Path);
}

bool ProjectCleanerUtility::DeleteEmptyFolders(TSet<FName>& EmptyFolders)
{
	if (EmptyFolders.Num() == 0) return false;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	
	bool ErrorWhileDeleting = false;
	TSet<FString> FailedToDeleteFolders;
	for (auto& EmptyFolder : EmptyFolders)
	{
		const FString EmptyFolderStr = EmptyFolder.ToString();
		if (!IFileManager::Get().DirectoryExists(*EmptyFolderStr)) continue;

		if (!IFileManager::Get().DeleteDirectory(*EmptyFolderStr, false, true))
		{
			ErrorWhileDeleting = true;
			UE_LOG(LogProjectCleaner, Error, TEXT("Failed to delete %s folder."), *EmptyFolderStr);
			FailedToDeleteFolders.Add(EmptyFolderStr);
			continue;
		}

		// removing folder path from asset registry
		AssetRegistryModule.Get().RemovePath(ConvertAbsolutePathToInternal(EmptyFolderStr));
	}

	EmptyFolders.Empty();
	
	if (ErrorWhileDeleting)
	{
		for (const auto& Folder : FailedToDeleteFolders)
		{
			EmptyFolders.Add(FName{*Folder});
		}
	}

	return !ErrorWhileDeleting;
}

bool ProjectCleanerUtility::IsEngineExtension(const FString& Extension)
{
	return Extension.Equals("uasset") || Extension.Equals("umap");
}

FString ProjectCleanerUtility::ConvertPathInternal(const FString& From, const FString To, const FString& Path)
{
	return Path.Replace(*From, *To, ESearchCase::IgnoreCase);
}

void ProjectCleanerUtility::FixupRedirectors()
{
	FScopedSlowTask SlowTask{1.0f, FText::FromString("Fixing up Redirectors...")};
	SlowTask.MakeDialog();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace(TEXT("/Game"));
	Filter.ClassNames.Emplace(UObjectRedirector::StaticClass()->GetFName());

	// Query for a list of assets
	TArray<FAssetData> AssetList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

	if (AssetList.Num() > 0)
	{
		TArray<UObject*> Objects;
		// loading asset if needed
		for (const auto& Asset : AssetList)
		{
			const auto AssetObj = Asset.GetAsset();
			if (!AssetObj) continue;
			Objects.Add(AssetObj);
		}

		// converting them to redirectors
		TArray<UObjectRedirector*> Redirectors;
		for (auto Object : Objects)
		{
			const auto Redirector = CastChecked<UObjectRedirector>(Object);
			if (!Redirector) continue;
			Redirectors.Add(Redirector);
		}

		// Fix up all founded redirectors
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetToolsModule.Get().FixupReferencers(Redirectors);
	}
	UE_LOG(LogProjectCleaner, Display, TEXT("Redirectors fixed!"));
	SlowTask.EnterProgressFrame(1.0f);
}

int32 ProjectCleanerUtility::DeleteAssets(TArray<FAssetData>& Assets, const bool ForceDelete)
{
	const int32 GivenAssetsNum = Assets.Num();
	// first try to delete normally
	int32 DeletedAssets = ObjectTools::DeleteAssets(Assets, false);

	// if normally not working try to force delete
	if (DeletedAssets != GivenAssetsNum && ForceDelete)
	{
		TArray<UObject*> AssetObjects;
		AssetObjects.Reserve(Assets.Num());
		
		FScopedSlowTask SlowTask(
		Assets.Num(),
		FText::FromString(FStandardCleanerText::LoadingAssets)
		);
		for (const auto& Asset : Assets)
		{
			SlowTask.EnterProgressFrame();
			const auto AssetObj = Asset.GetAsset();
			if(!AssetObj) continue;
			AssetObjects.Add(AssetObj);
		}
	
		DeletedAssets = ObjectTools::ForceDeleteObjects(AssetObjects, false);
	}

	return DeletedAssets;
}

void ProjectCleanerUtility::SaveAllAssets(const bool PromptUser = true)
{
	FEditorFileUtils::SaveDirtyPackages(
		PromptUser,
		true,
		true,
		false,
		false,
		false
	);
}

void ProjectCleanerUtility::UpdateAssetRegistry(bool bSyncScan = false)
{
	FAssetRegistryModule& AssetRegistry = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	
	TArray<FString> ScanFolders;
	ScanFolders.Add("/Game");

	// AssetRegistry.Get().ScanPathsSynchronous(ScanFolders, true);
	AssetRegistry.Get().SearchAllAssets(bSyncScan);
}

void ProjectCleanerUtility::FocusOnGameFolder()
{
	TArray<FString> FocusFolders;
	FocusFolders.Add(TEXT("/Game"));
	
	FContentBrowserModule& CBModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	CBModule.Get().SyncBrowserToFolders(FocusFolders);
}
