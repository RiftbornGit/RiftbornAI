// Copyright RiftbornAI. All Rights Reserved.
// AgentTaskRunner.h — thin view-model for the copilot panel.
//
// Historically this class owned a full plan/build/PIE/repair state machine
// that ran in parallel to FAgenticLoopRunner. Nothing actually drove that
// path — every copilot request funneled through
// FRiftbornAgentCore::StartTask → FAgenticLoopRunner::RunAsyncWithProgress,
// and the UI used FAgentTaskRunner only as a stable event-stream handle for
// SAgentTimeline. The parallel execution machinery has been removed.
//
// The surviving responsibilities:
//   1. Own the per-panel FAgentEventStream.
//   2. Expose a coarse ETaskRunnerState enum + OnStateChanged delegate so
//      the timeline widget can color its header and react to transitions.

#pragma once

#include "CoreMinimal.h"
#include "Agent/AgentEvent.h"
#include "Agent/AgentEventStream.h"

/**
 * Coarse display state for the copilot timeline header.
 * Drives header coloring and progress hints — not an execution machine.
 */
UENUM()
enum class ETaskRunnerState : uint8
{
	Idle,
	Setup,
	Planning,
	Executing,
	Building,
	Testing,
	Probing,
	Repairing,
	Stopped
};

/**
 * FAgentTaskRunner — view-model for the copilot panel.
 *
 * Owns the FAgentEventStream that SAgentTimeline renders. The actual task
 * execution runs in FAgenticLoopRunner; this object exists so Slate widgets
 * have a stable TSharedPtr handle to bind to, and so the panel has a single
 * place to push UI state updates.
 */
class RIFTBORNAI_API FAgentTaskRunner : public TSharedFromThis<FAgentTaskRunner>
{
public:
	FAgentTaskRunner() = default;
	~FAgentTaskRunner() = default;

	/** Get the event stream, or nullptr if none has been created yet. */
	TSharedPtr<FAgentEventStream> GetEventStream() const { return EventStream; }

	/** Lazily create a transient event stream on first access. Safe to call
	 *  repeatedly — returns the existing stream once created. */
	TSharedPtr<FAgentEventStream> EnsureEventStream();

	/** Current coarse display state. */
	ETaskRunnerState GetState() const { return State; }

	/** Update the display state and broadcast OnStateChanged if it changed. */
	void SetState(ETaskRunnerState NewState);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnStateChanged, ETaskRunnerState);
	FOnStateChanged OnStateChanged;

private:
	TSharedPtr<FAgentEventStream> EventStream;
	ETaskRunnerState State = ETaskRunnerState::Idle;
};
