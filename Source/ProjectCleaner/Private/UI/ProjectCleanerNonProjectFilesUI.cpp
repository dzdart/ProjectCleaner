﻿#include "UI/ProjectCleanerNonProjectFilesUI.h"
#include "StructsContainer.h"

#define LOCTEXT_NAMESPACE "FProjectCleanerModule"

#pragma optimize("", off)

void SProjectCleanerNonProjectFilesUI::Construct(const FArguments& InArgs)
{
	NonProjectFiles = InArgs._NonProjectFiles;

	for(const auto& NonProjectFile : NonProjectFiles)
	{
		auto Obj = NewObject<UNonProjectFilesUIStruct>();
		if(!Obj) continue;
		Obj->FileName = NonProjectFile.FileName;
		Obj->FilePath = NonProjectFile.FilePath;
		NonProjectFilesUIStructs.Add(Obj);		
	}
	
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
            .AutoWrapText(true)
            .Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Light.ttf"),20))
            .Text(LOCTEXT("NonProjectFiles", "Non project files"))
		]
		+SVerticalBox::Slot()
		.Padding(FMargin{0.0f, 10.0f})
        .AutoHeight()
        [
            SNew(STextBlock)
            .AutoWrapText(true)
            .Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Light.ttf"),8))
            .Text(LOCTEXT("dblclickonrow", "Double click on row to open in Explorer"))
        ]
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin{0.0f, 20.0f})
		[
			SNew(SListView<TWeakObjectPtr<UNonProjectFilesUIStruct>>)
			.ListItemsSource(&NonProjectFilesUIStructs)
			.SelectionMode(ESelectionMode::SingleToggle)
			.OnGenerateRow(this, &SProjectCleanerNonProjectFilesUI::OnGenerateRow)
			.OnMouseButtonDoubleClick_Raw(this, &SProjectCleanerNonProjectFilesUI::OnMouseDoubleClick)
			.HeaderRow
			(
				SNew(SHeaderRow)
	            + SHeaderRow::Column(FName("FileName"))
	            .FillWidth(0.3f)
	            .HeaderContentPadding(FMargin{5.0f})
	            [
	                SNew(STextBlock)
	                .Text(LOCTEXT("NameColumn", "FileName"))
	            ]
	            + SHeaderRow::Column(FName("FilePath"))
	            .FillWidth(0.7f)
	            .HeaderContentPadding(FMargin{5.0f})
	            [
	                SNew(STextBlock)
	                .Text(LOCTEXT("PathColumn", "FilePath"))
	            ]
			)
		]
	];
}

void SProjectCleanerNonProjectFilesUI::SetNonProjectFiles(const TArray<FNonProjectFile> NewNonProjectFiles)
{
	NonProjectFiles = NewNonProjectFiles;
	NonProjectFilesUIStructs.Reset();

	for(const auto& NonProjectFile : NonProjectFiles)
	{
		auto Obj = NewObject<UNonProjectFilesUIStruct>();
		if(!Obj) continue;
		Obj->FileName = NonProjectFile.FileName;
		Obj->FilePath = NonProjectFile.FilePath;
		NonProjectFilesUIStructs.Add(Obj);		
	}
}

TSharedRef<ITableRow> SProjectCleanerNonProjectFilesUI::OnGenerateRow(TWeakObjectPtr<UNonProjectFilesUIStruct> InItem,
                                                                      const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SNonProjectFileUISelectionRow, OwnerTable).SelectedRowItem(InItem);
}

void SProjectCleanerNonProjectFilesUI::OnMouseDoubleClick(TWeakObjectPtr<UNonProjectFilesUIStruct> Item)
{
	if(!Item.IsValid()) return;

	const auto DirectoryPath = FPaths::GetPath(Item.Get()->FilePath);
	if (FPaths::DirectoryExists(DirectoryPath))
	{
		FPlatformProcess::ExploreFolder(*DirectoryPath);
	}
}
#pragma optimize("", on)

#undef LOCTEXT_NAMESPACE