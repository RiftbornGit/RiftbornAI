// Copyright Riftborn AI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RiftbornCertSubsystem.generated.h"

/**
 * Event severity for certification events.
 * Maps to Python-side severity for filtering.
 */
UENUM(BlueprintType)
enum class ECertEventSeverity : uint8
{
	Info,       // Informational (BeginPlay, Init, etc.)
	Warning,    // Non-fatal issue detected
	Error,      // Runtime error that should fail certification
	Critical    // Crash-level - certification fails immediately
};

/**
 * Event type for certification events.
 * Used for filtering and routing on the collector side.
 */
UENUM(BlueprintType)
enum class ECertEventType : uint8
{
	// Session lifecycle
	SessionStart,       // PIE started, certification session begins
	SessionEnd,         // PIE ended, flush all events
	
	// L2: Class lifecycle
	ClassInstantiated,  // UObject created (class name, path)
	BeginPlayStarted,   // BeginPlay() entered
	BeginPlayCompleted, // BeginPlay() exited without error
	BeginPlayFailed,    // BeginPlay() threw or crashed
	
	// L3: Gameplay
	PawnPossessed,      // PlayerController possessed a pawn
	MovementDetected,   // Pawn moved (delta > threshold)
	InputReceived,      // Input action consumed
	
	// L4: Abilities
	AbilityGranted,     // Ability spec added to ASC
	AbilityActivated,   // ActivateAbility entered
	AbilityEnded,       // EndAbility called (success or cancel)
	AbilityFailed,      // Activation failed (cost, cooldown, tags)
	
	// L5: Network
	ReplicationEvent,   // Property replicated (server->client confirm)
	RPCExecuted,        // RPC called and completed
	PlayerJoined,       // Player connected to session
	
	// Errors
	RuntimeError,       // Caught error during certification window
	EnsureFailed,       // Ensure condition failed
	
	// Custom
	Custom              // User-defined event with payload
};

/**
 * A single certification event.
 * Immutable once created. Stored in memory buffer during PIE.
 */
USTRUCT(BlueprintType)
struct RIFTBORNAI_API FCertEvent
{
	GENERATED_BODY()

	/** Unique event ID within this session */
	UPROPERTY()
	int32 EventId = 0;

	/** Session ID (set at PIE start) */
	UPROPERTY()
	FString SessionId;

	/** Job ID (passed in from bridge, identifies the generation task) */
	UPROPERTY()
	FString JobId;

	/** Event type for filtering */
	UPROPERTY()
	ECertEventType EventType = ECertEventType::Custom;

	/** Severity level */
	UPROPERTY()
	ECertEventSeverity Severity = ECertEventSeverity::Info;

	/** Human-readable event name (e.g., "AMyGameMode::BeginPlay") */
	UPROPERTY()
	FString EventName;

	/** Structured payload as JSON string */
	UPROPERTY()
	FString PayloadJson;

	/** Tags for filtering (e.g., "generated", "gamemode", "character") */
	UPROPERTY()
	TArray<FString> Tags;

	/** Timestamp in seconds since session start */
	UPROPERTY()
	double TimestampSeconds = 0.0;

	/** Frame number when event was emitted */
	UPROPERTY()
	int64 FrameNumber = 0;

	/** Convert to JSON for transport */
	FString ToJson() const;

	/** Create from JSON */
	static FCertEvent FromJson(const FString& Json);
};

/**
 * URiftbornCertSubsystem - Runtime Certification Bus
 * 
 * A GameInstanceSubsystem that collects structured certification events
 * during PIE sessions. Replaces fragile log-parsing with deterministic
 * event emission.
 * 
 * Usage from generated code:
 *   URiftbornCertSubsystem::EmitBeginPlay(this);
 * 
 * Usage from Python bridge:
 *   events = cert_subsystem.flush_events()
 * 
 * Lifecycle:
 *   1. PIE starts -> SessionStart event emitted
 *   2. Generated classes call Emit*() during runtime
 *   3. PIE ends or bridge calls FlushEvents() -> all events returned
 *   4. Collector validates events against expectations
 */
UCLASS()
class RIFTBORNAI_API URiftbornCertSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ========================================================================
	// Subsystem Lifecycle
	// ========================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ========================================================================
	// Session Management
	// ========================================================================

	/**
	 * Start a certification session. Called automatically on PIE start,
	 * but can be called explicitly to set JobId.
	 * 
	 * @param JobId - Identifier linking this session to a generation task
	 * @return SessionId - Unique identifier for this certification session
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	FString StartSession(const FString& JobId = TEXT(""));

	/**
	 * End the certification session and prepare events for collection.
	 * Called automatically on PIE end.
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	void EndSession();

	/**
	 * Check if a certification session is active.
	 */
	UFUNCTION(BlueprintPure, Category = "RiftbornAI|Certification")
	bool IsSessionActive() const { return bSessionActive; }

	/**
	 * Get the current session ID.
	 */
	UFUNCTION(BlueprintPure, Category = "RiftbornAI|Certification")
	FString GetSessionId() const { return CurrentSessionId; }

	// ========================================================================
	// Event Emission (Called by generated code)
	// ========================================================================

	/**
	 * Generic event emission. Use specific Emit* methods when possible.
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	void EmitEvent(
		ECertEventType EventType,
		ECertEventSeverity Severity,
		const FString& EventName,
		const FString& PayloadJson,
		const TArray<FString>& Tags
	);
	
	/** Convenience overload with default empty payload and tags (not exposed to Blueprint) */
	void EmitEvent(ECertEventType EventType, ECertEventSeverity Severity, const FString& EventName)
	{
		EmitEvent(EventType, Severity, EventName, TEXT("{}"), TArray<FString>());
	}

	/**
	 * Emit BeginPlay event for a generated class.
	 * Call this at the START of BeginPlay().
	 * 
	 * @param Actor - The actor whose BeginPlay is executing
	 * @param bIsGenerated - True if this is a Riftborn-generated class
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	static void EmitBeginPlayStarted(AActor* Actor, bool bIsGenerated = true);

	/**
	 * Emit BeginPlay completed event.
	 * Call this at the END of BeginPlay() (or use the RAII guard).
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	static void EmitBeginPlayCompleted(AActor* Actor, bool bIsGenerated = true);

	/**
	 * Emit class instantiation event.
	 * Call this in constructor or PostInitializeComponents.
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	static void EmitClassInstantiated(UObject* Object, bool bIsGenerated = true);

	/**
	 * Emit pawn possession event.
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	static void EmitPawnPossessed(AController* Controller, APawn* Pawn);

	/**
	 * Emit ability lifecycle event.
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	static void EmitAbilityEvent(
		ECertEventType EventType,
		const FString& AbilityClassName,
		const FString& PayloadJson = TEXT("{}")
	);

	/**
	 * Emit runtime error event.
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	static void EmitRuntimeError(
		ECertEventSeverity Severity,
		const FString& ErrorMessage,
		const FString& Context = TEXT("")
	);

	/**
	 * Emit replication verification event.
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	static void EmitReplicationEvent(
		const FString& PropertyName,
		const FString& ExpectedValue,
		const FString& ActualValue,
		int32 SequenceNumber
	);

	// ========================================================================
	// Event Collection (Called by Python bridge)
	// ========================================================================

	/**
	 * Get all events and optionally clear the buffer.
	 * 
	 * @param bClear - If true, clears the event buffer after returning
	 * @return Array of all events since session start
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	TArray<FCertEvent> GetEvents(bool bClear = false);

	/**
	 * Get events as a JSON array string (for bridge transport).
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	FString GetEventsAsJson(bool bClear = false);

	/**
	 * Get events filtered by type.
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	TArray<FCertEvent> GetEventsByType(ECertEventType EventType);

	/**
	 * Get events filtered by tag.
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	TArray<FCertEvent> GetEventsByTag(const FString& Tag);

	/**
	 * Get count of events by severity (for quick pass/fail check).
	 */
	UFUNCTION(BlueprintPure, Category = "RiftbornAI|Certification")
	int32 GetEventCountBySeverity(ECertEventSeverity Severity) const;

	/**
	 * Check if any error-level events occurred.
	 */
	UFUNCTION(BlueprintPure, Category = "RiftbornAI|Certification")
	bool HasErrors() const;

	/**
	 * Clear all events (use with caution).
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	void ClearEvents();

	// ========================================================================
	// Static Access
	// ========================================================================

	/**
	 * Get the subsystem from a world context.
	 * Returns nullptr if not in PIE or subsystem not available.
	 */
	static URiftbornCertSubsystem* Get(const UObject* WorldContext);

	/**
	 * Get the subsystem from a game instance.
	 */
	static URiftbornCertSubsystem* GetFromGameInstance(UGameInstance* GameInstance);

	/**
	 * Get the subsystem instance for the current PIE session (Blueprint/Python accessible).
	 * @param WorldContextObject - Any object in the world to get context from
	 * @return The certification subsystem instance, or nullptr if not available
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification", meta = (WorldContext = "WorldContextObject"))
	static URiftbornCertSubsystem* GetCertSubsystem(const UObject* WorldContextObject);

	// ========================================================================
	// Static Shared Buffer (accessible from CDO for Python/Blueprint)
	// ========================================================================
	
	/**
	 * Get events from the shared static buffer.
	 * This is accessible from CDO since it's static, making it Python-friendly.
	 */
	UFUNCTION(BlueprintCallable, Category = "RiftbornAI|Certification")
	static FString GetSharedEventsAsJson(bool bClear = false);
	
	/**
	 * Get the shared session ID.
	 */
	UFUNCTION(BlueprintPure, Category = "RiftbornAI|Certification")
	static FString GetSharedSessionId();
	
	/**
	 * Check if shared session is active.
	 */
	UFUNCTION(BlueprintPure, Category = "RiftbornAI|Certification")
	static bool IsSharedSessionActive();
	
	/**
	 * Get shared event count by severity.
	 */
	UFUNCTION(BlueprintPure, Category = "RiftbornAI|Certification")
	static int32 GetSharedEventCount();

private:
	/** Current session ID (GUID-based) */
	FString CurrentSessionId;

	/** Job ID linking to generation task */
	FString CurrentJobId;

	/** Is a session currently active? */
	bool bSessionActive = false;

	/** Session start time for relative timestamps */
	double SessionStartTime = 0.0;

	/** Event buffer (cleared on flush or session end) */
	TArray<FCertEvent> EventBuffer;

	/** Next event ID */
	int32 NextEventId = 0;

	/** Thread-safe lock for event buffer */
	FCriticalSection EventBufferLock;

	/** Internal event emission (thread-safe) */
	void EmitEventInternal(FCertEvent Event);

	/** Generate a unique session ID */
	static FString GenerateSessionId();
	
	// ========================================================================
	// Static shared state (for CDO access from Python)
	// ========================================================================
	static FCriticalSection SharedBufferLock;
	static TArray<FCertEvent> SharedEventBuffer;
	static FString SharedSessionId;
	static bool bSharedSessionActive;
	static int32 SharedNextEventId;
};

// ============================================================================
// RAII Guard for BeginPlay certification
// ============================================================================

/**
 * RAII guard that automatically emits BeginPlayStarted on construction
 * and BeginPlayCompleted on destruction. Use in generated BeginPlay():
 * 
 *   void AMyActor::BeginPlay()
 *   {
 *       FRiftbornBeginPlayGuard Guard(this);
 *       Super::BeginPlay();
 *       // ... rest of BeginPlay
 *   }
 */
struct RIFTBORNAI_API FRiftbornBeginPlayGuard
{
	FRiftbornBeginPlayGuard(AActor* InActor, bool bInIsGenerated = true);
	~FRiftbornBeginPlayGuard();

private:
	AActor* Actor;
	bool bIsGenerated;
	bool bCompleted;
};

// ============================================================================
// Macros for generated code injection
// ============================================================================

/**
 * Emit BeginPlay events with minimal code injection.
 * Use in generated BeginPlay() implementations:
 * 
 *   void AMyActor::BeginPlay()
 *   {
 *       RIFTBORN_CERT_BEGINPLAY_GUARD(this);
 *       Super::BeginPlay();
 *       // ... rest
 *   }
 */
#define RIFTBORN_CERT_BEGINPLAY_GUARD(Actor) \
	FRiftbornBeginPlayGuard _RiftbornBeginPlayGuard(Actor, true)

/**
 * Emit class instantiation event. Use in constructors:
 * 
 *   AMyActor::AMyActor()
 *   {
 *       RIFTBORN_CERT_INSTANTIATED(this);
 *       // ...
 *   }
 */
#define RIFTBORN_CERT_INSTANTIATED(Object) \
	URiftbornCertSubsystem::EmitClassInstantiated(Object, true)
