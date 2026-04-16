// SRiftbornTodoListPanel.h
// Live mirror of FRiftbornTodoList. Refreshes on the OnTodoListChanged
// delegate so the user sees todos appear / flip status as the LLM updates
// them via update_todo_list.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

struct FRiftbornTodoItem;

class RIFTBORNAI_API SRiftbornTodoListPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRiftbornTodoListPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SRiftbornTodoListPanel() override;

private:
	void Refresh();
	TSharedRef<ITableRow> MakeRow(TSharedPtr<FRiftbornTodoItem> Item, const TSharedRef<STableViewBase>& Owner);
	FReply OnClearClicked();

	TArray<TSharedPtr<FRiftbornTodoItem>>             Items;
	TSharedPtr<SListView<TSharedPtr<FRiftbornTodoItem>>> ListView;
	FDelegateHandle                                   OnChangedHandle;
};
