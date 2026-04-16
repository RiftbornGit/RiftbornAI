// RiftbornDeterminism.h
// Process-global seed management for reproducible agent runs.
//
// Determinism is the moat. "Same prompt + same project state → same result"
// is what makes an AI plugin trustworthy enough to ship games with. This file
// is the central source of randomness for anything the agent touches that
// should be deterministic across replays.
//
// Design:
//   * One session seed (uint64), settable by the UI, console, or replay system.
//   * Per-context sub-seeds derived via a stable hash of (seed, context_string)
//     so independent subsystems never accidentally consume the same RNG stream.
//   * Thread-safe: the seed is atomic; sub-seed derivation is pure.
//
// Callers that produce randomness MUST consume this API rather than
// FMath::Rand / FPlatformTime::Cycles. Drift between callers is a bug.

#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "HAL/CriticalSection.h"
#include "HAL/PlatformAtomics.h"

class RIFTBORNAI_API FRiftbornDeterminism
{
public:
	static FRiftbornDeterminism& Get();

	/** Current session seed. Default 0 until SetSessionSeed is called. */
	uint64 GetSessionSeed() const;

	/** Replace the session seed. Invalidates sub-seeds derived from the old seed. */
	void SetSessionSeed(uint64 NewSeed);

	/** Randomise the session seed using a cryptographically arbitrary source.
	 *  Call this once at "fresh session" time so runs aren't accidentally
	 *  correlated across editor restarts. Returns the new seed. */
	uint64 ResetToRandomSeed();

	/** Derive a stable sub-seed from (SessionSeed, Context). The same
	 *  (seed, context) pair always produces the same sub-seed; different
	 *  contexts produce uncorrelated streams. */
	uint64 DeriveSubSeed(const FString& Context) const;

	/** Convenience: build an FRandomStream seeded for a named context. */
	FRandomStream StreamFor(const FString& Context) const;

	/** True once SetSessionSeed or ResetToRandomSeed has been called.
	 *  Consumers can warn if they pull randomness before this. */
	bool IsInitialized() const;

private:
	FRiftbornDeterminism() = default;

	// Atomic because readers may be on any thread; writers go through the lock
	// to preserve the invariant that SessionSeed + bInitialized flip together.
	mutable FCriticalSection Lock;
	uint64 SessionSeed = 0;
	bool   bInitialized = false;
};
