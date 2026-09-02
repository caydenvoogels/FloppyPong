\# AGENTS.md



\## Scope



Repository-wide instructions for AI coding agents and contributors.



A more specific `AGENTS.md` in a subdirectory may add or override rules for that subtree.



\## Project



\- Unreal Engine 5.8.1

\- Windows

\- C++-first; almost all gameplay and systems belong in C++

\- Single-player, first-person tactical / research / mystery game

\- Two-person development team

\- Future cooperative multiplayer

\- Steam integration

\- Full offline play is mandatory



Design today's single-player code so future cooperative multiplayer is not needlessly blocked, but do not add networking complexity before it is required.



\---



\# Non-Negotiable Rules



1\. \*\*Inspect existing code before editing.\*\* Match established nearby patterns when they are compatible with this file.

2\. \*\*Keep responsibilities small and explicit.\*\* No god objects, giant managers, catch-all subsystems, or unrelated responsibilities in one class.

3\. \*\*Apply SOLID pragmatically.\*\* Favor maintainability and clear ownership over shortcuts, but do not create abstractions without a real purpose.

4\. \*\*Prefer composition over deep inheritance.\*\*

5\. \*\*Tick is opt-in.\*\* Do not add per-frame polling when an event, delegate, timer, callback, or state change can solve the problem.

6\. \*\*C++ owns gameplay architecture.\*\* Blueprint exposure must be intentional and minimal.

7\. \*\*Do not assume one global player.\*\* Avoid player-index-0 and `GetFirstPlayerController()` dependencies in reusable gameplay code.

8\. \*\*Steam must not be required for offline play.\*\*

9\. \*\*Do not modify unrelated files or perform opportunistic refactors.\*\*

10\. \*\*Ask before making a new project-wide architectural decision.\*\*

11\. \*\*Compile the affected module/target and run relevant tests before claiming completion.\*\*

12\. \*\*Keep documentation high-signal.\*\* Update durable system docs; do not create a Markdown file for every task.



If existing code conflicts with a hard rule above, do not silently propagate the conflict. Raise it before extending the pattern.



\---



\# Architecture



Use a \*\*feature-oriented, composition-first Unreal C++ architecture\*\*.



Organize code by coherent gameplay/system responsibility. Prefer dependencies that flow from presentation into stable gameplay APIs and then into focused behavior/data layers.



Platform integrations such as Steam must sit behind project-owned integration boundaries rather than leaking through core gameplay code.



Avoid circular feature dependencies and hidden global state.



\## Unreal Framework Responsibilities



Use Unreal framework classes narrowly:



\- \*\*GameMode\*\*: game rules, spawning policy, authoritative session/world flow.

\- \*\*GameState\*\*: shared session/world state that may later need replication.

\- \*\*PlayerController\*\*: player intent, possession coordination, input-facing orchestration.

\- \*\*Pawn/Character\*\*: physical representation, movement, embodiment.

\- \*\*PlayerState\*\*: per-player state that should survive pawn changes and may later replicate.

\- \*\*GameInstance\*\*: thin application/session bootstrap and true across-map concerns.



Do not turn any of these into general-purpose managers.



\## Actor Components



Prefer `UActorComponent` for focused behavior that:



\- belongs to an Actor;

\- shares the Actor's lifetime;

\- is reusable or independently understandable;

\- represents a real capability.



Examples may include interaction, scanning, equipment, inventory ownership, research state, or other focused behaviors.



Do not create components only to move lines out of a large class. The component must represent a coherent responsibility.



\## UObjects and Plain C++



Use `UObject` when reflection, garbage collection, serialization, editor integration, delegates, or Unreal lifetime semantics are useful.



Use plain C++ types when Unreal object semantics provide no benefit.



Use reflected `USTRUCT`s for structured data when reflection, serialization, editor exposure, or eventual networking makes that useful.



\## Subsystems



Use a subsystem only when its responsibility naturally matches its lifetime:



\- `UWorldSubsystem`: world-scoped service;

\- `UGameInstanceSubsystem`: true game-instance/session service;

\- `ULocalPlayerSubsystem`: per-local-player service.



A subsystem is not permission to create a global manager.



Do not use service-locator or singleton patterns when ownership can be explicit.



\## Interfaces and Events



Use interfaces when they create a real decoupling boundary or multiple implementations are meaningful.



Keep interfaces small.



Use delegates/events when the sender should not know concrete receivers.



Do not introduce a global event bus unless explicitly approved.



Prefer explicit dependencies when they are clearer than event-driven indirection.



\---



\# SOLID and Class Design



Apply SOLID as a practical design constraint.



\- \*\*Single Responsibility:\*\* one coherent reason to change.

\- \*\*Open/Closed:\*\* provide extension points where variation is genuinely expected.

\- \*\*Liskov:\*\* inherit only when derived types preserve the base contract.

\- \*\*Interface Segregation:\*\* prefer small capability-focused interfaces.

\- \*\*Dependency Inversion:\*\* high-level gameplay must not depend directly on volatile/platform-specific implementations.



Do not use SOLID as an excuse for needless interfaces, factories, wrappers, or micro-classes.



A function may be long if it performs one coherent algorithm and splitting it would make the code harder to understand. Split functions when responsibilities, naming, reuse, testing, or readability genuinely improve.



No arbitrary line-count limits.



\---



\# Future Cooperative Multiplayer



Do not implement replication, RPCs, prediction, or network infrastructure unless the task requires it.



Instead, preserve ownership boundaries that make future networking possible.



Required habits:



\- Do not assume player index `0` is the relevant player.

\- Do not use `GetFirstPlayerController()` as a general dependency lookup.

\- Pass relevant player/controller/pawn/local-player context explicitly.

\- Keep per-player state separate from shared world/session state.

\- Keep local presentation state separate from gameplay state.

\- Avoid global mutable gameplay state.

\- Model actions around the actor/player performing them.

\- Place player concepts where they can later map naturally to PlayerState or player-scoped systems.

\- Place shared session concepts where they can later map naturally to GameState or authoritative world systems.



When cooperative networking is actually introduced, define authority, replication, RPC, prediction, and reconciliation rules explicitly then.



\---



\# Steam and Offline Play



Steam is an integration, not a core gameplay dependency.



The full single-player game must remain usable when:



\- Steam is not running;

\- Steam is unavailable;

\- internet access is unavailable;

\- Steam initialization fails.



Core gameplay must not call Steam APIs directly when a narrow project-owned adapter/service can isolate them.



Platform integration must degrade gracefully.



Gameplay startup, save/load, progression, settings, input, and normal offline flow must not require online services.



Future cooperative online functionality may depend on Steam/online services; offline single-player must remain independently functional.



\---



\# Unreal C++ Conventions



Follow Epic's Unreal Engine C++ Coding Standard unless this repository explicitly overrides it.



Prefer Unreal idioms over generic C++ patterns when interacting with Unreal-managed objects.



\## Object References and Lifetime



Choose reference types by semantics, not mechanically.



\- Prefer `TObjectPtr<T>` for reflected UObject member references where appropriate.

\- Use `UPROPERTY` when a UObject reference must participate in GC, serialization, editor exposure, or reflection.

\- Use `TWeakObjectPtr<T>` for non-owning UObject references whose targets may disappear.

\- Use `TSoftObjectPtr<T>` / `TSoftClassPtr<T>` when deferred asset/class loading is appropriate.

\- Use `TSubclassOf<T>` for constrained UObject class references.

\- Use `TUniquePtr` / `TSharedPtr` only for appropriate non-UObject C++ ownership.

\- Do not use shared-pointer ownership for UObjects.

\- Raw pointers are acceptable for clear, transient, non-owning use.



Use `const` when it accurately expresses intent.



Respect UObject, Actor, world, delegate, timer, and asynchronous-operation lifetimes.



\## Constructors and Initialization



Constructors should establish defaults and create default subobjects.



Do not perform world-dependent gameplay work in UObject/Actor constructors.



Use the correct lifecycle callback for initialization requiring a world, possession, runtime objects, or loaded assets.



Unregister delegates, timers, callbacks, and external integrations when their owner no longer needs them.



Avoid callbacks that can outlive their owner.



\---



\# Blueprint Policy



This is a C++-first project.



Core gameplay rules, reusable logic, state ownership, and system architecture belong in C++ unless there is a concrete reason otherwise.



Blueprints are appropriate for presentation, asset assembly, animation/effects hookup, designer defaults, and deliberately exposed extension points.



Expose to Blueprint only intentionally.



Prefer:



\- `BlueprintReadOnly` over `BlueprintReadWrite`;

\- narrow `BlueprintCallable` APIs over mutable public state;

\- `EditDefaultsOnly` for designer-tunable configuration.



Avoid broad `EditAnywhere` / write access without a specific need.



Every Blueprint-facing property/function must have:



\- a sensible `Category`;

\- appropriate metadata;

\- a clear tooltip or documentation comment;

\- the narrowest useful visibility and mutability.



Do not expose private implementation details simply for convenience.



\---



\# Gameplay Configuration



Prefer `UPROPERTY(EditDefaultsOnly)` for gameplay tuning values that naturally belong to class/default configuration.



Avoid magic numbers, magic strings, hard-coded names, and hard-coded asset paths.



Use appropriate named constants, enums, `FName`, structs, class defaults, or another already-established project mechanism.



Gameplay Tags or new data/configuration frameworks may be used when the project intentionally adopts them. Do not introduce a new project-wide data architecture without asking first.



\---



\# Tick and Runtime Work



Tick is disabled by default.



Before adding Tick, consider:



\- events/delegates;

\- timers;

\- callbacks;

\- state changes;

\- input events;

\- animation notifications;

\- asynchronous completion events;

\- engine lifecycle callbacks.



If Tick is genuinely required:



\- keep work small;

\- avoid expensive searches and allocations;

\- disable Tick while inactive when practical;

\- briefly document the non-obvious reason it must tick.



Do not use per-frame polling as the default solution.



\---



\# Unreal Anti-Patterns



Do not introduce these without strong task-specific justification:



\- god objects or giant manager classes;

\- catch-all GameInstance/GameMode/GameState/Controller/Subsystem classes;

\- static mutable global gameplay state;

\- service locators;

\- gameplay logic inside UI widgets;

\- unnecessary Tick;

\- repeated `GetAllActorsOfClass` as a lookup strategy;

\- repeated world searches for dependencies with clear ownership;

\- excessive casting;

\- hard-coded asset paths;

\- synchronous asset loading during active gameplay when deferred loading is appropriate;

\- deep inheritance trees used only for code reuse;

\- cross-feature access to private implementation details;

\- incorrectly tracked UObject references;

\- assumptions that the first player/controller is always the relevant one;

\- world-dependent constructor logic;

\- `BeginPlay` as a dumping ground for unrelated initialization;

\- platform/Steam calls spread through gameplay code;

\- duplicated gameplay rules;

\- broad Blueprint mutation access;

\- permanent `LogTemp` usage.



For plain C++ types, prefer explicit dependency injection where practical. For Unreal-created objects, use clear initialization/factory/ownership patterns rather than hidden global lookups.



\---



\# Asset Loading



Avoid synchronous asset loading during active gameplay unless there is a clear, understood reason.



Prefer soft references and managed/asynchronous loading when assets do not need to be resident immediately.



Do not scatter asset loading across consumers. Give loading responsibility to a clear owner.



Do not hard-code package/object paths in gameplay code.



\---



\# Errors, Assertions, and Logging



Use Unreal diagnostic idioms deliberately.



\- Expected runtime condition: normal branching.

\- Programmer invariant that must hold: `check` / `checkf`.

\- Unexpected but recoverable invariant failure: `ensure` / `ensureMsgf`.

\- User/input validation: normal validation, not assertions.



Prefer dedicated log categories.



Do not commit permanent `LogTemp`.



Log actionable context rather than noise.



Avoid defensive checks that silently hide broken invariants.



\---



\# Headers and Includes



Strict IWYU is not required.



Do not perform broad include/header cleanup unless relevant to the task.



Still avoid obvious circular dependencies and unnecessary coupling.



Use forward declarations when they clearly help without harming readability.



Follow Unreal generated-header rules.



Never edit generated Unreal files manually.



\---



\# Comments



Keep source comments minimal.



Code should explain \*\*what\*\* it does through structure and naming.



Comments should explain non-obvious \*\*why\*\*, such as:



\- engine/platform constraints;

\- lifetime assumptions;

\- subtle invariants;

\- intentional tradeoffs;

\- temporary workarounds and removal conditions.



Do not narrate obvious code.



Public C++ APIs should use concise Unreal-style `/\*\* ... \*/` documentation when their contract is not completely obvious.



Document important preconditions, lifetime/ownership, side effects, or return semantics when relevant.



\---



\# `docs/` Documentation



Maintain durable system documentation under repository-level `docs/`.



Documentation is organized by \*\*system\*\*, not by task/prompt.



Do not create a Markdown file for every change.



Appropriate documents include areas such as:



\- input and keybinds;

\- interaction;

\- research/investigation systems;

\- save/load;

\- Steam integration;

\- future cooperative architecture;

\- inventory/equipment;

\- UI architecture;

\- testing strategy.



Update an existing system document when a change affects how another developer must understand, configure, use, extend, or validate that system.



Create a new document only for a durable system with no suitable existing home.



Keep docs concise and high-signal. Include only useful information such as ownership, high-level flow, invariants, configuration, extension points, integration boundaries, and validation steps.



Do not duplicate source code line-by-line.



Keybind/input behavior changes must update the relevant input documentation.



\---



\# Automated Testing



Automated testing is part of normal development.



Use Unreal Engine C++ Automation Tests for testable gameplay/system behavior.



Prefer the smallest useful level:



\- deterministic/unit-style automation tests for logic;

\- engine/UObject-aware automation tests when needed;

\- functional/world tests only when behavior genuinely depends on Actors, worlds, maps, or integrated gameplay.



When fixing a reasonably testable bug, add a regression test.



When adding non-trivial logic, add focused tests for its important behavior and edge cases when practical.



Tests should be deterministic and independent of:



\- Steam availability;

\- internet access;

\- developer-specific machine state;

\- unrelated content.



Structure Steam/platform boundaries so core behavior can be tested without the real platform service.



If no test layout exists yet, prefer `Private/Tests/` inside the relevant module.



\---



\# Required Verification



For code changes:



1\. Compile the affected module/target.

2\. Run relevant automated tests.

3\. Fix build errors and test failures caused by the change.

4\. Report what was actually verified.

5\. If compilation/tests cannot be run in the current environment, say so explicitly.



Do not claim a build or test passed unless it actually ran.



Do not spend unrelated task time cleaning pre-existing warnings.



Avoid introducing new warnings, but warning cleanup is not a completion requirement unless explicitly requested.



\---



\# Change Scope



Make the smallest \*\*coherent\*\* change that correctly solves the task.



Do not modify unrelated code.



Do not opportunistically:



\- refactor neighboring systems;

\- rename unrelated code;

\- reformat unrelated files;

\- clean unrelated warnings;

\- reorganize directories;

\- change unrelated public APIs;

\- rewrite working systems.



For bug fixes, prefer the smallest root-cause fix with acceptable architectural quality.



Broad refactors require explicit direction.



If you notice an unrelated correctness, architecture, maintainability, or performance issue, mention it separately instead of silently changing it.



\---



\# Performance



Do not perform speculative micro-optimization.



Avoid obvious bad patterns, but profile before making performance-driven architectural changes.



Once profiling identifies a problem, optimize the measured bottleneck while preserving readability where possible.



Obvious problems such as heavy Tick work, repeated global actor searches, avoidable hot-path allocations, or accidental synchronous loads should be avoided without waiting for a profiler.



\---



\# Dependencies and Plugins



Dependencies/plugins may be added when they materially improve or are required by the implementation.



Do not add them casually.



Before adding one:



\- check whether Unreal/project code already provides the capability;

\- isolate third-party APIs behind an appropriate boundary;

\- consider offline behavior and packaging impact;

\- document non-obvious setup in the relevant existing system document.



Limit `.uproject`, `.uplugin`, `Build.cs`, target-file, and module dependency changes to what the task needs.



\---



\# Repository Hygiene



Do not edit or commit generated/transient output unless explicitly required.



Normally treat these as generated/transient:



\- `Binaries/`

\- `DerivedDataCache/`

\- `Intermediate/`

\- `Saved/`

\- `.vs/`



Do not manually edit generated headers/project files.



Do not modify binary content assets unless the task requires asset changes.



Preserve existing local file style and line endings when practical.



\---



\# Architectural Decisions



Ask before making a new project-wide architectural decision that is not already established here or in the repository.



Examples:



\- new global service pattern;

\- major system ownership change;

\- new persistence architecture;

\- new project-wide data/configuration framework;

\- networking architecture;

\- global event bus;

\- module boundary changes;

\- replacing an established subsystem;

\- a new core abstraction that many systems must depend on.



Routine implementation decisions inside an established architecture do not require permission.



\---



\# Definition of Done



A coding task is complete when:



\- requested behavior is implemented;

\- ownership and responsibilities remain clear;

\- existing compatible project patterns are respected;

\- no avoidable giant classes/functions or unnecessary coupling were introduced;

\- Blueprint exposure is intentional;

\- offline single-player remains functional unless explicitly changed;

\- future multiplayer is not needlessly blocked;

\- relevant tests were added/updated when practical;

\- affected code compiles;

\- relevant tests pass;

\- durable system docs were updated when needed;

\- unrelated files were not changed;

\- anything unverified is reported.



\---



\# Completion Report



Keep the final task report concise.



Include:



\- what changed;

\- important architectural/behavioral notes;

\- build verification;

\- tests run and result;

\- docs updated, if any;

\- anything not verified;

\- important unrelated issues noticed but intentionally left out of scope.



Do not write a long tutorial unless asked.



\---



\# References



Follow Epic's Unreal Engine 5.8 guidance unless explicitly overridden here:



\- Epic C++ Coding Standard:

&#x20; https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine

\- Write C++ Tests:

&#x20; https://dev.epicgames.com/documentation/en-us/unreal-engine/write-cplusplus-tests-in-unreal-engine

\- Automation System User Guide:

&#x20; https://dev.epicgames.com/documentation/unreal-engine/automation-system-user-guide-in-unreal-engine



