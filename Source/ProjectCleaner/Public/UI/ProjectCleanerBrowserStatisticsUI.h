﻿#pragma once

// Engine Headers
#include "StructsContainer.h"
#include "Widgets/SCompoundWidget.h"
#include "CoreMinimal.h"

/**
 * @brief Shows statistics info (UnusedAssets count, Total Size, EmptyFolder count, etc.)
 */
class SProjectCleanerBrowserStatisticsUI : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SProjectCleanerBrowserStatisticsUI) {}
		SLATE_ARGUMENT(FCleaningStats, Stats);
	SLATE_END_ARGS()
	void Construct(const FArguments& InArgs);
	void SetStats(const FCleaningStats& NewStats);
	FCleaningStats GetStats() const;
	void RefreshUIContent();
private:
	/**
	 * @brief Statistics data
	 */
	FCleaningStats Stats;
	TSharedRef<SWidget> WidgetRef = SNullWidget::NullWidget;
	
};