#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/DateTime.h"
#include "Misc/ScopeLock.h"
#include "Widgets/SWindow.h"

/**
 * Lightweight, thread-safe snapshot of Slate/editor GUI state.
 * Used to surface dialogs, popups, notifications, and menus to the bridge.
 */
class FEditorGuiState
{
private:
	static constexpr int32 MaxSnapshotWindows = 64;
	static constexpr int32 MaxSnapshotChildWindowsPerParent = 24;

	struct FWindowEntry
	{
		FString WindowId;
		FString ParentWindowId;
		FString Title;
		FString Classification;
		bool bModal = false;
		bool bVisible = false;
		bool bMinimized = false;
		bool bRegularWindow = false;
		bool bNotification = false;
		bool bRootEditorWindow = false;
		bool bHasSizingFrame = false;
		bool bHasMinimizeBox = false;
		bool bActiveTopLevel = false;
		bool bActiveModal = false;
		FSlateRect Rect;
	};

	struct FEventEntry
	{
		FString TimestampUtc;
		FString Source;
		FString Kind;
		FString Title;
		FString Classification;
		FString Action;
	};

	struct FSnapshot
	{
		FString TimestampUtc;
		bool bSlateInitialized = false;
		bool bMenusVisible = false;
		int32 InteractiveTopLevelCount = 0;
		int32 NotificationWindowCount = 0;
		FString RootEditorWindowId;
		FString ActiveTopLevelTitle;
		FString ActiveTopLevelWindowId;
		FString ActiveModalTitle;
		FString ActiveModalWindowId;
		TArray<FWindowEntry> TopLevelWindows;
		TArray<FWindowEntry> NotificationWindows;
	};

	struct FState
	{
		FCriticalSection Mutex;
		FSnapshot Snapshot;
		TArray<FEventEntry> RecentEvents;
		FString LastActiveModalTitle;
		bool bLastMenusVisible = false;
		int32 LastNotificationWindowCount = 0;
	};

	static FState& GetState()
	{
		static FState State;
		return State;
	}

public:
	static FString MakeWindowId(const SWindow* WindowPtr)
	{
		return WindowPtr ? FString::Printf(TEXT("window:%p"), WindowPtr) : TEXT("");
	}

	// Exposed for ModalDismisser — classifies a window title string
	static FString ClassifyWindowTitle(const FString& Title, bool bNotificationWindow)
	{
		const FString Lower = Title.TrimStartAndEnd().ToLower();
		if (Lower.IsEmpty())
		{
			return bNotificationWindow ? TEXT("notification_window") : TEXT("untitled");
		}
		if (Lower.Contains(TEXT("memory pressure")))
		{
			return TEXT("memory_pressure");
		}
		if (Lower.Contains(TEXT("restore packages")))
		{
			return TEXT("restore_packages");
		}
		if (Lower.Contains(TEXT("asset manager settings")))
		{
			return TEXT("asset_manager_settings");
		}
		if (Lower.Contains(TEXT("water body collision")))
		{
			return TEXT("water_body_collision");
		}
		if (Lower.Contains(TEXT("blueprint compilation")) || Lower.Contains(TEXT("blueprint compile")))
		{
			return TEXT("blueprint_compile_errors");
		}
		// BSP / Volume geometry rebuild prompt — UE pops this when loading old maps.
		// We don't want to rebuild random Electric Dreams / template geometry.
		if (Lower.Contains(TEXT("bsp")) || Lower.Contains(TEXT("volume geometry")) || Lower.Contains(TEXT("rebuild geometry")))
		{
			return TEXT("bsp_geometry_rebuild");
		}
		if (Lower.Contains(TEXT("crash")))
		{
			return TEXT("crash_reporter");
		}
		if (Lower.Contains(TEXT("save")) && Lower.Contains(TEXT("package")))
		{
			return TEXT("save_packages");
		}
		if (Lower.Contains(TEXT("notification")))
		{
			return TEXT("notification_window");
		}
		return TEXT("generic");
	}

private:
	static TSharedPtr<FJsonObject> BuildWindowJson(const FWindowEntry& Window)
	{
		TSharedPtr<FJsonObject> WindowObj = MakeShared<FJsonObject>();
		WindowObj->SetStringField(TEXT("window_id"), Window.WindowId);
		WindowObj->SetStringField(TEXT("parent_window_id"), Window.ParentWindowId);
		WindowObj->SetStringField(TEXT("title"), Window.Title);
		WindowObj->SetStringField(TEXT("classification"), Window.Classification);
		WindowObj->SetBoolField(TEXT("modal"), Window.bModal);
		WindowObj->SetBoolField(TEXT("visible"), Window.bVisible);
		WindowObj->SetBoolField(TEXT("minimized"), Window.bMinimized);
		WindowObj->SetBoolField(TEXT("regular_window"), Window.bRegularWindow);
		WindowObj->SetBoolField(TEXT("notification_window"), Window.bNotification);
		WindowObj->SetBoolField(TEXT("root_editor_window"), Window.bRootEditorWindow);
		WindowObj->SetBoolField(TEXT("has_sizing_frame"), Window.bHasSizingFrame);
		WindowObj->SetBoolField(TEXT("has_minimize_box"), Window.bHasMinimizeBox);
		WindowObj->SetBoolField(TEXT("active_top_level"), Window.bActiveTopLevel);
		WindowObj->SetBoolField(TEXT("active_modal"), Window.bActiveModal);

		TSharedPtr<FJsonObject> RectObj = MakeShared<FJsonObject>();
		RectObj->SetNumberField(TEXT("left"), Window.Rect.Left);
		RectObj->SetNumberField(TEXT("top"), Window.Rect.Top);
		RectObj->SetNumberField(TEXT("right"), Window.Rect.Right);
		RectObj->SetNumberField(TEXT("bottom"), Window.Rect.Bottom);
		RectObj->SetNumberField(TEXT("width"), Window.Rect.Right - Window.Rect.Left);
		RectObj->SetNumberField(TEXT("height"), Window.Rect.Bottom - Window.Rect.Top);
		WindowObj->SetObjectField(TEXT("screen_rect"), RectObj);
		return WindowObj;
	}

	static void AddEventUnlocked(FState& State, const FString& Source, const FString& Kind, const FString& Title, const FString& Action)
	{
		FEventEntry Event;
		Event.TimestampUtc = FDateTime::UtcNow().ToIso8601();
		Event.Source = Source;
		Event.Kind = Kind;
		Event.Title = Title;
		Event.Classification = ClassifyWindowTitle(Title, Kind == TEXT("notification"));
		Event.Action = Action;
		State.RecentEvents.Add(Event);
		if (State.RecentEvents.Num() > 40)
		{
			State.RecentEvents.RemoveAt(0, State.RecentEvents.Num() - 40, EAllowShrinking::No);
		}
	}

	static FWindowEntry MakeWindowEntry(
		const TSharedRef<SWindow>& Window,
		const TSharedPtr<SWindow>& RootEditorWindow,
		const TSharedPtr<SWindow>& ActiveTopLevelWindow,
		const TSharedPtr<SWindow>& ActiveModalWindow,
		const TSet<const SWindow*>& NotificationWindowPtrs)
	{
		const SWindow* WindowPtr = &Window.Get();
		const TSharedPtr<SWindow> ParentWindow = Window->GetParentWindow();

		FWindowEntry Entry;
		Entry.WindowId = MakeWindowId(WindowPtr);
		Entry.ParentWindowId = ParentWindow.IsValid() ? MakeWindowId(ParentWindow.Get()) : TEXT("");
		Entry.Title = Window->GetTitle().ToString();
		Entry.bModal = Window->IsModalWindow();
		Entry.bVisible = Window->IsVisible();
		Entry.bMinimized = Window->IsWindowMinimized();
		Entry.bRegularWindow = Window->IsRegularWindow();
		Entry.bNotification = NotificationWindowPtrs.Contains(WindowPtr);
		Entry.bRootEditorWindow = RootEditorWindow.IsValid() && RootEditorWindow.Get() == WindowPtr;
		Entry.bHasSizingFrame = Window->HasSizingFrame();
		Entry.bHasMinimizeBox = Window->HasMinimizeBox();
		Entry.bActiveTopLevel = ActiveTopLevelWindow.IsValid() && ActiveTopLevelWindow.Get() == WindowPtr;
		Entry.bActiveModal = ActiveModalWindow.IsValid() && ActiveModalWindow.Get() == WindowPtr;
		Entry.Rect = Window->GetRectInScreen();
		Entry.Classification = ClassifyWindowTitle(Entry.Title, Entry.bNotification);
		return Entry;
	}

	static void AddWarningIfMissing(TArray<FString>& Warnings, const TCHAR* Code)
	{
		const FString Warning(Code);
		if (!Warnings.Contains(Warning))
		{
			Warnings.Add(Warning);
		}
	}

	static void AppendWindowTreeEntries(
		TArray<FWindowEntry>& OutEntries,
		TSet<const SWindow*>& SeenWindows,
		const TSharedRef<SWindow>& Window,
		const TSharedPtr<SWindow>& RootEditorWindow,
		const TSharedPtr<SWindow>& ActiveTopLevelWindow,
		const TSharedPtr<SWindow>& ActiveModalWindow,
		const TSet<const SWindow*>& NotificationWindowPtrs)
	{
		TArray<TSharedRef<SWindow>> PendingWindows;
		PendingWindows.Add(Window);

		while (PendingWindows.Num() > 0 && OutEntries.Num() < MaxSnapshotWindows)
		{
			const TSharedRef<SWindow> CurrentWindow = PendingWindows.Pop(EAllowShrinking::No);
			const SWindow* WindowPtr = &CurrentWindow.Get();
			if (SeenWindows.Contains(WindowPtr))
			{
				continue;
			}

			SeenWindows.Add(WindowPtr);
			OutEntries.Add(MakeWindowEntry(CurrentWindow, RootEditorWindow, ActiveTopLevelWindow, ActiveModalWindow, NotificationWindowPtrs));

			const TArray<TSharedRef<SWindow>>& ChildWindows = CurrentWindow->GetChildWindows();
			const int32 ChildLimit = FMath::Min(ChildWindows.Num(), MaxSnapshotChildWindowsPerParent);
			for (int32 ChildIndex = ChildLimit - 1; ChildIndex >= 0; --ChildIndex)
			{
				const TSharedRef<SWindow>& ChildWindow = ChildWindows[ChildIndex];
				if (&ChildWindow.Get() == WindowPtr || SeenWindows.Contains(&ChildWindow.Get()))
				{
					continue;
				}

				PendingWindows.Add(ChildWindow);
			}
		}
	}

public:
	static void CaptureSlateSnapshot(const FString& Source = TEXT("capture"))
	{
		FState& State = GetState();
		FScopeLock Lock(&State.Mutex);

		State.Snapshot = FSnapshot();
		State.Snapshot.TimestampUtc = FDateTime::UtcNow().ToIso8601();

		if (!FSlateApplication::IsInitialized())
		{
			State.Snapshot.bSlateInitialized = false;
			return;
		}

		FSlateApplication& SlateApp = FSlateApplication::Get();
		State.Snapshot.bSlateInitialized = true;
		State.Snapshot.bMenusVisible = SlateApp.AnyMenusVisible();

		const TSharedPtr<SWindow> RootEditorWindow = FGlobalTabmanager::Get()->GetRootWindow();
		const TSharedPtr<SWindow> ActiveTopLevelWindow = SlateApp.GetActiveTopLevelWindow();
		const TSharedPtr<SWindow> ActiveModalWindow = SlateApp.GetActiveModalWindow();
		State.Snapshot.RootEditorWindowId = RootEditorWindow.IsValid() ? MakeWindowId(RootEditorWindow.Get()) : TEXT("");
		State.Snapshot.ActiveTopLevelTitle = ActiveTopLevelWindow.IsValid() ? ActiveTopLevelWindow->GetTitle().ToString() : TEXT("");
		State.Snapshot.ActiveTopLevelWindowId = ActiveTopLevelWindow.IsValid() ? MakeWindowId(ActiveTopLevelWindow.Get()) : TEXT("");
		State.Snapshot.ActiveModalTitle = ActiveModalWindow.IsValid() ? ActiveModalWindow->GetTitle().ToString() : TEXT("");
		State.Snapshot.ActiveModalWindowId = ActiveModalWindow.IsValid() ? MakeWindowId(ActiveModalWindow.Get()) : TEXT("");
		State.Snapshot.InteractiveTopLevelCount = SlateApp.GetInteractiveTopLevelWindows().Num();

		TArray<TSharedRef<SWindow>> NotificationWindows;
		FSlateNotificationManager::Get().GetWindows(NotificationWindows);
		State.Snapshot.NotificationWindowCount = NotificationWindows.Num();

		TSet<const SWindow*> NotificationWindowPtrs;
		for (const TSharedRef<SWindow>& NotificationWindow : NotificationWindows)
		{
			NotificationWindowPtrs.Add(&NotificationWindow.Get());
		}

		const TArray<TSharedRef<SWindow>> TopLevelWindows = SlateApp.GetTopLevelWindows();
		TSet<const SWindow*> SeenWindows;
		State.Snapshot.TopLevelWindows.Reserve(FMath::Min(TopLevelWindows.Num(), MaxSnapshotWindows));
		for (const TSharedRef<SWindow>& Window : TopLevelWindows)
		{
			if (State.Snapshot.TopLevelWindows.Num() >= MaxSnapshotWindows)
			{
				break;
			}

			const SWindow* WindowPtr = &Window.Get();
			if (SeenWindows.Contains(WindowPtr))
			{
				continue;
			}

			SeenWindows.Add(WindowPtr);
			State.Snapshot.TopLevelWindows.Add(MakeWindowEntry(Window, RootEditorWindow, ActiveTopLevelWindow, ActiveModalWindow, NotificationWindowPtrs));
		}

		State.Snapshot.NotificationWindows.Reserve(NotificationWindows.Num());
		for (const TSharedRef<SWindow>& Window : NotificationWindows)
		{
			if (SeenWindows.Contains(&Window.Get()))
			{
				continue;
			}
			State.Snapshot.NotificationWindows.Add(MakeWindowEntry(Window, RootEditorWindow, ActiveTopLevelWindow, ActiveModalWindow, NotificationWindowPtrs));
		}

		if (!State.Snapshot.ActiveModalTitle.IsEmpty() && State.Snapshot.ActiveModalTitle != State.LastActiveModalTitle)
		{
			AddEventUnlocked(State, Source, TEXT("modal"), State.Snapshot.ActiveModalTitle, TEXT("observed"));
		}
		if (State.Snapshot.bMenusVisible && !State.bLastMenusVisible)
		{
			AddEventUnlocked(State, Source, TEXT("menu"), TEXT("Application Menu Stack"), TEXT("observed"));
		}
		if (State.Snapshot.NotificationWindowCount > 0 && State.LastNotificationWindowCount == 0)
		{
			AddEventUnlocked(
				State,
				Source,
				TEXT("notification"),
				FString::Printf(TEXT("%d notification window(s)"), State.Snapshot.NotificationWindowCount),
				TEXT("observed"));
		}

		State.LastActiveModalTitle = State.Snapshot.ActiveModalTitle;
		State.bLastMenusVisible = State.Snapshot.bMenusVisible;
		State.LastNotificationWindowCount = State.Snapshot.NotificationWindowCount;
	}

	static void RecordWindowAction(const FString& Source, const FString& Kind, const FString& Title, const FString& Action)
	{
		FState& State = GetState();
		FScopeLock Lock(&State.Mutex);
		AddEventUnlocked(State, Source, Kind, Title, Action);
	}

	static TSharedPtr<FJsonObject> BuildSnapshotJson(int32 MaxWindows = 24, int32 MaxNotifications = 12, int32 MaxEvents = 20)
	{
		MaxWindows = FMath::Clamp(MaxWindows, 1, 100);
		MaxNotifications = FMath::Clamp(MaxNotifications, 1, 100);
		MaxEvents = FMath::Clamp(MaxEvents, 1, 100);

		FState& State = GetState();
		FScopeLock Lock(&State.Mutex);

		TSharedPtr<FJsonObject> RootObj = MakeShared<FJsonObject>();
		RootObj->SetStringField(TEXT("timestamp"), State.Snapshot.TimestampUtc);
		RootObj->SetBoolField(TEXT("slate_initialized"), State.Snapshot.bSlateInitialized);
		RootObj->SetBoolField(TEXT("menus_visible"), State.Snapshot.bMenusVisible);
		RootObj->SetBoolField(TEXT("modal_open"), !State.Snapshot.ActiveModalTitle.IsEmpty());
		RootObj->SetStringField(TEXT("root_editor_window_id"), State.Snapshot.RootEditorWindowId);
		RootObj->SetStringField(TEXT("active_top_level_title"), State.Snapshot.ActiveTopLevelTitle);
		RootObj->SetStringField(TEXT("active_top_level_window_id"), State.Snapshot.ActiveTopLevelWindowId);
		RootObj->SetStringField(TEXT("active_modal_title"), State.Snapshot.ActiveModalTitle);
		RootObj->SetStringField(TEXT("active_modal_window_id"), State.Snapshot.ActiveModalWindowId);
		RootObj->SetNumberField(TEXT("interactive_top_level_count"), State.Snapshot.InteractiveTopLevelCount);
		RootObj->SetNumberField(TEXT("notification_window_count"), State.Snapshot.NotificationWindowCount);

		TArray<TSharedPtr<FJsonValue>> WindowArray;
		for (int32 Index = 0; Index < State.Snapshot.TopLevelWindows.Num() && Index < MaxWindows; ++Index)
		{
			WindowArray.Add(MakeShared<FJsonValueObject>(BuildWindowJson(State.Snapshot.TopLevelWindows[Index])));
		}
		RootObj->SetArrayField(TEXT("top_level_windows"), WindowArray);

		TArray<TSharedPtr<FJsonValue>> NotificationArray;
		for (int32 Index = 0; Index < State.Snapshot.NotificationWindows.Num() && Index < MaxNotifications; ++Index)
		{
			NotificationArray.Add(MakeShared<FJsonValueObject>(BuildWindowJson(State.Snapshot.NotificationWindows[Index])));
		}
		RootObj->SetArrayField(TEXT("notification_windows"), NotificationArray);

		TArray<TSharedPtr<FJsonValue>> EventArray;
		const int32 EventStartIndex = FMath::Max(0, State.RecentEvents.Num() - MaxEvents);
		for (int32 Index = EventStartIndex; Index < State.RecentEvents.Num(); ++Index)
		{
			const FEventEntry& Event = State.RecentEvents[Index];
			TSharedPtr<FJsonObject> EventObj = MakeShared<FJsonObject>();
			EventObj->SetStringField(TEXT("timestamp"), Event.TimestampUtc);
			EventObj->SetStringField(TEXT("source"), Event.Source);
			EventObj->SetStringField(TEXT("kind"), Event.Kind);
			EventObj->SetStringField(TEXT("title"), Event.Title);
			EventObj->SetStringField(TEXT("classification"), Event.Classification);
			EventObj->SetStringField(TEXT("action"), Event.Action);
			EventArray.Add(MakeShared<FJsonValueObject>(EventObj));
		}
		RootObj->SetArrayField(TEXT("recent_events"), EventArray);

		return RootObj;
	}

	static void AppendWarningCodes(TArray<FString>& InOutWarnings)
	{
		FState& State = GetState();
		FScopeLock Lock(&State.Mutex);

		if (!State.Snapshot.bSlateInitialized)
		{
			AddWarningIfMissing(InOutWarnings, TEXT("no_slate_application"));
			return;
		}

		if (State.Snapshot.bMenusVisible)
		{
			AddWarningIfMissing(InOutWarnings, TEXT("gui_menus_visible"));
		}
		if (!State.Snapshot.ActiveModalTitle.IsEmpty())
		{
			AddWarningIfMissing(InOutWarnings, TEXT("gui_modal_open"));
		}
		if (State.Snapshot.NotificationWindowCount > 0)
		{
			AddWarningIfMissing(InOutWarnings, TEXT("gui_notifications_visible"));
		}

		const auto MaybeAddClassificationWarning = [&InOutWarnings](const FString& Classification)
		{
			if (Classification == TEXT("memory_pressure"))
			{
				AddWarningIfMissing(InOutWarnings, TEXT("gui_memory_pressure"));
			}
			else if (Classification == TEXT("restore_packages"))
			{
				AddWarningIfMissing(InOutWarnings, TEXT("gui_restore_packages"));
			}
			else if (Classification == TEXT("asset_manager_settings"))
			{
				AddWarningIfMissing(InOutWarnings, TEXT("gui_asset_manager_settings"));
			}
			else if (Classification == TEXT("water_body_collision"))
			{
				AddWarningIfMissing(InOutWarnings, TEXT("gui_water_body_collision"));
			}
			else if (Classification == TEXT("blueprint_compile_errors"))
			{
				AddWarningIfMissing(InOutWarnings, TEXT("gui_blueprint_compile_errors"));
			}
			else if (Classification == TEXT("crash_reporter"))
			{
				AddWarningIfMissing(InOutWarnings, TEXT("gui_crash_reporter"));
			}
		};

		for (const FWindowEntry& Window : State.Snapshot.TopLevelWindows)
		{
			MaybeAddClassificationWarning(Window.Classification);
		}
		for (const FWindowEntry& Window : State.Snapshot.NotificationWindows)
		{
			MaybeAddClassificationWarning(Window.Classification);
		}
	}
};
