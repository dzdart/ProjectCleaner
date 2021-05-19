// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ProjectCleanerCommands.h"
// Engine Headers
#include "Framework/Commands/Commands.h"
#include "ProjectCleanerStyle.h"

#define LOCTEXT_NAMESPACE "FProjectCleanerModule"

FProjectCleanerCommands::FProjectCleanerCommands() : TCommands<FProjectCleanerCommands>(
	TEXT("ProjectCleaner"),
	NSLOCTEXT("Contexts", "ProjectCleaner", "ProjectCleaner Plugin"),
	NAME_None,
	FProjectCleanerStyle::GetStyleSetName()
)
{
}

void FProjectCleanerCommands::RegisterCommands()
{
	UI_COMMAND(
		PluginAction,
		"ProjectCleaner",
		"Delete unused assets and empty folders.",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		DeleteAsset,
		"Delete Asset",
		"Delete Selected Assets",
		EUserInterfaceActionType::Button,
		FInputChord()
	);

	UI_COMMAND(
		ExcludeAsset,
		"Exclude Asset",
		"Exclude selected assets from deletion list",
		EUserInterfaceActionType::Button,
		FInputChord()
	);

	UI_COMMAND(
		IncludeAsset,
		"Include Asset",
		"Include selected assets to deletion list",
		EUserInterfaceActionType::Button,
		FInputChord()
	);
}

#undef LOCTEXT_NAMESPACE