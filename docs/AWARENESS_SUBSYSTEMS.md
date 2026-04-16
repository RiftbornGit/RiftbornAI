# RiftbornAI — Awareness Subsystems (Deep Dive)

**Last updated**: 2026-02-21

RiftbornAI gives the Copilot LLM **real-time perception** of the UE Editor state through 10 awareness subsystems. Each subsystem is a C++ singleton that introspects live engine state and exposes it as structured data the LLM can query via MCP tools.

This is what makes RiftbornAI different from a chatbot — the LLM can *see* the world.

---

## 1. Architecture Overview

```
                       ┌──────────────────────────────────────┐
                       │           Awareness Layer             │
                       │  (10 singletons, live engine access)  │
                       └────────────────┬─────────────────────┘
                                        │ query
       ┌─────────────┬─────────────┬────┴────┬───────────┬─────────────┐
       ▼             ▼             ▼         ▼           ▼             ▼
  Spatial     Temporal      Audio    Animation   Performance    Physics
  Awareness   Awareness    Awareness  Awareness   Awareness    Awareness
       │             │             │         │           │             │
       ▼             ▼             ▼         ▼           ▼             ▼
  PlayerState  Gameplay     Project    Asset Visual
  Awareness    Systems      Structure  Intelligence
               Awareness    Awareness
```

Each subsystem follows the same pattern:
- **Singleton access**: `FXyzAwareness::Get()`
- **Structured returns**: All data returned as `USTRUCT(BlueprintType)` structs
- **MCP tools**: Each exposes 3-8 tools (e.g., `get_spatial_relationships`, `get_performance_state`)
- **JSON serializable**: All results convert to JSON for LLM consumption

---

## 2. Spatial Awareness

**Header**: `SpatialAwareness.h` (372 lines)
**Implementation**: `SpatialAwareness.cpp` + `SpatialAwarenessTools.cpp`

Gives the LLM understanding of 3D positions, distances, bounds, and spatial relationships.

### Key Types

| Type | Description |
|------|-------------|
| `FActorSpatialInfo` | Actor position, rotation, bounds, bounding sphere, forward vector, movability, collision |
| `FSpatialRelationship` | Distance, direction, line-of-sight between two actors |
| `ESpatialRelation` | `Above`, `Below`, `InFrontOf`, `Behind`, `LeftOf`, `RightOf`, `Inside`, `Outside`, `Touching`, `Near`, `Far`, `Overlapping` |
| `EVisibilityState` | `FullyVisible`, `PartiallyVisible`, `Occluded`, `OutOfRange`, `BehindCamera` |
| `ENavigationState` | `Reachable`, `Unreachable`, `RequiresJump`, `RequiresFall`, `RequiresSwim`, `RequiresFlight`, `Blocked` |

### Key Queries

```
GetActorSpatialInfo(Actor)        → Position, bounds, orientation
GetSpatialRelationship(A, B)      → Distance, direction, LOS
GetNearbyActors(Origin, Radius)   → All actors within radius
GetVisibilityState(Actor, Camera) → How visible the actor is
GetNavigationState(From, To)      → Can you walk there?
```

### Why It Matters

When the LLM says "place a light above the table" or "is the spawn point reachable from the arena?", spatial awareness turns vague language into precise engine queries.

---

## 3. Temporal Awareness

**Header**: `TemporalAwareness.h` (486 lines)
**Implementation**: `TemporalAwareness.cpp` + `TemporalAwarenessTools.cpp` + `TemporalAwareness_Level.cpp`

Understands time of day, weather, seasons, and temporal lighting.

### Key Types

| Type | Description |
|------|-------------|
| `ETimeOfDay` | `Dawn`, `Morning`, `Noon`, `Afternoon`, `Dusk`, `Evening`, `Night`, `Midnight` |
| `EWeatherCondition` | 23 weather types from `Clear` to `MeteorShower` |
| `EWindIntensity` | `Calm` → `Hurricane` (6 levels) |
| `ESeason` | `Spring`, `Summer`, `Fall`, `Winter` + `Monsoon`, `DrySeason`, `Eternal` |
| `ETemporalLightingMood` | `Bright`, `GoldenHour`, `BlueHour`, `Moonlit`, `Dark`, `Overcast`, `Dramatic` |

### Use Case

"Set the scene to a stormy night" → Temporal awareness provides the structured weather/time data that lighting and sky tools need.

---

## 4. Audio Awareness

**Header**: `AudioAwareness.h` (290 lines)
**Implementation**: `AudioAwareness.cpp` + `AudioAwarenessTools.cpp`

Tracks playing sounds, music states, ambient zones, and audio properties.

### Key Types

| Type | Description |
|------|-------------|
| `FPlayingSoundInfo` | Name, asset path, category, location, volume, pitch, looping, 3D, attenuation radius, time remaining, owner |
| `FAudioZoneInfo` | Zone name, type, center, extent, reverb amount |
| `EAudioCategory` | `Music`, `Ambient`, `SFX`, `Voice`, `UI`, `Foley`, `Weather`, `Combat` |
| `EMusicState` | `None`, `Exploration`, `Combat`, `Tension`, `Victory`, `Defeat`, `Menu`, `Cinematic` |
| `EAmbientZone` | `Indoor`, `Outdoor`, `Cave`, `Underwater`, `Forest`, `City`, `Desert`, `Snow`, `Industrial` |

---

## 5. Animation Awareness

**Header**: `AnimationAwareness.h` (131 lines)
**Implementation**: `AnimationAwareness.cpp` + `AnimationAwarenessTools.cpp`

Inspects animation state machines, playing montages, blend spaces, and ragdoll state.

### Key Types

| Type | Description |
|------|-------------|
| `FSkeletalMeshAnimState` | Full anim state: playing animations, slots, state machines, ragdoll, root motion |
| `FPlayingAnimationInfo` | Name, type, play state, time, duration, rate, weight, looping, section |
| `FStateMachineInfo` | Machine name, current state, time in state, available transitions |
| `FAnimationSlotInfo` | Slot name, active montage, weight |
| `FAnimNotifyInfo` | Notify name, trigger time, duration |

### Key Queries

```
GetAnimationState(Actor)              → Full skeletal mesh state
GetPlayingAnimations(Actor)           → Currently playing anims
GetActiveMontage(Actor)               → Active montage info
GetCurrentAnimState(Actor, Machine)   → Current state machine state
IsRagdoll(Actor)                      → Ragdoll check
```

---

## 6. Performance Awareness

**Header**: `PerformanceAwareness.h` (123 lines)
**Implementation**: `PerformanceAwareness.cpp` + `PerformanceAwarenessTools.cpp`

Live performance metrics — FPS, frame time, memory, draw calls, bottleneck detection.

### Key Types

| Type | Description |
|------|-------------|
| `FPerformanceState` | Aggregate: level, bottleneck, frame/memory/scene metrics, warnings, recommendations |
| `FRiftbornFrameMetrics` | FPS, frame time ms, game thread ms, render thread ms, GPU time ms, draw calls, triangles |
| `FMemoryMetrics` | Used/available memory MB, texture/mesh/audio memory, loaded/streaming textures |
| `FSceneMetrics` | Actor count, visible actors, mesh components, lights, shadow-casting lights, particle systems, decals |
| `EPerformanceLevel` | `Excellent` (60+ FPS) → `Critical` (<20 FPS) |
| `EBottleneckType` | `CPU_GameThread`, `CPU_RenderThread`, `GPU`, `Memory`, `Streaming`, `Physics`, `Animation`, `AI`, `Network` |

### Smart Recommendations

The system generates optimization recommendations automatically:
- "Reduce shadow-casting lights (15 → 6) to improve GPU time"
- "Consider LODs for high-poly meshes in camera view"
- "Physics substep count (4) causing CPU bottleneck"

---

## 7. Physics Awareness

**Header**: `PhysicsAwareness.h` (120 lines)
**Implementation**: `PhysicsAwareness.cpp` + `PhysicsAwarenessTools.cpp`

Physics world state — simulating bodies, constraints, velocities, raycasts.

### Key Types

| Type | Description |
|------|-------------|
| `FPhysicsWorldState` | Gravity, body counts (simulating/sleeping/static), constraints, broken constraints, physics time ms |
| `FPhysicsBodyInfo` | Actor, component, state, collision type, velocity, angular velocity, mass, damping, gravity |
| `FConstraintInfo` | Constraint name, parent/child actors, type, broken state, limits |
| `EPhysicsBodyState` | `Static`, `Kinematic`, `Simulating`, `Sleeping`, `Frozen` |

### Key Queries

```
GetWorldState()                        → Full physics world state
GetSimulatingBodies()                  → All active physics bodies
GetBodiesInRadius(Center, Radius)      → Spatial physics query
GetVelocity(Actor)                     → Actor's linear velocity
RaycastSingle(Start, End)             → Physics raycast
SweepSingle(Start, End, Radius)       → Sphere sweep
```

---

## 8. Player State Awareness

**Header**: `PlayerStateAwareness.h` (410 lines)
**Implementation**: `PlayerStateAwareness.cpp` + `PlayerStateAwarenessTools.cpp`

Everything about the player: input bindings, camera, movement state, combat state, attributes.

### Key Types

| Type | Description |
|------|-------------|
| `FCameraStateInfo` | Position, rotation, FOV, forward vector, camera mode, arm length, look-at target |
| `FInputBindingInfo` | Action name, description, primary/secondary keys, gamepad button, axis flag |
| `FInputContextInfo` | Context name, priority, active state, bindings |
| `FPlayerAttributeInfo` | Attribute name, current/max/base values |
| `EPlayerMovementState` | `Idle`, `Walking`, `Running`, `Sprinting`, `Crouching`, `Jumping`, `Falling`, `Swimming`, `Flying`, `Climbing` |
| `EPlayerCombatState` | `Neutral`, `InCombat`, `Attacking`, `Blocking`, `Dodging`, `Aiming`, `Reloading`, `Dead` |
| `EInputDeviceType` | `KeyboardMouse`, `Gamepad`, `Touch`, `Motion` |

---

## 9. Gameplay Systems Awareness

**Header**: `GameplaySystemsAwareness.h` (183 lines)
**Implementation**: `GameplaySystemsAwareness.cpp` + `GameplaySystemsAwarenessTools.cpp`

GAS abilities, gameplay effects, attribute sets, AI controllers, quests, game state.

### Key Types

| Type | Description |
|------|-------------|
| `FGameplayState` | Game state (playing/paused/etc), current level, game time, score, active quests, AI controllers |
| `FAbilityInfo` | Ability name, class, state (ready/cooldown/active/blocked), cooldown, level, cost, tags |
| `FGameplayEffectInfo` | Effect name, duration, time remaining, stack count, source, modified attributes |
| `FAttributeSetInfo` | Set name, base values, current values |
| `FAIControllerInfo` | Controller class, AI state, behavior tree, current task, blackboard, perception radii, perceived actors |
| `FQuestInfo` | Quest name/ID, description, current objective, progress, completion state |

### Key Queries

```
GetAbilities(Actor)                → All GAS abilities
CanActivateAbility(Actor, Name)    → Can this ability fire?
GetActiveEffects(Actor)            → All gameplay effects
GetAIControllerInfo(Actor)         → Full AI state
GetGameState()                     → Current game state
```

---

## 10. Project Structure Awareness

**Header**: `ProjectStructureAwareness.h` (306 lines)
**Implementation**: `ProjectStructureAwareness.cpp` + `ProjectStructureAwarenessTools.cpp` + `ProjectStructureAwarenessTools_Browse.cpp`

Understanding of the UE project itself — modules, classes, assets, configs, build targets.

### Key Types

| Type | Description |
|------|-------------|
| `FModuleInfo` | Module name, path, type, public/private dependencies, header/source/class counts |
| `FClassInfo` | Class name, parent, code type, category, file path, module, interfaces, abstract/blueprintable |
| `FDataTableInfo` | Table name, asset path, row struct, row count, column/row names |
| `FConfigInfo` | Config name, path, type, sections, setting count |
| `FProjectModuleInfo` | Project name, engine version, game/plugin/engine modules, dependency graph |
| `FBuildTargetInfo` | Target name, platforms, configurations |
| `FPluginInfo` | (details about installed plugins) |

---

## 11. Asset Visual Intelligence

**Header**: `AssetVisualIntelligence.h` (613 lines)
**Implementation**: `AssetVisualIntelligence.cpp` + `AssetVisualIntelligence_LLM.cpp` + `AssetVisualIntelligenceTools.cpp`

Uses Vision LLMs (LLaVA, GPT-4V, Claude Vision) to semantically classify assets — "what IS this mesh?"

### Semantic Categories (84 types)

| Domain | Categories |
|--------|-----------|
| **Nature** | Tree, Bush, Grass, Flower, Rock, Boulder, Cliff, Terrain, Water |
| **Architecture** | Building, Wall, Floor, Ceiling, Door, Window, Stairs, Pillar, Arch |
| **Furniture** | Chair, Table, Bed, Shelf, Cabinet, Desk, Couch, Lamp |
| **Props** | Barrel, Crate, Chest, Pot, Vase, Book, Candle, Sign |
| **Vehicles** | Car, Truck, Boat, Aircraft, Motorcycle, Cart |
| **Characters** | HumanCharacter, AnimalCreature, MonsterCreature, RobotMech |
| **Weapons** | Sword, Axe, Hammer, Bow, Gun, Shield, Staff |
| **Consumables** | Food, Drink, Potion |

### Visual Styles
`Realistic`, `Stylized`, `LowPoly`, `Pixel`, `Cartoon`, `Anime`, `Medieval`, `SciFi`, `Modern`, `Fantasy`, `Horror`, `Steampunk`, `PostApocalyptic`

### Detail Levels
`AAA` (legacy high-polish tier), `Indie`, `Prototype`, `Primitive`

### Why It Matters

When you say "fill this room with medieval furniture" the system can look at imported asset packs and determine which meshes are actually chairs, tables, etc. — without relying on filenames.

---

## 12. Cross-Cutting Patterns

### Query Flow

```
User: "Is the arena too dark?"
    │
    ▼ LLM selects tools
    │
    ├── get_performance_state()     → FPerformanceState
    ├── get_spatial_info("Arena")   → FActorSpatialInfo  
    ├── get_lighting_analysis()     → From SpatialLightingSystem
    └── get_temporal_state()        → ETimeOfDay, ELightingMood
    │
    ▼ LLM synthesizes answer
    │
"The arena has 2 point lights at 1000 intensity each (dim). 
 It's set to Night/Dark mood. Recommendation: add 4 more point 
 lights at 5000 intensity, or switch to Afternoon/Bright."
```

### Memory Efficiency

Each awareness subsystem:
- Is lazily initialized (created on first `::Get()` call)
- Caches last query result with TTL (~100ms for frame metrics, ~1s for static data)
- Returns by value (no dangling pointers across frames)
- Uses `USTRUCT(BlueprintType)` for GC safety where needed

### Thread Safety

All awareness queries are **game-thread only** — they read UE state that's only safe to access from the main thread. The bridge HTTP server dispatches awareness queries to the game thread via `FTicker`.
