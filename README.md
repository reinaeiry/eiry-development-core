# eiry development core

A foundational scripting framework for Arma Reforger mod development. Provides common infrastructure systems so every mod you write does not reinvent the same patterns from scratch.

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

1. Subscribe to **eiry development core** on the Reforger Workshop, or clone this repository into your workbench addons directory.
2. Add it as a dependency in your addon's `.gproj` file.
3. All classes are globally available - no imports required.

---

## Systems

---

### Event Bus

**Files:** `Scripts/Game/EventBusInterop/EIR_EventBus.c`, `EIR_EventPayload.c`, `EIR_EventNames.c`

A global publish/subscribe event bus. Any script can fire named events and any other script can subscribe to them with no compile-time dependency between the two sides. This is the primary mechanism for optional mod interoperability.

#### Quick Example

```c
// Fire
EIR_EventBus.GetInstance().Fire(EIR_EventNames.QUEST_COMPLETED);

// Subscribe
EIR_EventBus.GetInstance().GetInvoker(EIR_EventNames.QUEST_COMPLETED).Insert(this.OnQuestCompleted);

void OnQuestCompleted(string eventName, EIR_EventPayload payload)
{
    // payload may be null - cast to your own subclass if you expect data
}

// Unsubscribe
EIR_EventBus.GetInstance().GetInvoker(EIR_EventNames.QUEST_COMPLETED).Remove(this.OnQuestCompleted);
```

#### Passing Data

Extend `EIR_EventPayload` to attach typed data to events:

```c
class MyQuestPayload : EIR_EventPayload
{
    int playerId;
    string questId;
}

MyQuestPayload p = new MyQuestPayload();
p.playerId = 5;
p.questId  = "sweep_stary";
EIR_EventBus.GetInstance().Fire(EIR_EventNames.QUEST_STARTED, p);

// In the subscriber:
void OnQuestStarted(string eventName, EIR_EventPayload payload)
{
    MyQuestPayload p = MyQuestPayload.Cast(payload);
    if (!p)
        return;
    // use p.playerId, p.questId
}
```

#### API - `EIR_EventBus`

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `GetInstance()` | - | `EIR_EventBus` | Returns the singleton. Creates it on first call. |
| `GetInvoker(eventName)` | `string eventName` | `EIR_EventInvoker` | Returns the invoker for `eventName`, creating it if it does not exist. Call `.Insert(callback)` to subscribe and `.Remove(callback)` to unsubscribe. Safe to call before anything fires. |
| `Fire(eventName, payload)` | `string eventName`, `EIR_EventPayload payload = null` | `void` | Calls all subscribers for `eventName` synchronously in insertion order. `payload` may be `null`. Safe to call with no subscribers registered. |

#### API - `EIR_EventPayload`

Base class for all event data. Extend it to carry typed fields. No public methods - it exists solely as a type-safe container for cast-and-check at the receiver.

#### Callback Signature

All subscriber methods must match:
```c
void MyCallback(string eventName, EIR_EventPayload payload)
```

---

### Feature Registry

**File:** `Scripts/Game/FeatureRegistry/EIR_FeatureRegistry.c`

A runtime capability registry. Mods announce themselves at startup so other mods can detect their presence without a compile-time dependency.

#### Quick Example

```c
// In your mod's init component:
EIR_FeatureRegistry.GetInstance().Register("eir.questing", "2.1.0");

// In another mod that wants to integrate:
if (EIR_FeatureRegistry.GetInstance().IsRegistered("eir.questing"))
{
    string ver = EIR_FeatureRegistry.GetInstance().GetVersion("eir.questing");
}
```

#### API - `EIR_FeatureRegistry`

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `GetInstance()` | - | `EIR_FeatureRegistry` | Returns the singleton. |
| `Register(featureName, version)` | `string featureName`, `string version = ""` | `void` | Registers `featureName` with an optional `version` string. Calling this multiple times for the same name is safe - subsequent calls are silently ignored. Convention: `"author.feature"` e.g. `"eir.questing"`. |
| `IsRegistered(featureName)` | `string featureName` | `bool` | Returns `true` if the feature has been registered. |
| `GetVersion(featureName)` | `string featureName` | `string` | Returns the version string passed to `Register`. Returns `""` if not registered or if no version was given. |
| `GetAllRegistered(outFeatures)` | `out array<string> outFeatures` | `void` | Fills `outFeatures` with the names of all currently registered features. The array is cleared before filling. |

---

### Net Bus

**Files:** `Scripts/Game/NetBus/EIR_NetBus.c`, `EIR_NetBusComponent.c`

A centralised RPC abstraction layer. All network messages from any mod flow through three gateway RPCs hosted on a single `ScriptComponent`. Subscribers register by message type ID and receive callbacks with the payload and sender identity.

#### Setup

Add `EIR_NetBusComponent` to your GameMode entity in the Workbench prefab editor. Only one instance is needed for the entire server.

#### Quick Example

```c
// Server sends to a specific player:
EIR_NetBusComponent.GetInstance().SendToPlayer(playerId, "quest.update", jsonPayload);

// Server sends to all clients:
EIR_NetBusComponent.GetInstance().SendToAll("server.announcement", message);

// Client sends to server:
EIR_NetBusComponent.GetInstance().SendToServer("player.action", actionData);

// Client receives from server:
EIR_NetBus.GetInstance().GetClientInvoker("quest.update").Insert(this.OnQuestUpdate);

void OnQuestUpdate(string typeId, string payload, int fromId)
{
    // fromId is -1 for server-originated messages
}

// Server receives from client:
EIR_NetBus.GetInstance().GetServerInvoker("player.action").Insert(this.OnPlayerAction);

void OnPlayerAction(string typeId, string payload, int fromId)
{
    // fromId is the playerId of the sender
}
```

#### API - `EIR_NetBus`

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `GetInstance()` | - | `EIR_NetBus` | Returns the singleton. |
| `GetClientInvoker(typeId)` | `string typeId` | `EIR_NetHandlerInvoker` | Returns the invoker fired when a server-to-client message with `typeId` arrives on this peer. Use `.Insert(callback)` / `.Remove(callback)`. Created on first call if it does not exist. |
| `GetServerInvoker(typeId)` | `string typeId` | `EIR_NetHandlerInvoker` | Returns the invoker fired when a client-to-server message with `typeId` arrives on the server. Use `.Insert(callback)` / `.Remove(callback)`. |
| `DispatchToClient(typeId, payload, fromId)` | `string typeId`, `string payload`, `int fromId` | `void` | Called internally by `EIR_NetBusComponent` when a gateway RPC arrives. Do not call directly. |
| `DispatchToServer(typeId, payload, fromId)` | `string typeId`, `string payload`, `int fromId` | `void` | Called internally by `EIR_NetBusComponent`. Do not call directly. |

#### API - `EIR_NetBusComponent`

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `GetInstance()` | - | `EIR_NetBusComponent` | Returns the singleton instance. Logs a warning if the component has not been placed on the GameMode entity. |
| `SendToPlayer(playerId, typeId, payload)` | `int playerId`, `string typeId`, `string payload` | `void` | Sends a reliable message to a specific player. Call from the server only. |
| `SendToAll(typeId, payload)` | `string typeId`, `string payload` | `void` | Sends a reliable broadcast to all connected clients. Call from the server only. |
| `SendToServer(typeId, payload)` | `string typeId`, `string payload` | `void` | Sends a reliable message to the server. Call from a client. On a listen server the message is routed locally without a network hop. |

#### Callback Signature

```c
void MyCallback(string typeId, string payload, int fromId)
```

`fromId` is the sender's `playerId`, or `-1` when the message originated from the server itself.

---

### Logger

**File:** `Scripts/Game/Logging/EIR_Logger.c`

Structured logging with category prefixes and level filtering. Wraps `Print()` with consistent formatting and per-category output control.

Output format: `[EIR][category] message`

#### Quick Example

```c
EIR_Logger.Info("questing", "Player 5 completed sweep_stary");
EIR_Logger.Warning("netbus", "No EIR_NetBusComponent found");
EIR_Logger.Error("economy", "Balance underflow for player 3");

// Enable debug output globally:
EIR_Logger.SetDebugEnabled(true);
EIR_Logger.Debug("questing", "Step 2 evaluated");
EIR_Logger.Verbose("questing", "Checking kill condition");

// Silence a noisy category:
EIR_Logger.DisableCategory("questing");
EIR_Logger.EnableCategory("questing");
```

#### API - `EIR_Logger`

All methods are `static`. No instance is needed.

| Method | Parameters | Returns | LogLevel | Suppressed by default | Description |
|---|---|---|---|---|---|
| `SetDebugEnabled(enabled)` | `bool enabled` | `void` | - | - | Enables or disables `Debug` and `Verbose` output globally. Both are off by default. |
| `DisableCategory(category)` | `string category` | `void` | - | - | Suppresses all log output for `category` including `Warning` and `Error`. Use with care. |
| `EnableCategory(category)` | `string category` | `void` | - | - | Re-enables a previously disabled category. Safe to call for categories that were never disabled. |
| `Debug(category, message)` | `string category`, `string message` | `void` | `LogLevel.DEBUG` | Yes | Prints at debug level. Only output when `SetDebugEnabled(true)` has been called and the category is not disabled. |
| `Verbose(category, message)` | `string category`, `string message` | `void` | `LogLevel.VERBOSE` | Yes | Prints at verbose level. Same gating as `Debug`. |
| `Info(category, message)` | `string category`, `string message` | `void` | `LogLevel.NORMAL` | No | General information. Always printed unless the category is disabled. |
| `Warning(category, message)` | `string category`, `string message` | `void` | `LogLevel.WARNING` | No | Non-fatal problem. Always printed unless the category is disabled. |
| `Error(category, message)` | `string category`, `string message` | `void` | `LogLevel.ERROR` | No | Fatal or data-corrupting problem. Always printed unless the category is disabled. |

---

### Timer Manager

**File:** `Scripts/Game/Timers/EIR_TimerManager.c`

Named timers wrapping `GetGame().GetCallqueue()`. Cancel by name rather than holding a callback reference. Detect duplicate scheduling with `IsScheduled`. Scheduling the same name twice cancels the first timer automatically.

This file also defines `EIR_TimerInvoker`, which is reused by `EIR_Sequence`.

#### Quick Example

```c
// One-shot:
EIR_TimerInvoker inv = new EIR_TimerInvoker();
inv.Insert(this.OnDelayedAction);
EIR_TimerManager.GetInstance().Schedule("spawn.delay", inv, 5.0);

// Repeating:
EIR_TimerInvoker tick = new EIR_TimerInvoker();
tick.Insert(this.OnTick);
EIR_TimerManager.GetInstance().Schedule("economy.tick", tick, 60.0, true);

// Cancel:
EIR_TimerManager.GetInstance().Cancel("spawn.delay");

// Check:
if (EIR_TimerManager.GetInstance().IsScheduled("spawn.delay")) { ... }
```

Callbacks must have no parameters:
```c
void OnDelayedAction() { ... }
```

#### API - `EIR_TimerManager`

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `GetInstance()` | - | `EIR_TimerManager` | Returns the singleton. |
| `Schedule(name, invoker, delaySeconds, repeat)` | `string name`, `EIR_TimerInvoker invoker`, `float delaySeconds`, `bool repeat = false` | `void` | Schedules a timer. `delaySeconds` is the delay before the first fire and the interval between repeats for repeating timers. `repeat = false` makes it a one-shot that removes itself after firing. If a timer with the same `name` exists it is cancelled first. |
| `Cancel(name)` | `string name` | `void` | Cancels a scheduled timer by name. Safe to call if no timer by that name is currently scheduled. |
| `IsScheduled(name)` | `string name` | `bool` | Returns `true` if a timer with `name` is currently scheduled and has not yet fired (or is a repeating timer). |
| `OnEntryFired(name)` | `string name` | `void` | Internal - called by `EIR_TimerEntry` after a one-shot fires to remove it from the registry. Do not call directly. |

#### Types Defined in This File

```c
void EIR_TimerCallback();
typedef func EIR_TimerCallback;
typedef ScriptInvokerBase<EIR_TimerCallback> EIR_TimerInvoker;
```

`EIR_TimerInvoker` is also the callback container used by `EIR_Sequence`.

---

### State Machine

**File:** `Scripts/Game/StateMachine/EIR_StateMachine.c`

A simple named finite state machine. Register states, attach enter/exit callbacks, then drive transitions. Not a singleton - create one per system that needs managed state.

#### Quick Example

```c
EIR_StateMachine fsm = new EIR_StateMachine();

EIR_StateEntry idle = fsm.RegisterState("idle");
idle.GetOnEnter().Insert(this.OnEnterIdle);
idle.GetOnExit().Insert(this.OnExitIdle);

EIR_StateEntry combat = fsm.RegisterState("combat");
combat.GetOnEnter().Insert(this.OnEnterCombat);

fsm.Transition("idle");    // fires OnEnterIdle("", "idle")
fsm.Transition("combat");  // fires OnExitIdle("idle","combat"), then OnEnterCombat("idle","combat")
fsm.Transition("combat");  // no-op - already in this state

string current = fsm.GetCurrentState();  // "combat"
if (fsm.IsInState("idle")) { ... }

void OnEnterIdle(string prevState, string nextState) { ... }
void OnExitIdle(string prevState, string nextState)  { ... }
```

#### API - `EIR_StateEntry`

Returned by `EIR_StateMachine.RegisterState()`. Use it to attach callbacks before transitions begin.

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `GetName()` | - | `string` | Returns the name this state was registered with. |
| `GetOnEnter()` | - | `EIR_StateInvoker` | Returns the invoker called when this state is entered. Use `.Insert(callback)`. Callback signature: `void MyCallback(string prevState, string nextState)`. |
| `GetOnExit()` | - | `EIR_StateInvoker` | Returns the invoker called when this state is exited. Same callback signature as `GetOnEnter`. |

#### API - `EIR_StateMachine`

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `RegisterState(name)` | `string name` | `EIR_StateEntry` | Registers a state and returns its entry so you can attach callbacks. Overwrites any existing state registered under `name`. |
| `Transition(newState)` | `string newState` | `bool` | Transitions to `newState`. Fires the current state's `OnExit` then `newState`'s `OnEnter`. Returns `false` if `newState` is not registered or is already the current state. Logs a warning for unregistered state names. |
| `GetCurrentState()` | - | `string` | Returns the name of the current state. Returns `""` before any `Transition` call. |
| `IsInState(name)` | `string name` | `bool` | Returns `true` if the machine is currently in the named state. |
| `HasState(name)` | `string name` | `bool` | Returns `true` if the named state has been registered. |

#### Callback Signature

```c
void MyCallback(string prevState, string nextState)
```

On the very first transition, `prevState` is `""`.

---

### Sequence

**File:** `Scripts/Game/Sequence/EIR_Sequence.c`

Chains a series of timed actions. Each step fires after a configured delay from the previous step. Not a singleton - create one per use case.

#### Quick Example

```c
EIR_Sequence seq = new EIR_Sequence();

EIR_SequenceStep step1 = seq.AddStep(0);      // fires immediately on Start()
step1.GetInvoker().Insert(this.DoPhaseOne);

EIR_SequenceStep step2 = seq.AddStep(3.0);    // fires 3 seconds after step 1
step2.GetInvoker().Insert(this.DoPhaseTwo);

EIR_SequenceStep step3 = seq.AddStep(1.5);    // fires 1.5 seconds after step 2
step3.GetInvoker().Insert(this.DoPhaseThree);

seq.Start();

// Cancel at any point:
seq.Stop();
```

Callbacks must have no parameters:
```c
void DoPhaseOne()   { ... }
void DoPhaseTwo()   { ... }
```

#### API - `EIR_SequenceStep`

Returned by `EIR_Sequence.AddStep()`.

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `GetDelay()` | - | `float` | Returns the delay in seconds configured for this step. |
| `GetInvoker()` | - | `EIR_TimerInvoker` | Returns the invoker that fires when this step executes. Use `.Insert(callback)` to attach your action. Callback must have no parameters: `void MyCallback()`. |

#### API - `EIR_Sequence`

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `AddStep(delaySeconds)` | `float delaySeconds = 0` | `EIR_SequenceStep` | Appends a step to the sequence. `delaySeconds` is the wait after the previous step fires before this step executes. A delay of `0` fires synchronously within the same frame. Returns the step so you can insert callbacks. |
| `Start()` | - | `void` | Begins executing steps from the first one. If the sequence is already running it restarts from step 0, cancelling any pending step first. |
| `Stop()` | - | `void` | Cancels any pending step. Already-fired steps are not undone. Safe to call when the sequence is not running. |
| `IsRunning()` | - | `bool` | Returns `true` if the sequence is currently executing (started and not yet finished or stopped). |
| `GetCurrentStepIndex()` | - | `int` | Returns the zero-based index of the step that will fire next. |

---

### Cache

**File:** `Scripts/Game/Cache/EIR_Cache.c`

A TTL-based key/value cache for `Managed` objects. Entries expire automatically on access after their TTL. Not a singleton - create one per system.

#### Quick Example

```c
EIR_Cache cache = new EIR_Cache();

// Store a value for 30 seconds:
cache.Set("player.5.profile", profileObject, 30.0);

// Retrieve (returns null if expired or not found):
MyProfile profile = MyProfile.Cast(cache.Get("player.5.profile"));
if (!profile)
{
    // cache miss - fetch from source
}

// Check without retrieving:
if (cache.Has("player.5.profile")) { ... }

// Invalidate:
cache.Remove("player.5.profile");
cache.Clear();
```

#### API - `EIR_Cache`

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `Set(key, value, ttlSeconds)` | `string key`, `Managed value`, `float ttlSeconds` | `void` | Stores `value` under `key` with a TTL in seconds. Overwrites any existing entry for `key`. The entry expires after `ttlSeconds` seconds from the time of this call. |
| `Get(key)` | `string key` | `Managed` | Returns the cached value, or `null` if not present or expired. Expired entries are removed from the map on access. Cast the return value to your expected type. |
| `Has(key)` | `string key` | `bool` | Returns `true` if `key` exists and has not yet expired. Equivalent to `Get(key) != null`. |
| `Remove(key)` | `string key` | `void` | Removes the entry for `key` regardless of whether it has expired. Safe to call if `key` does not exist. |
| `Clear()` | - | `void` | Removes all entries, including ones that have not yet expired. |
| `Purge()` | - | `void` | Removes all entries that have already expired. Entries are evicted lazily on `Get`/`Has` by default; call `Purge()` periodically only when you have many entries that expire without being read. |
| `Count()` | - | `int` | Returns the total number of stored entries, including expired ones not yet purged. |

---

### Rate Limiter

**File:** `Scripts/Game/RateLimiter/EIR_RateLimiter.c`

A per-key cooldown limiter. Prevents the same action from firing more often than a configured interval. Not a singleton - create one per action category.

#### Quick Example

```c
// One limiter per notification type, shared across all players:
EIR_RateLimiter notifLimiter = new EIR_RateLimiter(5.0); // 5-second cooldown

string key = string.ToString(playerId);
if (!notifLimiter.Allow(key))
    return;  // still on cooldown

// Compound key for per-player per-event limiting:
string key = string.Format("%1.%2", playerId, eventName);
if (!limiter.Allow(key)) return;
```

#### API - `EIR_RateLimiter`

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `EIR_RateLimiter(cooldownSeconds)` | `float cooldownSeconds` | - | Constructor. Sets the minimum gap between allowed calls for any given key. |
| `Allow(key)` | `string key` | `bool` | Returns `true` if the action is permitted for `key` and records the current time as the last allowed time. Returns `false` if `key` is still within the cooldown window. Calling `Allow` does not update the timestamp when it returns `false`. |
| `IsLimited(key)` | `string key` | `bool` | Returns `true` if `key` is currently within the cooldown window (i.e. `Allow` would return `false`). Does not update the timestamp. |
| `GetRemainingCooldown(key)` | `string key` | `float` | Returns the number of seconds remaining on the cooldown for `key`. Returns `0` if the key is not limited or has never been allowed. |
| `Reset(key)` | `string key` | `void` | Clears the cooldown record for `key`, allowing it to fire immediately on the next `Allow` call. Safe to call for keys that have never been used. |
| `ResetAll()` | - | `void` | Clears all cooldown records. |
| `SetCooldown(cooldownSeconds)` | `float cooldownSeconds` | `void` | Changes the cooldown duration going forward. Does not retroactively affect already-running cooldowns. |
| `GetCooldown()` | - | `float` | Returns the currently configured cooldown duration in seconds. |

---

### JSON Builder

**File:** `Scripts/Game/Json/EIR_JsonBuilder.c`

A fluent builder for constructing JSON strings. Enfusion has no built-in JSON serialiser. Create builders using the static factory methods rather than `new`.

#### Quick Example

```c
// Object:
EIR_JsonBuilder b = EIR_JsonBuilder.NewObject();
b.AddString("questId", "sweep_stary");
b.AddInt("playerId", 5);
b.AddFloat("progress", 0.75);
b.AddBool("completed", false);
b.AddNull("reward");
string json = b.Build();
// -> {"questId":"sweep_stary","playerId":5,"progress":0.750,"completed":false,"reward":null}

// Array:
EIR_JsonBuilder arr = EIR_JsonBuilder.NewArray();
arr.PushString("alpha");
arr.PushInt(42);
arr.PushBool(true);
arr.PushNull();
string json = arr.Build();
// -> ["alpha",42,true,null]

// Nested:
EIR_JsonBuilder inner = EIR_JsonBuilder.NewObject();
inner.AddInt("x", 10);
inner.AddInt("y", 20);

EIR_JsonBuilder outer = EIR_JsonBuilder.NewObject();
outer.AddString("name", "spawn");
outer.AddRaw("position", inner.Build());
string json = outer.Build();
// -> {"name":"spawn","position":{"x":10,"y":20}}
```

#### API - `EIR_JsonBuilder`

**Factory methods:**

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `NewObject()` | - | `EIR_JsonBuilder` | Creates a builder for a JSON object `{ }`. |
| `NewArray()` | - | `EIR_JsonBuilder` | Creates a builder for a JSON array `[ ]`. |

**Object methods** (use on a builder created with `NewObject()`):

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `AddString(key, value)` | `string key`, `string value` | `void` | Adds a string field. The value is JSON-escaped automatically (handles embedded double quotes). |
| `AddInt(key, value)` | `string key`, `int value` | `void` | Adds an integer field. |
| `AddFloat(key, value, decimals)` | `string key`, `float value`, `int decimals = 3` | `void` | Adds a float field. `decimals` controls the number of decimal places in the output (default `3`). |
| `AddBool(key, value)` | `string key`, `bool value` | `void` | Adds a boolean field, written as `true` or `false`. |
| `AddNull(key)` | `string key` | `void` | Adds a `null` field. |
| `AddRaw(key, rawJson)` | `string key`, `string rawJson` | `void` | Adds an already-serialised JSON value (object, array, number literal, etc.) without any further escaping. Use this to nest one builder's output inside another. |

**Array methods** (use on a builder created with `NewArray()`):

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `PushString(value)` | `string value` | `void` | Appends a JSON-escaped string element. |
| `PushInt(value)` | `int value` | `void` | Appends an integer element. |
| `PushFloat(value, decimals)` | `float value`, `int decimals = 3` | `void` | Appends a float element. `decimals` controls decimal places (default `3`). |
| `PushBool(value)` | `bool value` | `void` | Appends a boolean element (`true` or `false`). |
| `PushNull()` | - | `void` | Appends a `null` element. |
| `PushRaw(rawJson)` | `string rawJson` | `void` | Appends an already-serialised JSON value without further escaping. |

**Finalise:**

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `Build()` | - | `string` | Returns the completed JSON string, wrapped in `{ }` for an object builder or `[ ]` for an array builder. |

---

## Utilities

---

### Player Utils

**File:** `Scripts/Game/Utils/EIR_PlayerUtils.c`

Static helpers for the most common player resolution boilerplate.

#### Quick Example

```c
if (!EIR_PlayerUtils.IsValidId(playerId)) return;

IEntity ent  = EIR_PlayerUtils.GetEntityFromId(playerId);
string  name = EIR_PlayerUtils.GetName(playerId);
string  desc = EIR_PlayerUtils.Describe(playerId);  // "Alice (id:5)"

array<int> players = {};
int count = EIR_PlayerUtils.GetAllConnectedIds(players);

int foundId;
if (EIR_PlayerUtils.TryGetIdFromEntity(someEntity, foundId))
    Print("Controlled by player: " + foundId.ToString());
```

#### API - `EIR_PlayerUtils`

All methods are `static`. No instance is needed.

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `IsValidId(playerId)` | `int playerId` | `bool` | Returns `true` if `playerId` is greater than zero. This is a basic sanity check and does not verify the player is currently connected. |
| `GetEntityFromId(playerId)` | `int playerId` | `IEntity` | Returns the entity currently controlled by the given player, or `null` if not found or the ID is invalid. |
| `GetName(playerId)` | `int playerId` | `string` | Returns the display name of the given player as reported by `PlayerManager`, or `""` if not found. |
| `Describe(playerId)` | `int playerId` | `string` | Returns a formatted `"Name (id:5)"` string for use in log messages. Falls back to `"(id:5)"` if no name is available. |
| `GetAllConnectedIds(outPlayers)` | `out array<int> outPlayers` | `int` | Fills `outPlayers` with the IDs of all currently connected players. Clears the array before filling. Returns the count. |
| `TryGetIdFromEntity(entity, outPlayerId)` | `IEntity entity`, `out int outPlayerId` | `bool` | Returns `true` if `entity` is currently controlled by any player and writes that player's ID into `outPlayerId`. Returns `false` and sets `outPlayerId` to `-1` if no player controls it. |

---

### String Utils

**File:** `Scripts/Game/Utils/EIR_StringUtils.c`

Static string helpers that complement Enfusion's built-in `string` proto methods and `SCR_StringHelper`. Does not duplicate: `string.Split`, `SCR_StringHelper.Join`, `SCR_StringHelper.TrimLeft`, `SCR_StringHelper.TrimRight`.

#### API - `EIR_StringUtils`

All methods are `static`. No instance is needed.

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `IsEmpty(str)` | `string str` | `bool` | Returns `true` if `str` has a length of zero. Enfusion strings cannot be null but can be empty. |
| `StartsWith(str, prefix)` | `string str`, `string prefix` | `bool` | Returns `true` if `str` begins with `prefix`. Returns `true` for an empty `prefix`. |
| `EndsWith(str, suffix)` | `string str`, `string suffix` | `bool` | Returns `true` if `str` ends with `suffix`. Returns `true` for an empty `suffix`. |
| `Contains(str, substr)` | `string str`, `string substr` | `bool` | Returns `true` if `substr` appears anywhere within `str`. Returns `true` for an empty `substr`. |
| `Trim(str)` | `string str` | `string` | Strips leading and trailing space and tab characters. Delegates to `SCR_StringHelper.TrimLeft` and `TrimRight`. |
| `PadRight(str, targetLength)` | `string str`, `int targetLength` | `string` | Pads `str` on the right with spaces until it reaches `targetLength` characters. Returns `str` unchanged if it is already at or beyond that length. |
| `PadLeft(str, targetLength)` | `string str`, `int targetLength` | `string` | Pads `str` on the left with spaces until it reaches `targetLength` characters. Returns `str` unchanged if already at or beyond that length. |
| `Repeat(str, count)` | `string str`, `int count` | `string` | Concatenates `str` with itself `count` times and returns the result. Returns `""` for `count = 0`. |

---

### Math Utils

**File:** `Scripts/Game/Utils/EIR_MathUtils.c`

Static math helpers that complement the built-in `Math` class. Does not duplicate: `Math.Clamp`, `Math.Lerp`, `Math.Map`, `Math.InverseLerp`, `Math.RandomFloat`, `Math.RandomInt`, `Math.Round`, `Math.Abs`, etc.

#### API - `EIR_MathUtils`

All methods are `static`. No instance is needed.

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `Sign(value)` | `float value` | `int` | Returns `-1` if `value < 0`, `0` if `value == 0`, `1` if `value > 0`. |
| `ApproximatelyEqual(a, b, epsilon)` | `float a`, `float b`, `float epsilon = 0.0001` | `bool` | Returns `true` if `a` and `b` differ by no more than `epsilon`. Use instead of `==` for float comparisons. Default epsilon is `0.0001`. |
| `RemapClamped(value, inMin, inMax, outMin, outMax)` | `float value`, `float inMin`, `float inMax`, `float outMin`, `float outMax` | `float` | Remaps `value` from the range `[inMin, inMax]` to `[outMin, outMax]` and clamps the result within the output range. |
| `RandomPointInCircle(origin, radius)` | `vector origin`, `float radius` | `vector` | Returns a uniformly distributed random point within a 2D circle of `radius` centered on `origin`. The Y component of `origin` is preserved unchanged in the result. |
| `RandomPointInSphere(origin, radius)` | `vector origin`, `float radius` | `vector` | Returns a uniformly distributed random point within a sphere of `radius` centered on `origin`. All three axes are randomised. |
| `Wrap(value, min, max)` | `float value`, `float min`, `float max` | `float` | Wraps `value` within `[min, max]` with modular wraparound rather than clamping. Example: `Wrap(370, 0, 360)` returns `10`. Returns `min` if `max <= min`. |
| `Percent(value, min, max)` | `float value`, `float min`, `float max` | `float` | Returns where `value` sits within `[min, max]` as a percentage from `0` to `100`, clamped. Returns `0` if `max <= min`. |
| `InRange(value, min, max)` | `float value`, `float min`, `float max` | `bool` | Returns `true` if `value >= min` and `value <= max` (inclusive bounds). |
| `RoundTo(value, decimals)` | `float value`, `int decimals` | `float` | Rounds `value` to `decimals` decimal places and returns the result as a float. |

---

### Component Utils

**File:** `Scripts/Game/Utils/EIR_ComponentUtils.c`

Static shorthand wrappers for common component search patterns. All methods return `Managed` - cast the result to your expected type.

#### Quick Example

```c
HealthComponent hp = HealthComponent.Cast(EIR_ComponentUtils.OnSelf(entity, HealthComponent));
InventoryComponent inv = InventoryComponent.Cast(EIR_ComponentUtils.WalkUp(entity, InventoryComponent));

array<Managed> weapons = {};
EIR_ComponentUtils.CollectFromChildren(entity, WeaponComponent, weapons);
```

#### API - `EIR_ComponentUtils`

All methods are `static`. No instance is needed.

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `OnSelf(entity, componentType)` | `notnull IEntity entity`, `typename componentType` | `Managed` | Searches for `componentType` on `entity` itself only. Returns `null` if not found. |
| `InParents(entity, componentType)` | `notnull IEntity entity`, `typename componentType` | `Managed` | Searches `entity` first, then its direct parent. Returns the first match found, or `null`. |
| `OnRoot(entity, componentType)` | `notnull IEntity entity`, `typename componentType` | `Managed` | Searches for `componentType` on the root-most ancestor in the hierarchy. Falls back to `entity` itself if it has no parent. |
| `InChildren(entity, componentType)` | `notnull IEntity entity`, `typename componentType` | `Managed` | Searches direct children of `entity` one level deep (not recursive). Returns the first match found, or `null`. |
| `WalkUp(entity, componentType)` | `notnull IEntity entity`, `typename componentType` | `Managed` | Searches `entity` itself, then walks up the parent chain to the root until the component is found. Returns `null` if not found anywhere. |
| `CollectFromChildren(entity, componentType, outComponents)` | `notnull IEntity entity`, `typename componentType`, `out array<Managed> outComponents` | `int` | Fills `outComponents` with all instances of `componentType` found on direct children. Clears the array first. Returns the count found. |

---

### Collections

**File:** `Scripts/Game/Utils/EIR_Collections.c`

Static array utility methods. Because Enfusion has no generics, helpers are provided per element type. Covered types: `int`, `string`, `IEntity`.

#### API - `EIR_Collections`

All methods are `static`. No instance is needed.

**Random element:**

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `RandomEntity(arr)` | `notnull array<IEntity> arr` | `IEntity` | Returns a uniformly random element from `arr`. Returns `null` if `arr` is empty. |
| `RandomInt(arr)` | `notnull array<int> arr` | `int` | Returns a uniformly random element from `arr`. Returns `0` if `arr` is empty. |
| `RandomString(arr)` | `notnull array<string> arr` | `string` | Returns a uniformly random element from `arr`. Returns `""` if `arr` is empty. |

**First / Last:**

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `FirstEntity(arr)` | `notnull array<IEntity> arr` | `IEntity` | Returns the element at index 0, or `null` if `arr` is empty. |
| `LastEntity(arr)` | `notnull array<IEntity> arr` | `IEntity` | Returns the element at the last index, or `null` if `arr` is empty. |
| `FirstString(arr)` | `notnull array<string> arr` | `string` | Returns the element at index 0, or `""` if `arr` is empty. |
| `LastString(arr)` | `notnull array<string> arr` | `string` | Returns the element at the last index, or `""` if `arr` is empty. |

**Shuffle (Fisher-Yates, in-place):**

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `ShuffleEntities(arr)` | `notnull array<IEntity> arr` | `void` | Randomly reorders the elements of `arr` in place using a Fisher-Yates shuffle. |
| `ShuffleInts(arr)` | `notnull array<int> arr` | `void` | Randomly reorders the elements of `arr` in place. |
| `ShuffleStrings(arr)` | `notnull array<string> arr` | `void` | Randomly reorders the elements of `arr` in place. |

**Contains:**

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `ContainsString(arr, value)` | `notnull array<string> arr`, `string value` | `bool` | Returns `true` if `value` is present in `arr`. |
| `ContainsInt(arr, value)` | `notnull array<int> arr`, `int value` | `bool` | Returns `true` if `value` is present in `arr`. |

**Deduplicate (in-place):**

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `DeduplicateStrings(arr)` | `notnull array<string> arr` | `void` | Removes duplicate strings from `arr` in place, keeping the first occurrence of each value. O(n^2). |
| `DeduplicateInts(arr)` | `notnull array<int> arr` | `void` | Removes duplicate ints from `arr` in place, keeping the first occurrence of each value. O(n^2). |

---

### World Utils

**File:** `Scripts/Game/Utils/EIR_WorldUtils.c`

Static spatial and trace query helpers. Wraps `QueryEntitiesBySphere` and `TraceMove` for common use cases.

#### API - `EIR_WorldUtils`

All methods are `static`. No instance is needed.

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `FindEntitiesInRadius(origin, radius, outEntities, flags)` | `vector origin`, `float radius`, `out array<IEntity> outEntities`, `EQueryEntitiesFlags flags = EQueryEntitiesFlags.ALL` | `int` | Fills `outEntities` with all entities within `radius` metres of `origin`. `flags` defaults to all entity types; pass `EQueryEntitiesFlags` values to narrow the search. Clears the array first. Returns the count found. |
| `FindClosestEntityInRadius(origin, radius, flags)` | `vector origin`, `float radius`, `EQueryEntitiesFlags flags = EQueryEntitiesFlags.ALL` | `IEntity` | Returns the single closest entity within `radius` of `origin`, or `null` if none are found. Uses the same sphere query as `FindEntitiesInRadius` then picks the nearest by squared distance. |
| `GetSurfaceY(pos, traceHeight)` | `vector pos`, `float traceHeight = 500.0` | `float` | Returns the Y (height) of the terrain or surface directly below `pos`. Traces downward from `pos.Y + traceHeight`. Returns `pos.Y` if no surface is hit. Default `traceHeight` is `500`. |
| `HasLineOfSight(from, to, exclude)` | `vector from`, `vector to`, `IEntity exclude = null` | `bool` | Returns `true` if a ray from `from` to `to` travels the full distance without hitting terrain or any entity. `exclude` is typically the entity performing the check so it is not counted as an obstruction. |
| `TracePosition(start, end, exclude)` | `vector start`, `vector end`, `IEntity exclude = null` | `vector` | Returns the world position where a ray from `start` toward `end` first hits terrain or an entity. Returns `end` unchanged if nothing is hit. `exclude` is skipped during the trace. |

---

### Replication Utils

**File:** `Scripts/Game/Utils/EIR_ReplicationUtils.c`

Static wrappers for common `Replication` / `RplSession` peer role checks. Makes intent explicit and removes repetitive boilerplate.

#### API - `EIR_ReplicationUtils`

All methods are `static`. No instance is needed.

| Method | Parameters | Returns | Description |
|---|---|---|---|
| `IsServer()` | - | `bool` | Returns `true` on any server peer - both dedicated and listen server. Wraps `Replication.IsServer()`. |
| `IsDedicatedServer()` | - | `bool` | Returns `true` only on a dedicated server (no local player). Checks `RplSession.Mode() == RplMode.Dedicated`. |
| `IsListenServer()` | - | `bool` | Returns `true` when running as a listen server - the host is both server and a connected client. |
| `IsClient()` | - | `bool` | Returns `true` on client peers. Returns `false` on any server peer. |
| `IsMultiplayer()` | - | `bool` | Returns `true` if a replication session is active (any multiplayer session). Returns `false` in singleplayer or local editor play. |
| `GetPeerRole()` | - | `string` | Returns a human-readable string describing the current peer: `"DedicatedServer"`, `"ListenServer"`, `"Client"`, or `"Singleplayer"`. Useful for log messages. |

---

## Event Name Constants

**File:** `Scripts/Game/EventBusInterop/EIR_EventNames.c`

Predefined constants for standard EIR event names. Use these with `EIR_EventBus` instead of raw strings to avoid typos and keep all names in one place.

| Constant | String Value | When It Fires |
|---|---|---|
| `EIR_EventNames.PLAYER_CONNECTED` | `"eir.player.connected"` | A player connects and their data has been loaded. |
| `EIR_EventNames.PLAYER_DISCONNECTED` | `"eir.player.disconnected"` | A player disconnects after their data has been saved. |
| `EIR_EventNames.QUEST_STARTED` | `"eir.quest.started"` | A player starts a new quest. |
| `EIR_EventNames.QUEST_PROGRESS` | `"eir.quest.progress"` | A player makes progress on an active quest objective. |
| `EIR_EventNames.QUEST_COMPLETED` | `"eir.quest.completed"` | A player completes a quest. |
| `EIR_EventNames.QUEST_ABANDONED` | `"eir.quest.abandoned"` | A player's quest is abandoned or reset. |
| `EIR_EventNames.SERVER_READY` | `"eir.server.ready"` | The server finishes initialising all core systems. |
| `EIR_EventNames.ECONOMY_BALANCE_CHANGED` | `"eir.economy.balance.changed"` | A player's currency balance changes. |
| `EIR_EventNames.ECONOMY_PURCHASE` | `"eir.economy.purchase"` | A player completes a purchase. |

Convention for custom events in your own mods: `"author.category.action"`, e.g. `"myaddon.base.captured"`.

---

## Script Module Structure

```
Scripts/
  Game/
    Cache/
      EIR_Cache.c
    EventBusInterop/
      EIR_EventBus.c
      EIR_EventNames.c
      EIR_EventPayload.c
    FeatureRegistry/
      EIR_FeatureRegistry.c
    Json/
      EIR_JsonBuilder.c
    Logging/
      EIR_Logger.c
    NetBus/
      EIR_NetBus.c
      EIR_NetBusComponent.c
    RateLimiter/
      EIR_RateLimiter.c
    Sequence/
      EIR_Sequence.c
    StateMachine/
      EIR_StateMachine.c
    Timers/
      EIR_TimerManager.c
    Utils/
      EIR_Collections.c
      EIR_ComponentUtils.c
      EIR_MathUtils.c
      EIR_PlayerUtils.c
      EIR_ReplicationUtils.c
      EIR_StringUtils.c
      EIR_WorldUtils.c
```
