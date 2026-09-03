\# AGENTS.md — Unreal Engine C++ + Fontys Portflow Development Instructions



\## Project Context



This is an Unreal Engine game project.



The project is being developed primarily in C++ for speed, control, maintainability, and portfolio evidence.



Existing Blueprints may already contain working gameplay, data, configuration, assets, events, or editor setup.



DO NOT assume Blueprint code should be deleted.



The goal is:



\- use C++ as the main implementation layer

\- preserve useful Blueprint content

\- migrate Blueprint logic only when worthwhile

\- keep Blueprint assets working

\- expose C++ systems to Blueprint when designers/assets need access

\- preserve development evidence for the Fontys Development and Review portfolio



The actual portfolio is maintained in Portflow.



The `portfolio/` directory in this repository is a drafting/evidence workspace for Portflow.



\---



\# Prime Directive



Build the game quickly without destroying existing working systems.



Prefer:



working iteration

→ validation

→ improvement



over:



large rewrite

→ broken project

→ unclear cause



Do not rewrite functioning systems just because they are implemented in Blueprint.



When replacing Blueprint logic with C++, migrate deliberately and preserve behavior.



\---



\# Unreal Engine Project Rules



Before changing gameplay code:



1\. Inspect the relevant C++ classes.

2\. Inspect Blueprint subclasses or related Blueprint assets when their relationship is visible from the repository.

3\. Check config files.

4\. Check input mappings / Enhanced Input setup.

5\. Check GameMode, GameState, PlayerController, Character/Pawn relationships.

6\. Check existing interfaces, components, data assets, and subsystems.

7\. Understand ownership and lifecycle before creating new systems.



Do not create duplicate systems when a working Unreal system already exists.



\---



\# C++ First Architecture



New gameplay systems should normally be implemented in C++.



Use C++ for:



\- gameplay logic

\- reusable components

\- state

\- systems

\- algorithms

\- AI logic

\- inventory logic

\- interaction systems

\- combat

\- save/load

\- networking logic

\- gameplay rules

\- performance-sensitive code

\- reusable base classes

\- engine-facing architecture



Blueprint may be used for:



\- asset configuration

\- animation graphs

\- visual scripting where appropriate

\- designer tuning

\- UI binding

\- level-specific logic

\- visual/audio setup

\- simple orchestration

\- data assignment

\- Blueprint child classes



Avoid moving visual/editor-oriented work into C++ when Blueprint is the more practical Unreal workflow.



\---



\# Existing Blueprints



Existing Blueprints are part of the project and may contain important behavior.



DO NOT:



\- delete Blueprint assets without checking their role

\- rename C++ classes carelessly

\- rename reflected properties without considering Blueprint serialization

\- remove UPROPERTY fields used by Blueprint

\- change function signatures used by Blueprint without checking impact

\- remove BlueprintCallable functions without reason

\- replace working Blueprint logic blindly

\- assume a Blueprint is unused because its name is unclear



When replacing Blueprint logic:



1\. Determine what the Blueprint currently does.

2\. Identify dependencies.

3\. Reimplement the logic in C++.

4\. Expose necessary properties/functions back to Blueprint.

5\. Reparent Blueprint assets when appropriate.

6\. Preserve assigned assets and defaults.

7\. Validate behavior.

8\. Only remove obsolete Blueprint logic after the C++ replacement works.



\---



\# Blueprint / C++ Migration Rule



Do not perform large Blueprint-to-C++ migrations in one step unless explicitly requested.



Prefer incremental migration.



Example:



BP\_Player

→ create C++ APlayerCharacter base

→ move movement logic

→ keep camera/assets configured in BP child

→ validate

→ move interaction logic

→ validate

→ move combat logic

→ validate



This creates better debugging and stronger portfolio evidence.



\---



\# Reflected Unreal API



Use Unreal reflection intentionally.



Use:



UCLASS

USTRUCT

UENUM

UPROPERTY

UFUNCTION



where Unreal needs editor, serialization, garbage collection, replication, or Blueprint access.



Do not expose everything to Blueprint unnecessarily.



Prefer the smallest useful public API.



\---



\# UPROPERTY Rules



Use appropriate property specifiers.



Examples:



EditDefaultsOnly

EditInstanceOnly

VisibleAnywhere

BlueprintReadOnly

BlueprintReadWrite



Choose based on intended ownership.



Avoid making internal state editable solely for convenience.



Use categories that keep the Unreal editor organized.



Example:



UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement")

float JumpStrength = 600.0f;



\---



\# UObject Safety



Respect Unreal object ownership and garbage collection.



Do not use raw UObject pointers carelessly.



Use Unreal-supported pointer/property patterns appropriate to the engine version and existing project style.



Avoid constructing UObjects with normal `new`.



Use Unreal object creation systems such as:



NewObject

CreateDefaultSubobject

SpawnActor



as appropriate.



\---



\# Actor Components



Prefer reusable ActorComponents when functionality belongs to multiple actors.



Good candidates:



\- HealthComponent

\- InteractionComponent

\- InventoryComponent

\- CombatComponent

\- AbilityComponent

\- EquipmentComponent



Do not create components merely to split small amounts of code.



\---



\# Inheritance



Avoid deep inheritance trees.



Prefer:



base class

\+ components

\+ interfaces



when practical.



Example:



ACharacter

→ ABaseCharacter

→ APlayerCharacter



is reasonable.



Avoid:



ACharacter

→ ABaseCharacter

→ ACombatCharacter

→ AHumanoidCombatCharacter

→ APlayerCombatCharacter

→ ...



unless the project genuinely requires it.



\---



\# Interfaces



Use Unreal interfaces when multiple unrelated actors need the same capability.



Examples:



Interactable

Damageable

InventoryOwner

Targetable



Do not use interfaces when a simple component reference is sufficient.



\---



\# Gameplay Architecture



Before creating a new manager class, consider whether the responsibility belongs in:



\- Actor

\- ActorComponent

\- PlayerController

\- PlayerState

\- GameMode

\- GameState

\- GameInstance

\- GameInstanceSubsystem

\- WorldSubsystem

\- LocalPlayerSubsystem



Avoid global singleton-style systems when Unreal already provides an appropriate lifecycle system.



\---



\# Tick



Avoid Tick unless continuous updates are actually required.



Prefer:



\- events

\- delegates

\- timers

\- animation callbacks

\- overlap events

\- input events



when appropriate.



If Tick is necessary, keep it lightweight.



Do not add expensive actor searches inside Tick.



\---



\# Actor Lookups



Avoid repeatedly using broad actor searches such as:



GetAllActorsOfClass



during gameplay loops.



Prefer:



\- stored references

\- subsystems

\- interfaces

\- components

\- events

\- registration systems



One-time setup searches may be acceptable when appropriate.



\---



\# Input



Follow the existing project input architecture.



If Enhanced Input is already used, continue using it.



Keep:



\- input actions

\- mapping contexts

\- C++ bindings

\- Blueprint asset references



consistent.



Do not create a parallel input system.



\---



\# UI



Do not move all UMG implementation to C++ automatically.



Prefer C++ for:



\- UI data

\- gameplay state

\- reusable logic

\- delegates/events

\- view-model-like behavior



Prefer UMG/Blueprint for:



\- visual layout

\- animation

\- asset assignment

\- designer-facing presentation



Keep gameplay logic out of widgets whenever possible.



\---



\# Animation



Do not rewrite Animation Blueprints in C++ simply because the project is C++ focused.



Animation Blueprints remain appropriate for:



\- state machines

\- blend spaces

\- animation transitions

\- montages

\- visual animation logic



Expose necessary gameplay state from C++.



\---



\# Data



Prefer Unreal data-driven patterns where appropriate.



Possible tools:



\- Data Assets

\- Primary Data Assets

\- Data Tables

\- Curve assets

\- config properties

\- Blueprint defaults



Do not hardcode asset references unnecessarily.



\---



\# Asset References



Avoid fragile hardcoded paths when a reflected property or data asset is more appropriate.



Do not rename or move assets casually.



Unreal asset references may depend on paths.



\---



\# Performance



Do not prematurely optimize.



However, avoid obviously expensive patterns such as:



\- repeated world searches

\- heavy Tick logic

\- unnecessary allocations

\- repeated asset loading

\- unnecessary casting chains

\- large Blueprint loops every frame



When performance work is performed, preserve measurable before/after evidence when available.



Never invent performance metrics.



\---



\# Debugging



When debugging:



1\. reproduce the issue

2\. identify the likely system

3\. inspect logs/errors

4\. isolate the cause

5\. make the smallest useful fix

6\. validate the fix

7\. preserve meaningful debugging evidence if portfolio-worthy



Use UE\_LOG where appropriate.



Do not leave excessive temporary logging in production code.



\---



\# Build Safety



Before considering C++ work complete:



1\. compile the relevant target

2\. address errors introduced by the change

3\. inspect Unreal Header Tool errors carefully

4\. consider Blueprint compatibility

5\. validate affected gameplay when possible



Do not "fix" compilation by disabling unrelated systems.



Do not silently remove reflection macros merely to make code compile.



\---



\# Compilation Strategy



Prefer smaller compile cycles.



Do not modify dozens of gameplay classes at once unless required.



For large changes:



1\. create base structure

2\. compile

3\. implement core behavior

4\. compile

5\. integrate

6\. validate



This reduces Unreal iteration time and makes failures easier to identify.



\---



\# Header Hygiene



Keep Unreal headers clean.



Prefer forward declarations where appropriate.



Avoid unnecessary includes.



Do not fight Unreal Header Tool requirements.



Respect generated header placement.



`\*.generated.h` must remain the final include in Unreal reflected headers where required.



\---



\# Naming



Follow Unreal naming conventions already present in the project.



Typical conventions:



A = Actor

U = UObject

F = Struct

E = Enum

I = Interface



Examples:



APlayerCharacter

UHealthComponent

FWeaponData

EWeaponState

IInteractable



Do not rename existing public classes merely to enforce naming style unless necessary.



\---



\# Source Structure



Respect the existing module layout.



When new systems become substantial, organize by feature.



Example:



Source/GameName/

├── Characters/

├── Components/

├── Combat/

├── Interaction/

├── Inventory/

├── AI/

├── UI/

└── Core/



Do not reorganize the entire codebase during unrelated tasks.



\---



\# Vibe Coding Workflow



The user may make fast informal requests such as:



"make jumping feel better"



"redo the enemy AI"



"move this blueprint to C++"



"make combat less janky"



"add interaction"



"this menu is broken"



Interpret these as implementation requests.



Do not demand a formal specification unless necessary.



First inspect the current implementation and make a reasonable implementation based on project context.



For meaningful tasks:



1\. understand the current system

2\. determine the problem

3\. identify the smallest useful implementation

4\. preserve existing behavior

5\. implement

6\. compile

7\. validate

8\. document meaningful evidence

9\. identify the next logical step



\---



\# Do Not Overengineer



The goal is to finish the project efficiently.



Do not introduce:



\- unnecessary frameworks

\- custom dependency injection

\- elaborate abstraction layers

\- complicated messaging systems

\- premature plugin architecture

\- unnecessary templates

\- unnecessary generic systems



Prefer straightforward Unreal patterns.



A simple working solution is better than an elegant unfinished architecture.



\---



\# Portfolio Purpose



This project contributes evidence to a Fontys ICT Development and Review portfolio.



The portfolio must demonstrate:



\- effort

\- development

\- results

\- reflection

\- validated products

\- feedback

\- improvement over time



It is NOT only a showcase of successful final work.



Preserve failed attempts when they demonstrate useful development.



\---



\# Portfolio Workspace



Maintain when relevant:



portfolio/

├── READING\_GUIDE.md

├── PORTFLOW\_QUEUE.md

├── DEVLOGS/

├── PRODUCTS/

├── REFLECTIONS/

├── LEARNING\_OUTCOMES/

└── REVIEWS/



Learning outcome files:



portfolio/LEARNING\_OUTCOMES/

├── analysis.md

├── advice.md

├── design.md

├── realization.md

├── manage-and-control.md

├── professional-standard.md

└── personal-leadership.md



Do not create meaningless empty documents.



\---



\# Devlog Rule



Create a devlog for meaningful:



\- C++ gameplay systems

\- Blueprint-to-C++ migrations

\- prototypes

\- architecture changes

\- gameplay iterations

\- important bugs

\- AI changes

\- combat changes

\- player movement changes

\- testing

\- optimization

\- design experiments

\- technical research

\- playtests

\- feedback-driven changes



Do not create a devlog for tiny cleanup.



\---



\# Devlog Template



Every meaningful devlog should use:



\# <Title>



\## What is the problem?



Explain:



\- the challenge

\- why it matters

\- what desired result or player experience is intended



When appropriate, use:



Develop a <solution> to enable <users> in <context> to <perform activity> with <target performance>.



If relevant, include an MDA aesthetic.



\---



\## How did I tackle the problem?



Describe:



\- investigation

\- research

\- Unreal systems considered

\- prototypes

\- C++ implementation

\- Blueprint implementation or migration

\- alternatives

\- testing

\- iterations



If formal research was actually performed, identify the research method.



Do not invent research retroactively.



\---



\## What is the result?



Show actual evidence.



For Unreal work, good evidence includes:



\- gameplay video

\- GIF

\- screenshot

\- before/after

\- C++ commit

\- branch

\- small code snippet

\- Blueprint screenshot

\- architecture diagram

\- Unreal Insights capture

\- profiler result

\- playable build



For Blueprint-to-C++ migration, preserve evidence of:



Before:

Blueprint implementation



After:

C++ implementation + Blueprint child/configuration



Explain what moved and what remained in Blueprint.



\---



\## What is the quality of the result?



Evaluate against the original problem.



Possible evidence:



\- gameplay test

\- playtest

\- expert feedback

\- lecturer feedback

\- technical validation

\- FPS/profile data

\- compile/build validation

\- behavior comparison

\- bug reproduction tests



Do not claim quality without evidence.



If not validated yet, write:



TODO: validation needed



\---



\## What is my advice?



Explain:



\- what was learned

\- what recommendation follows

\- what should remain as-is

\- what should be changed next

\- whether the solution is reusable

\- what the next iteration should address



Advice must follow from actual results.



\---



\# Blueprint-to-C++ Portfolio Evidence



A Blueprint-to-C++ migration may support multiple learning outcomes.



Possible mapping:



Analysis:

\- investigated current Blueprint behavior

\- identified dependencies

\- identified migration risk



Design:

\- planned C++ class/component structure



Realization:

\- implemented the C++ replacement



Manage \& Control:

\- migrated incrementally

\- compiled/tested after each stage

\- preserved version control history



Advice:

\- concluded which responsibilities should remain Blueprint vs C++



Professional Standard:

\- documented the migration clearly

\- followed Unreal conventions



Do not automatically assign every LO.



Only use those genuinely demonstrated.



\---



\# Git Evidence



Git is important portfolio evidence.



For meaningful work, reference:



\- branch

\- commit

\- pull request



when available.



Prefer descriptive commits.



Examples:



feat: add C++ interaction component



refactor: migrate player health from BP to C++



fix: prevent duplicate enemy aggro registration



test: add combat validation scenario



Do not fabricate commit hashes.



\---



\# C++ Evidence



For code evidence, prefer linking to Git.



If a code snippet is useful, keep it small and focused.



Explain:



\- what problem it solves

\- why this implementation exists

\- what Unreal concept it demonstrates



Do not paste entire classes into Portflow unless necessary.



\---



\# Blueprint Evidence



Blueprints are valid portfolio evidence.



Do not treat Blueprint as inferior evidence.



Useful Blueprint evidence includes:



\- visual state machines

\- animation graphs

\- UI flows

\- Blueprint child configuration

\- event graphs

\- before/after migration screenshots



If Blueprint remains the better tool for part of the system, document that decision.



\---



\# Architecture Evidence



For significant architecture work, create diagrams when useful.



Examples:



Player

→ InteractionComponent

→ IInteractable

→ World Object



or C4-style diagrams when appropriate.



Do not create architecture diagrams for trivial systems.



\---



\# Research



Technical research may include:



\- Unreal documentation

\- engine architecture comparison

\- gameplay framework investigation

\- profiling

\- comparing approaches

\- prototype comparison



Research must lead to:



question

→ findings

→ conclusion

→ actionable point



Do not create fake research merely to justify a decision.



\---



\# Failed Attempts



Preserve meaningful failures.



Example:



Attempt 1:

Implemented interaction in PlayerCharacter.



Problem:

Character class became responsible for too many systems.



Attempt 2:

Moved interaction into UInteractionComponent.



Result:

System became reusable across player and AI-controlled pawns.



This is useful development evidence.



Do not erase failed attempts from the portfolio narrative.



\---



\# Group Work



Group products are valid evidence.



When documenting group work, explicitly state:



My contribution:

...



Examples:



\- implemented C++ enemy state logic

\- created interaction component

\- integrated teammate's animation Blueprint

\- reviewed gameplay architecture

\- fixed replication issue

\- implemented save system



Never imply ownership of the entire group product unless true.



\---



\# Reflection



Do not fabricate personal reflection.



Create reflection drafts only from information the user actually provides.



Possible subjects:



\- learning C++

\- becoming faster in Unreal

\- moving away from Blueprint-heavy architecture

\- planning

\- feedback

\- collaboration

\- technical confidence

\- professional development

\- internship preparation



Reflections belong mainly to Personal Leadership.



\---



\# Reading Guide



Maintain:



portfolio/READING\_GUIDE.md



The reading guide should:



\- briefly describe the project

\- explain development chronologically

\- connect devlogs and products

\- show cause and effect

\- point to evidence



Use:



this led to...

because of this...

testing showed...

therefore...

the next iteration...



Do not make it a simple evidence list.



\---



\# PORTFLOW\_QUEUE.md



Maintain:



portfolio/PORTFLOW\_QUEUE.md



Use this as the queue of evidence that should eventually be entered into Portflow.



Format:



\## <Evidence title>



Type:

Devlog / Product / Reflection / Reading Guide Update / Feedback



Suggested Portflow location:

...



Learning Outcomes:

...



Evidence:

...



Status:

Draft / Ready for Portflow / Added to Portflow



Notes:

...



Never mark:



Added to Portflow



unless the user explicitly confirms it.



\---



\# Portflow Mapping



Use this general rule:



Collection Description

→ Reading Guide



Professional Duties:

\- Analysis

\- Advice

\- Design

\- Realization

\- Manage \& Control

→ professional products/evidence



Professional Standard

→ devlogs showing structured research and development



Personal Leadership

→ reflection documents



Do not mechanically attach everything everywhere.



\---



\# Portfolio Reviews



When the user says a review is approaching, prepare a review document grouped by learning outcome.



Include:



\- new evidence

\- strongest evidence

\- what changed

\- validation

\- feedback received

\- next development target



Mark new portfolio evidence with:



\*NEW\*



when useful.



\---



\# Accuracy



NEVER invent:



\- playtests

\- player feedback

\- lecturer feedback

\- teammate feedback

\- external stakeholder feedback

\- research

\- research methods

\- measurements

\- profiling numbers

\- FPS gains

\- commit hashes

\- branches

\- Blueprint behavior not inspected

\- C++ behavior not implemented

\- personal reflections

\- group contributions



If evidence is missing, use:



TODO: evidence needed



TODO: validation needed



TODO: ask user for reflection



\---



\# Completion Behavior



After a meaningful task, report briefly:



1\. what was changed

2\. whether C++ or Blueprint was used and why

3\. what was compiled/tested

4\. what portfolio evidence was updated

5\. what the logical next step is



Example:



Implemented the interaction system as a reusable C++ ActorComponent and kept prompt presentation in the existing Widget Blueprint. The project compiles and interaction was validated with the current door Blueprint. Added Devlog 004 and queued the commit as Realization + Design evidence. Next step is testing the component with a second interactable type.



Keep completion reports concise.



\---



\# Final Rule



Use C++ aggressively where it improves the project.



Do not use C++ dogmatically.



Keep good Blueprints.



Migrate bad or overly complex Blueprint logic incrementally.



Build first.



Validate.



Preserve evidence.



Keep the portfolio synchronized with meaningful development.

