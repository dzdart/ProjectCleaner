﻿// Copyright 2021. Ashot Barkhudaryan. All Rights Reserved.

#include "Core/ProjectCleanerManager.h"
#include "StructsContainer.h"
#include "UI/ProjectCleanerNotificationManager.h"
// Engine Headers
#include "AssetRegistryModule.h"
#include "Core/ProjectCleanerUtility.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/FileHelper.h"
#include "Engine/AssetManagerSettings.h"
#include "Engine/AssetManager.h"

#define LOCTEXT_NAMESPACE "FProjectCleanerModule"

FProjectCleanerManager::FProjectCleanerManager()
{
	CleanerConfigs = GetMutableDefault<UCleanerConfigs>();
	
	ensure(CleanerConfigs);
}

FProjectCleanerManager::~FProjectCleanerManager()
{
}

void FProjectCleanerManager::Update()
{
	if (DataManager.IsLoadingAssets(true)) return;
	
	FScopedSlowTask UpdateTask(1.0f, FText::FromString(FStandardCleanerText::Scanning));
	UpdateTask.MakeDialog();

	DataManager.SetCleanerConfigs(CleanerConfigs);
	DataManager.AnalyzeProject();

	UpdateTask.EnterProgressFrame();
	
	// Broadcast to all bounded objects that data is updated
	if (OnCleanerManagerUpdated.IsBound())
	{
		OnCleanerManagerUpdated.Execute();
	}
}

void FProjectCleanerManager::ExcludeSelectedAssets(const TArray<FAssetData>& Assets)
{
	DataManager.ExcludeSelectedAssets(Assets);
	
	Update();
}

void FProjectCleanerManager::ExcludeSelectedAssetsByType(const TArray<FAssetData>& Assets)
{
	for (const auto& Asset : Assets)
	{
		if (Asset.AssetClass.IsEqual(TEXT("Blueprint")))
		{
			const auto LoadedAsset = Asset.GetAsset();
			if (!LoadedAsset) continue;
			
			const auto BlueprintAsset = Cast<UBlueprint>(LoadedAsset);
			if (BlueprintAsset && BlueprintAsset->GeneratedClass)
			{
				CleanerConfigs->Classes.AddUnique(BlueprintAsset->GeneratedClass);
			}
		}
		else
		{
			CleanerConfigs->Classes.AddUnique(Asset.GetClass());
		}
	}
	
	Update();
}

bool FProjectCleanerManager::ExcludePath(const FString& InPath)
{
	if (!DataManager.ExcludePath(InPath))
	{
		return false;
	}
	
	FDirectoryPath DirectoryPath;
	DirectoryPath.Path = InPath;
	
	const bool bAlreadyExcluded = CleanerConfigs->Paths.ContainsByPredicate([&] (const FDirectoryPath& DirPath)
	{
		return DirPath.Path.Equals(InPath); 
	});
	
	if (!bAlreadyExcluded)
	{
		CleanerConfigs->Paths.Add(DirectoryPath);
	}
	
	Update();

	return true;
}

bool FProjectCleanerManager::IncludePath(const FString& InPath)
{
	if (!DataManager.IncludePath(InPath))
	{
		ProjectCleanerNotificationManager::AddTransient(
			FText::FromString(FStandardCleanerText::CantIncludePath),
			SNotificationItem::CS_Fail,
			10.0f
		);
		return false;
	}
	
	CleanerConfigs->Paths.RemoveAll([&] (const FDirectoryPath& DirPath)
	{
		return DirPath.Path.Equals(InPath);
	});
	
	Update();

	return true;
}

bool FProjectCleanerManager::IncludeSelectedAssets(const TArray<FAssetData>& Assets)
{
	if (!DataManager.IncludeSelectedAssets(Assets))
	{
		ProjectCleanerNotificationManager::AddTransient(
			FText::FromString(FStandardCleanerText::CantIncludeSomeAssets),
			SNotificationItem::CS_Fail,
			10.0f
		);
		return false;
	}
	
	Update();

	return true;
}

int32 FProjectCleanerManager::DeleteSelectedAssets(const TArray<FAssetData>& Assets)
{
	const int32 AssetNum = Assets.Num();
	const int32 DeletedAssetsNum = DataManager.DeleteSelectedAssets(Assets);

	if (DeletedAssetsNum != 0 && DeletedAssetsNum != AssetNum)
	{
		ProjectCleanerNotificationManager::AddTransient(
			FText::FromString(FStandardCleanerText::FailedToDeleteSomeAssets),
			SNotificationItem::CS_Fail,
			3.0f
		);
	}

	if (DeletedAssetsNum > 0)
	{
		Update();
	}

	return DeletedAssetsNum;
}

void FProjectCleanerManager::DeleteAllUnusedAssets()
{
	DataManager.CleanProject();
}

int32 FProjectCleanerManager::DeleteEmptyFolders()
{
	const int32 EmptyFoldersNum = DataManager.GetEmptyFolders().Num();
	const int32 DeletedFoldersNum = DataManager.DeleteEmptyFolders();

	if (DeletedFoldersNum != EmptyFoldersNum)
	{
		ProjectCleanerNotificationManager::AddTransient(
			FText::FromString(FStandardCleanerText::FailedToDeleteSomeFolders),
			SNotificationItem::CS_Fail,
			5.0f
		);
	}
	else
	{
		ProjectCleanerNotificationManager::AddTransient(
			FText::FromString(FStandardCleanerText::FoldersSuccessfullyDeleted),
			SNotificationItem::CS_Success,
			5.0f
		);
	}

	ProjectCleanerUtility::FocusOnGameFolder();
	
	return DeletedFoldersNum;
}

const FProjectCleanerDataManager& FProjectCleanerManager::GetDataManager() const
{
	return DataManager;
}

const TArray<FAssetData>& FProjectCleanerManager::GetAllAssets() const
{
	return DataManager.GetAllAssets();
}

const TArray<FAssetData>& FProjectCleanerManager::GetUnusedAssets() const
{
	return DataManager.GetUnusedAssets();
}

const TSet<FName>& FProjectCleanerManager::GetExcludedAssets() const
{
	return DataManager.GetExcludedAssets();
}

const TSet<FName>& FProjectCleanerManager::GetCorruptedAssets() const
{
	return DataManager.GetCorruptedAssets();
}

const TSet<FName>& FProjectCleanerManager::GetNonEngineFiles() const
{
	return DataManager.GetNonEngineFiles();
}

const TMap<FAssetData, FIndirectAsset>& FProjectCleanerManager::GetIndirectAssets() const
{
	return DataManager.GetIndirectAssets();
}

const TSet<FName>& FProjectCleanerManager::GetEmptyFolders() const
{
	return DataManager.GetEmptyFolders();
}

const TSet<FName>& FProjectCleanerManager::GetPrimaryAssetClasses() const
{
	return DataManager.GetPrimaryAssetClasses();
}

UCleanerConfigs* FProjectCleanerManager::GetCleanerConfigs() const
{
	return CleanerConfigs;
}

float FProjectCleanerManager::GetUnusedAssetsPercent() const
{
	if (DataManager.GetAllAssets().Num() == 0) return 0.0f;

	return DataManager.GetUnusedAssets().Num() * 100.0f / DataManager.GetAllAssets().Num();
}

void FProjectCleanerManager::IncludeAllAssets()
{
	CleanerConfigs->Classes.Empty();
	CleanerConfigs->Paths.Empty();
	DataManager.IncludeAllAssets();

	Update();
}

#undef LOCTEXT_NAMESPACE
