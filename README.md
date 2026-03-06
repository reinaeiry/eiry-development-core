# eiry development core

A foundational scripting framework for Arma Reforger mod development. Provides common infrastructure systems so every mod you write doesn't reinvent the same patterns from scratch.

---

## Table of Contents

- [Installation](#installation)
- [Systems](#systems)
  - [Event Bus](#event-bus)
  - [Feature Registry](#feature-registry)
  - [Net Bus](#net-bus)
  - [Logger](#logger)
  - [Timer Manager](#timer-manager)
  - [State Machine](#state-machine)
  - [Sequence](#sequence)
  - [Cache](#cache)
  - [Rate Limiter](#rate-limiter)
  - [JSON Builder](#json-builder)
  - [Utilities](#utilities)
    - [Player Utils](#player-utils)
    - [String Utils](#string-utils)
    - [Math Utils](#math-utils)
    - [Component Utils](#component-utils)
    - [Collections](#collections)
    - [World Utils](#world-utils)
    - [Replication Utils](#replication-utils)
- [Event Name Constants](#event-name-constants)
- [Script Module Structure](#script-module-structure)

---

## Installation

1. Subscribe to **eiry development core** on the Reforger Workshop.
2. Add it as a dependency in your addon's `.gproj` file.
3. All classes are globally available — no imports required.

---

## Systems

---

### Event Bus

**Files:** `Scripts/Game/EventBusInterop/EIR_EventBus.c`, `EIR_EventPayload.c`, `EIR_EventNames.c`

A global publish/subscribe event bus. Any script can fire named events and any other script can subscribe to them — **with no compile-time dependency between the two sides**. This is the primary mechanism for optional mod interoperability.

#### Firing an event

```c
// With payload data
EIR_QuestCompletedPayload p = new EIR_QuestCompletedPayload();
p.questId = "sweep_stary";
p.playerGuid = "5";
EIR_EventBus.GetInstance().Fire(EIR_EventNames.QUEST_COMPLETED, p);

// Without payload
EIR_EventBus.GetInstance().Fire(EIR_EventNames.SERVER_READY);
```

#### Subscribing

```c
// In your component's EOnInit or equivalent:
EIR_EventBus.GetInstance().GetInvoker(EIR_EventNames.QUEST_COMPLETED).Insert(this.OnQuestCompleted);

// Handler signature must match: void MyHandler(string eventName, EIR_EventPayload payload)
void OnQuestCompleted(string eventName, EIR_EventPayload payload)
{
    EIR_QuestCompletedPayload qp = EIR_QuestCompletedPayload.Cast(payload);
    if (!qp)
        return;

    Print("Quest completed: " + qp.questId);
}
```

#### Unsubscribing

```c
EIR_EventBus.GetInstance().GetInvoker(EIR_EventNames.QUEST_COMPLETED).Remove(this.OnQuestCompleted);
```

#### Defining a custom payload

```c
class EIR_QuestCompletedPayload : EIR_EventPayload
{
    string questId;
    string playerGuid;
    int rewardAmount;
}
```

#### Key behaviours
- Firing an event with no subscribers is safe — nothing happens.
- Multiple handlers may be registered for the same event name.
- `GetInvoker()` creates the invoker on first call — there is no cost to subscribing before anything fires.
- Subscribers are called synchronously in insertion order.

---

### Feature Registry

**File:** `Scripts/Game/FeatureRegistry/EIR_FeatureRegistry.c`

A runtime registry where mods announce their presence. Other mods can query it without importing any of the registering mod's classes.

#### Registering

```c
// In your mod's bootstrap component EOnInit:
EIR_FeatureRegistry.GetInstance().Register("eir.questing", "1.2.0");
```

#### Querying

```c
if (EIR_FeatureRegistry.GetInstance().IsRegistered("eir.questing"))
{
    // Show quest UI, hook quest events, etc.
}

string version = EIR_FeatureRegistry.GetInstance().GetVersion("eir.questing");
```

#### Listing all registered features

```c
array<string> features = {};
EIR_FeatureRegistry.GetInstance().GetAllRegistered(features);
foreach (string name : features)
{
    Print("Loaded: " + name);
}
```

#### Key behaviours
- Registering the same feature name twice is silently ignored.
- Returns empty string for version if the feature is not registered.
- Convention for feature names: `"author.feature"` — e.g. `"eir.questing"`, `"eir.economy"`.

---

### Net Bus

**Files:** `Scripts/Game/NetBus/EIR_NetBus.c`, `EIR_NetBusComponent.c`

An RPC abstraction layer. Instead of writing a new `[RplRpc]` method for every networked message, all messages route through three gateway RPCs. Register string-keyed handlers and send messages in any direction with a single line.

#### Setup

Place one `EIR_NetBusComponent` on your **GameMode entity** in Workbench. It hosts the gateway RPCs and must exist in the world for any NetBus messages to work.

#### Sending: server → specific player

```c
EIR_JsonBuilder b = EIR_JsonBuilder.NewObject();
b.AddString("questId", "sweep_stary");
b.AddInt("progress", 7);
string payload = b.Build();

EIR_NetBusComponent.GetInstance().SendToPlayer(playerId, "quest.update", payload);
```

#### Sending: server → all clients

```c
EIR_NetBusComponent.GetInstance().SendToAll("server.event", payload);
```

#### Sending: client → server

```c
EIR_NetBusComponent.GetInstance().SendToServer("player.action", payload);
```

#### Receiving on the client

```c
// In client-side component init:
EIR_NetBus.GetInstance().GetClientInvoker("quest.update").Insert(this.OnQuestUpdate);

// Handler signature: void MyHandler(string typeId, string payload, int fromId)
void OnQuestUpdate(string typeId, string payload, int fromId)
{
    // parse payload, update UI, etc.
}
```

#### Receiving on the server

```c
EIR_NetBus.GetInstance().GetServerInvoker("player.action").Insert(this.OnPlayerAction);

void OnPlayerAction(string typeId, string payload, int fromId)
{
    // fromId is the playerId of the sending client
}
```

#### Key behaviours
- Listen-server hosts are handled correctly — `SendToServer` routes locally without a network hop.
- `fromId` is `-1` when a message originates from the server.
- The payload is a freeform string — use `EIR_JsonBuilder` to structure it.

---

### Logger

**File:** `Scripts/Game/Logging/EIR_Logger.c`

Structured logging with category prefixes and level filtering. All output is tagged `[EIR][category]` for easy filtering in logs.

#### Basic usage

```c
EIR_Logger.Info("questing", "Player 5 completed sweep_stary");
EIR_Logger.Warning("netbus", "No EIR_NetBusComponent found in scene");
EIR_Logger.Error("economy", "Balance underflow for player 3");
```

Output format:
```
[EIR][questing] Player 5 completed sweep_stary
[EIR][netbus] No EIR_NetBusComponent found in scene
```

#### Debug and verbose output

Debug and Verbose lines are suppressed by default to keep production logs clean.

```c
// Enable globally (e.g. in development builds)
EIR_Logger.SetDebugEnabled(true);

EIR_Logger.Debug("questing", "Processing kill event for quest sweep_stary");
EIR_Logger.Verbose("netbus", "Dispatching typeId quest.update to 3 handlers");
```

#### Silencing a category

```c
EIR_Logger.DisableCategory("questing");
// All log levels for "questing" are now suppressed

EIR_Logger.EnableCategory("questing");
```

#### Log levels

| Method | Enfusion LogLevel | Suppressed by default |
|---|---|---|
| `Debug` | `LogLevel.DEBUG` | Yes |
| `Verbose` | `LogLevel.VERBOSE` | Yes |
| `Info` | `LogLevel.NORMAL` | No |
| `Warning` | `LogLevel.WARNING` | No |
| `Error` | `LogLevel.ERROR` | No |

---

### Timer Manager

**File:** `Scripts/Game/Timers/EIR_TimerManager.c`

Named wrapper around `GetGame().GetCallqueue()`. Eliminates the need to hold function references for cancellation, prevents duplicate timers, and lets you check whether a timer is active by name.

#### One-shot timer

```c
EIR_TimerInvoker inv = new EIR_TimerInvoker();
inv.Insert(this.OnRespawnReady);
EIR_TimerManager.GetInstance().Schedule("respawn.player5", inv, 10.0);
```

#### Repeating timer

```c
EIR_TimerInvoker inv = new EIR_TimerInvoker();
inv.Insert(this.OnAutoSaveTick);
EIR_TimerManager.GetInstance().Schedule("autosave", inv, 60.0, true);
```

#### Cancelling

```c
EIR_TimerManager.GetInstance().Cancel("respawn.player5");
```

#### Checking if scheduled

```c
if (!EIR_TimerManager.GetInstance().IsScheduled("respawn.player5"))
{
    // safe to schedule again
}
```

#### Key behaviours
- Scheduling a name that already exists cancels the previous timer automatically.
- One-shot timers remove themselves from the registry after firing.
- Timer callbacks have no parameters: `void MyCallback() { ... }`

---

### State Machine

**File:** `Scripts/Game/StateMachine/EIR_StateMachine.c`

A simple named-state machine with enter/exit callbacks. Create one instance per system that needs managed state transitions.

#### Setup

```c
EIR_StateMachine m_FSM;

void InitFSM()
{
    m_FSM = new EIR_StateMachine();

    EIR_StateEntry idle = m_FSM.RegisterState("idle");
    idle.GetOnEnter().Insert(this.OnEnterIdle);
    idle.GetOnExit().Insert(this.OnExitIdle);

    EIR_StateEntry combat = m_FSM.RegisterState("combat");
    combat.GetOnEnter().Insert(this.OnEnterCombat);

    EIR_StateEntry dead = m_FSM.RegisterState("dead");
    dead.GetOnEnter().Insert(this.OnEnterDead);

    m_FSM.Transition("idle");
}
```

#### Transitioning

```c
m_FSM.Transition("combat");
// Fires: OnExitIdle("idle", "combat"), OnEnterCombat("idle", "combat")

m_FSM.Transition("dead");
// Fires: OnExitCombat (if registered), OnEnterDead("combat", "dead")
```

#### Callback signature

```c
// void MyCallback(string prevState, string nextState)
void OnEnterCombat(string prevState, string nextState)
{
    Print("Entered combat from: " + prevState);
}
```

#### Querying

```c
string current = m_FSM.GetCurrentState();  // "combat"

if (m_FSM.IsInState("dead"))
{
    // handle dead state
}
```

#### Key behaviours
- Transitioning to the current state is a no-op.
- Transitioning to an unregistered state logs a warning and does nothing.
- `prevState` is an empty string on the initial `Transition()` call.

---

### Sequence

**File:** `Scripts/Game/Sequence/EIR_Sequence.c`

Chains a series of timed actions. Each step fires after its configured delay has elapsed since the previous step completed. Eliminates deeply nested `CallLater` callbacks.

#### Usage

```c
EIR_Sequence seq = new EIR_Sequence();

EIR_SequenceStep step1 = seq.AddStep(0);       // fires immediately on Start()
step1.GetInvoker().Insert(this.ShowIntroText);

EIR_SequenceStep step2 = seq.AddStep(3.0);     // fires 3s after step 1
step2.GetInvoker().Insert(this.SpawnObjective);

EIR_SequenceStep step3 = seq.AddStep(1.5);     // fires 1.5s after step 2
step3.GetInvoker().Insert(this.NotifyPlayers);

seq.Start();
```

#### Cancelling

```c
seq.Stop();
```

#### Restarting

```c
seq.Start(); // restarts from step 0 even if already running
```

#### Key behaviours
- Steps with `delaySeconds = 0` fire synchronously within the same frame.
- `Stop()` cancels any pending step — already-fired steps are not undone.
- Step callbacks have no parameters: `void MyCallback() { ... }`

---

### Cache

**File:** `Scripts/Game/Cache/EIR_Cache.c`

TTL-based key/value cache for `Managed` objects. Create one instance per system — this is not a singleton.

#### Usage

```c
// Create a cache instance (e.g. as a class field)
protected ref EIR_Cache m_PlayerDataCache = new EIR_Cache();

// Store for 30 seconds
m_PlayerDataCache.Set("player.5", myDataObject, 30.0);

// Retrieve — returns null if expired or not found
MyDataClass data = MyDataClass.Cast(m_PlayerDataCache.Get("player.5"));
if (!data)
{
    // cache miss — fetch from source
}

// Check without retrieving
if (m_PlayerDataCache.Has("player.5"))
{
    // cache hit
}

// Invalidate manually
m_PlayerDataCache.Remove("player.5");

// Clear everything
m_PlayerDataCache.Clear();
```

#### Purging expired entries

Entries are evicted lazily when accessed. If you have many entries that expire without being read, call `Purge()` periodically to free memory.

```c
m_PlayerDataCache.Purge();
```

---

### Rate Limiter

**File:** `Scripts/Game/RateLimiter/EIR_RateLimiter.c`

Per-key rate limiting. Prevents the same action from firing more often than a configured cooldown allows. Create one instance per notification type or action category.

#### Basic usage

```c
// One limiter per notification type, shared across all players
protected ref EIR_RateLimiter m_NotifLimiter = new EIR_RateLimiter(5.0); // 5s cooldown

void SendProgressNotification(int playerId, string message)
{
    string key = playerId.ToString();
    if (!m_NotifLimiter.Allow(key))
        return; // still on cooldown for this player

    // proceed to send notification
}
```

#### Per-player per-event cooldowns

```c
string key = string.Format("%1.%2", playerId, eventName);
if (!m_NotifLimiter.Allow(key))
    return;
```

#### Querying and resetting

```c
if (m_NotifLimiter.IsLimited("player.5"))
{
    float remaining = m_NotifLimiter.GetRemainingCooldown("player.5");
    Print("On cooldown for: " + remaining.ToString() + "s");
}

m_NotifLimiter.Reset("player.5");   // reset one key
m_NotifLimiter.ResetAll();          // reset all keys
```

---

### JSON Builder

**File:** `Scripts/Game/Json/EIR_JsonBuilder.c`

Fluent builder for constructing JSON strings. Enfusion has no built-in serialiser — this covers the common cases needed for NetBus payloads and web API integration.

#### Building an object

```c
EIR_JsonBuilder b = EIR_JsonBuilder.NewObject();
b.AddString("questId", "sweep_stary");
b.AddInt("playerId", 5);
b.AddFloat("progress", 0.75);
b.AddBool("completed", false);
b.AddNull("reward");
string json = b.Build();
// -> {"questId":"sweep_stary","playerId":5,"progress":0.750,"completed":false,"reward":null}
```

#### Building an array

```c
EIR_JsonBuilder arr = EIR_JsonBuilder.NewArray();
arr.PushString("alpha");
arr.PushString("bravo");
arr.PushInt(42);
arr.PushBool(true);
string json = arr.Build();
// -> ["alpha","bravo",42,true]
```

#### Nesting

```c
EIR_JsonBuilder pos = EIR_JsonBuilder.NewObject();
pos.AddFloat("x", 1234.5);
pos.AddFloat("y", 80.0);
pos.AddFloat("z", 987.3);

EIR_JsonBuilder outer = EIR_JsonBuilder.NewObject();
outer.AddString("playerId", "5");
outer.AddRaw("position", pos.Build());
string json = outer.Build();
// -> {"playerId":"5","position":{"x":1234.500,"y":80.000,"z":987.300}}
```

#### Float precision

```c
b.AddFloat("value", 3.14159, 2);   // -> "value":3.14
b.AddFloat("value", 3.14159, 4);   // -> "value":3.1415
```

---

## Utilities

---

### Player Utils

**File:** `Scripts/Game/Utils/EIR_PlayerUtils.c`

Static helpers for common player resolution boilerplate.

```c
// Validate an ID
if (!EIR_PlayerUtils.IsValidId(playerId))
    return;

// Get controlled entity
IEntity playerEnt = EIR_PlayerUtils.GetEntityFromId(playerId);

// Get display name
string name = EIR_PlayerUtils.GetName(playerId);

// Log-friendly description: "PlayerName (id:5)"
string desc = EIR_PlayerUtils.Describe(playerId);

// Get all connected player IDs
array<int> players = {};
EIR_PlayerUtils.GetAllConnectedIds(players);

// Reverse lookup: entity -> playerId
int foundId;
if (EIR_PlayerUtils.TryGetIdFromEntity(someEntity, foundId))
{
    Print("Entity is controlled by player: " + foundId.ToString());
}
```

---

### String Utils

**File:** `Scripts/Game/Utils/EIR_StringUtils.c`

String helpers that complement Enfusion's built-in string methods and `SCR_StringHelper`.

```c
EIR_StringUtils.IsEmpty("");                        // true
EIR_StringUtils.StartsWith("eir.questing", "eir"); // true
EIR_StringUtils.EndsWith("player.5", ".5");         // true
EIR_StringUtils.Contains("sweep_stary", "stary");   // true
EIR_StringUtils.Trim("  hello  ");                  // "hello"
EIR_StringUtils.PadRight("HP", 10);                 // "HP        "
EIR_StringUtils.PadLeft("42", 6);                   // "    42"
EIR_StringUtils.Repeat("-", 20);                    // "--------------------"
```

---

### Math Utils

**File:** `Scripts/Game/Utils/EIR_MathUtils.c`

Math helpers that complement the built-in `Math` class.

```c
EIR_MathUtils.Sign(-5.0);                               // -1
EIR_MathUtils.Sign(0.0);                                // 0
EIR_MathUtils.ApproximatelyEqual(0.1 + 0.2, 0.3);      // true
EIR_MathUtils.RemapClamped(75, 0, 100, 0, 1);           // 0.75 (clamped)
EIR_MathUtils.Wrap(370, 0, 360);                        // 10
EIR_MathUtils.Percent(75, 0, 100);                      // 75.0
EIR_MathUtils.InRange(5.0, 0, 10);                      // true
EIR_MathUtils.RoundTo(3.14159, 2);                      // 3.14

vector point = EIR_MathUtils.RandomPointInCircle(myPos, 50.0);
vector sphere = EIR_MathUtils.RandomPointInSphere(myPos, 100.0);
```

---

### Component Utils

**File:** `Scripts/Game/Utils/EIR_ComponentUtils.c`

Entity hierarchy component lookups without writing the traversal loop every time.

```c
// On the entity itself only
Managed comp = EIR_ComponentUtils.OnSelf(entity, HealthComponent);

// On entity or its direct parent
Managed comp = EIR_ComponentUtils.InParents(entity, InventoryComponent);

// Walk up the full hierarchy until found
Managed comp = EIR_ComponentUtils.WalkUp(entity, FactionComponent);

// On the root-most parent
Managed comp = EIR_ComponentUtils.OnRoot(entity, VehicleComponent);

// First matching component in direct children
Managed comp = EIR_ComponentUtils.InChildren(entity, WeaponComponent);

// Collect all matching components from children
array<Managed> weapons = {};
EIR_ComponentUtils.CollectFromChildren(entity, WeaponComponent, weapons);

// Cast the result to your type
HealthComponent hp = HealthComponent.Cast(EIR_ComponentUtils.OnSelf(entity, HealthComponent));
```

---

### Collections

**File:** `Scripts/Game/Utils/EIR_Collections.c`

Array helpers for common operations Enfusion's array class doesn't provide.

```c
// Random element
IEntity target = EIR_Collections.RandomEntity(entityArray);
int     id     = EIR_Collections.RandomInt(playerIds);
string  name   = EIR_Collections.RandomString(names);

// First / Last
IEntity first = EIR_Collections.FirstEntity(entities);
IEntity last  = EIR_Collections.LastEntity(entities);

// Shuffle in-place (Fisher-Yates)
EIR_Collections.ShuffleEntities(spawnPoints);
EIR_Collections.ShuffleInts(playerIds);
EIR_Collections.ShuffleStrings(questPool);

// Contains
EIR_Collections.ContainsString(names, "PlayerOne");  // true/false
EIR_Collections.ContainsInt(playerIds, 5);           // true/false

// Remove duplicates in-place
EIR_Collections.DeduplicateStrings(tags);
EIR_Collections.DeduplicateInts(ids);
```

---

### World Utils

**File:** `Scripts/Game/Utils/EIR_WorldUtils.c`

World and spatial query helpers wrapping `QueryEntitiesBySphere` and `TraceMove`.

```c
// Find all entities within 100m
array<IEntity> nearby = {};
EIR_WorldUtils.FindEntitiesInRadius(origin, 100.0, nearby);

// Find all dynamic entities within 50m
EIR_WorldUtils.FindEntitiesInRadius(origin, 50.0, nearby, EQueryEntitiesFlags.DYNAMIC);

// Find closest entity within 200m
IEntity closest = EIR_WorldUtils.FindClosestEntityInRadius(origin, 200.0);

// Get terrain/surface height below a position
float groundY = EIR_WorldUtils.GetSurfaceY(myPos);
vector grounded = Vector(myPos[0], groundY, myPos[2]);

// Line of sight check
if (EIR_WorldUtils.HasLineOfSight(shooterPos, targetPos, shooterEntity))
{
    // clear line of sight
}

// Ray hit position
vector hitPos = EIR_WorldUtils.TracePosition(gunMuzzle, gunMuzzle + aimDir * 500);
```

---

### Replication Utils

**File:** `Scripts/Game/Utils/EIR_ReplicationUtils.c`

Clean static wrappers for the repetitive `Replication.IsServer()` / `RplSession.Mode()` checks.

```c
EIR_ReplicationUtils.IsServer()          // true on listen server or dedicated server
EIR_ReplicationUtils.IsDedicatedServer() // true only on headless dedicated server
EIR_ReplicationUtils.IsListenServer()    // true when host is also a player
EIR_ReplicationUtils.IsClient()          // true on pure client peers
EIR_ReplicationUtils.IsMultiplayer()     // true in any networked session

// Useful for log messages
Print("Running as: " + EIR_ReplicationUtils.GetPeerRole());
// -> "Running as: DedicatedServer"
// -> "Running as: ListenServer"
// -> "Running as: Client"
```

---

## Event Name Constants

**File:** `Scripts/Game/EventBusInterop/EIR_EventNames.c`

Predefined constants for standard EIR event names. Use these instead of raw strings.

```c
EIR_EventNames.PLAYER_CONNECTED        // "eir.player.connected"
EIR_EventNames.PLAYER_DISCONNECTED     // "eir.player.disconnected"
EIR_EventNames.QUEST_STARTED           // "eir.quest.started"
EIR_EventNames.QUEST_PROGRESS          // "eir.quest.progress"
EIR_EventNames.QUEST_COMPLETED         // "eir.quest.completed"
EIR_EventNames.QUEST_ABANDONED         // "eir.quest.abandoned"
EIR_EventNames.SERVER_READY            // "eir.server.ready"
EIR_EventNames.ECONOMY_BALANCE_CHANGED // "eir.economy.balance.changed"
EIR_EventNames.ECONOMY_PURCHASE        // "eir.economy.purchase"
```

---

## Script Module Structure

```
Scripts/Game/
├── EventBusInterop/
│   ├── EIR_EventBus.c         Global pub/sub event bus
│   ├── EIR_EventPayload.c     Base class for event data
│   └── EIR_EventNames.c       Standard event name constants
├── FeatureRegistry/
│   └── EIR_FeatureRegistry.c  Runtime mod capability registry
├── NetBus/
│   ├── EIR_NetBus.c           RPC routing and handler registry
│   └── EIR_NetBusComponent.c  Gateway RPC host (place on GameMode entity)
├── Logging/
│   └── EIR_Logger.c           Category-based structured logging
├── Timers/
│   └── EIR_TimerManager.c     Named timer scheduling and cancellation
├── StateMachine/
│   └── EIR_StateMachine.c     Named-state FSM with enter/exit callbacks
├── Sequence/
│   └── EIR_Sequence.c         Chained timed action sequences
├── Cache/
│   └── EIR_Cache.c            TTL key/value cache for Managed objects
├── RateLimiter/
│   └── EIR_RateLimiter.c      Per-key cooldown rate limiting
├── Json/
│   └── EIR_JsonBuilder.c      JSON string builder
└── Utils/
    ├── EIR_PlayerUtils.c       Player ID and entity resolution
    ├── EIR_StringUtils.c       String manipulation helpers
    ├── EIR_MathUtils.c         Extended math utilities
    ├── EIR_ComponentUtils.c    Entity hierarchy component lookups
    ├── EIR_Collections.c       Array shuffle, random, dedup helpers
    ├── EIR_WorldUtils.c        Sphere queries and trace helpers
    └── EIR_ReplicationUtils.c  Replication state check wrappers
```

---

## License

MIT License. See `license.txt`.
