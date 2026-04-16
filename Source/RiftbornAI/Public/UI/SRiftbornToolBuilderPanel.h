// SRiftbornToolBuilderPanel.h
// Editor panel where users author, test, and publish custom RiftbornAI tools.
// Visually a sibling of SRiftbornCopilotPanel — shares palette, fonts, and
// radii via CopilotPanelColors.h and FRiftbornAIStyle.

#pragma once

#include "CoreMinimal.h"
#include "Tools/RiftToolModel.h"
#include "Tools/RiftMarketplaceClient.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

enum class EToolBuilderMode : uint8
{
	Tools,
	Marketplace
};

// Inline browse filters. The price chip is independent of the category
// selection, so two small enums instead of one combined one.
enum class EMarketplacePriceFilter : uint8
{
	All,
	Free,
	Paid,
	Installed,
};

enum class EMarketplaceSort : uint8
{
	Popular,
	Recent,
	PriceAsc,
	PriceDesc,
	Rating,
};

class SCheckBox;
class SEditableTextBox;
class SMultiLineEditableTextBox;
template<typename> class SComboBox;

class RIFTBORNAI_API SRiftbornToolBuilderPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRiftbornToolBuilderPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// --- layout builders ---
	TSharedRef<SWidget> BuildHeader();
	TSharedRef<SWidget> BuildLeftRail();
	TSharedRef<SWidget> BuildEditor();
	TSharedRef<SWidget> BuildEditorForm();
	TSharedRef<SWidget> BuildEmptyState();
	TSharedRef<SWidget> BuildTestConsole();
	TSharedRef<SWidget> BuildMarketplaceView();
	TSharedRef<SWidget> BuildMarketplaceHeader();
	TSharedRef<SWidget> BuildMarketplaceSidebar();
	TSharedRef<SWidget> BuildMarketplaceBody();
	TSharedRef<SWidget> BuildMarketplaceEmptyState();
	TSharedRef<SWidget> BuildMarketplaceSkeletonRows();
	TSharedRef<SWidget> BuildFeaturedHero(const FRiftListingSummary& L);
	TSharedRef<SWidget> BuildListingThumbnail(const FRiftListingSummary& L, float SizePx);
	TSharedRef<SWidget> BuildListingDetailExpansion(const FRiftListingSummary& L);
	TSharedRef<SWidget> BuildModeSwitcher();
	TSharedRef<ITableRow> MakeListingRow(TSharedPtr<FRiftListingSummary> Item, const TSharedRef<STableViewBase>& Table);
	void RebuildVisibleListings();  // apply filter/sort to Listings → VisibleListings

	// --- list row ---
	TSharedRef<ITableRow> MakeToolRow(TSharedPtr<FRiftTool> Item, const TSharedRef<STableViewBase>& Table);
	void OnToolSelected(TSharedPtr<FRiftTool> Item, ESelectInfo::Type Info);

	// --- actions ---
	FReply OnNewToolClicked();
	FReply OnSaveClicked();
	FReply OnDeleteClicked();
	FReply OnRunClicked();
	FReply OnPublishClicked();
	FReply OnRefreshMarketplaceClicked();
	FReply OnInstallClicked(TSharedPtr<FRiftListingSummary> Listing);
	FReply OnSyncAllClicked();
	void   OnEnabledToggled(ECheckBoxState NewState);
	void RefreshToolList();
	void PushFormFromModel();
	void PullFormIntoModel();

	// --- state ---
	TArray<TSharedPtr<FRiftTool>> Tools;
	TSharedPtr<FRiftTool>         Selected;       // pointer into Tools or a fresh unsaved tool
	bool                          bUnsavedNew = false;
	FString                       StatusText;

	// --- widgets (form fields) ---
	TSharedPtr<SListView<TSharedPtr<FRiftTool>>>  ToolListView;
	TSharedPtr<SEditableTextBox>                  NameBox;
	TSharedPtr<SMultiLineEditableTextBox>         DescBox;
	TSharedPtr<SMultiLineEditableTextBox>         SchemaBox;
	TSharedPtr<SMultiLineEditableTextBox>         HandlerBox;

	TArray<TSharedPtr<FString>>                   KindOptions;
	TSharedPtr<FString>                           KindCurrent;
	TSharedPtr<SComboBox<TSharedPtr<FString>>>    KindCombo;

	TArray<TSharedPtr<FString>>                   BlastOptions;
	TSharedPtr<FString>                           BlastCurrent;
	TSharedPtr<SComboBox<TSharedPtr<FString>>>    BlastCombo;

	// Test console
	TSharedPtr<SMultiLineEditableTextBox>         TestArgsBox;
	TSharedPtr<SMultiLineEditableTextBox>         TestOutputBox;
	bool                                          bTestSucceeded = false;

	TSharedPtr<SCheckBox>                         EnabledToggle;

	// Marketplace
	EToolBuilderMode                              Mode = EToolBuilderMode::Marketplace;
	TSharedPtr<SEditableTextBox>                  MarketSearchBox;
	TArray<TSharedPtr<FRiftListingSummary>>       Listings;         // full set from server
	TArray<TSharedPtr<FRiftListingSummary>>       VisibleListings;  // after filter/sort
	TArray<FRiftCategoryCount>                    Categories;
	TSharedPtr<SListView<TSharedPtr<FRiftListingSummary>>> ListingsView;
	TSharedPtr<class SBox>                        HeroSlot;          // replaced on refresh
	TSharedPtr<class SBox>                        SidebarSlot;       // replaced on refresh
	bool                                          bMarketBusy = false;
	bool                                          bMarketEverFetched = false;  // drives first-load empty vs. loaded-empty state
	FString                                       MarketError;
	EMarketplacePriceFilter                       PriceFilter    = EMarketplacePriceFilter::All;
	EMarketplaceSort                              SortMode       = EMarketplaceSort::Popular;
	FString                                       CategoryFilter;  // empty = all categories
	TSet<FString>                                 ExpandedListingIds;  // rows the user has clicked to expand
};
