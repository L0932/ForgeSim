# ForgeSim Architecture

## 1. Purpose and Status

ForgeSim is a systems-oriented real-time engine and editor for mechanically interactive games and professional simulation applications. It is written primarily in modern C++ and is intended to support interactive tools, machinery, mechanical assemblies, desktop and VR interaction, and data-driven simulation workflows.

This document defines the project's architectural direction, subsystem responsibilities, dependency rules, and cross-cutting constraints. `Roadmap.md` defines delivery order and scope. Architecture decision records document important decisions and their alternatives.

The architecture is intentionally incremental. Conceptual subsystem boundaries described here do not require an immediate one-to-one mapping to CMake targets. A target should be introduced when code has a concrete responsibility, a meaningful dependency boundary, or an independent testing or reuse requirement.

### Decision Status

Statements using **must**, **must not**, or **will** are current architectural constraints. Statements marked **provisional** require validation through implementation or an architecture decision record.

---

## 2. Product Scope

ForgeSim provides a shared technical foundation for two related categories of applications.

### 2.1 Games and Interactive Experiences

- Mechanic and repair simulations
- Physics-supported construction and assembly
- Interactive machines and vehicles
- Functional logic, power, fluid, and control systems
- Larger systemic and automation-oriented experiences over time

### 2.2 Professional Simulation and Training

- Equipment and maintenance training
- Procedural instruction and assessment
- Fault diagnosis and operational workflows
- VR-based interaction with tools and machinery
- Telemetry, replay, validation, and instructor-facing tools

The shared engine owns world representation, rendering, physics integration, interaction, mechanical state, assets, persistence, and general authoring tools. Game rules and training rules are application-level concerns built above that shared foundation.

### 2.3 Initial Product Boundary

The initial product target is a compact mechanic-simulator vertical slice. It will validate inspection, diagnosis, tool use, mechanical disassembly, part replacement, reassembly, functional testing, editor support, persistence, and eventually VR interaction.

ForgeSim is not intended to compete feature-for-feature with mature general-purpose engines. The initial architecture must not be distorted by requirements for every genre, every content pipeline, or hypothetical future backends.

---

## 3. Architectural Principles

### 3.1 Clear Dependency Direction

Higher-level policy may depend on lower-level mechanisms. Lower-level engine systems must not depend on editor UI, project-specific gameplay, or training procedures.

### 3.2 Runtime and Editor Separation

Runtime functionality must remain usable without Dear ImGui or the editor executable. The editor is a client of engine APIs and domain operations.

### 3.3 Platform Isolation

Platform-specific behavior and third-party platform callbacks must remain outside general engine and simulation code.

### 3.4 Explicit Ownership

Resources and subsystem lifetimes must have identifiable owners. RAII and value semantics should be preferred where appropriate. Shared ownership must not be used merely to avoid lifecycle design.

### 3.5 Hybrid Physical and Logical Modeling

Rigid-body physics will model motion, collision, forces, and constraints. Durable mechanical truth—such as installation, fastening, compatibility, condition, connectivity, and operational state—will be represented explicitly rather than inferred solely from instantaneous physics state.

### 3.6 Device-Independent Interaction

Interaction intent must not be defined in terms of a specific device. Desktop input, conventional controllers, and VR hands should be capable of driving the same interaction and mechanical-domain operations.

### 3.7 Testable Simulation Logic

Logic that does not inherently require graphics, native windows, or tracked hardware must be testable without them.

### 3.8 Incremental Abstraction

Abstractions should arise from concrete requirements. ForgeSim should encapsulate current implementation details without pretending to support unimplemented backends or extension models.

### 3.9 Performance by Measurement

Subsystems should expose useful measurements and debugging state. Data-oriented or multithreaded designs should be introduced when profiling demonstrates a need.

### 3.10 Stable Authored Data

Persistent content must use stable identities and versioned schemas. Runtime addresses, container positions, and editor-only state must not serve as persistent identity.

---

## 4. System Context

ForgeSim is composed through executable-level composition roots rather than a single monolithic engine object with implicit ownership of every subsystem.

```text
Project or Demonstration Code
            |
            v
   Runtime / World / Domain Systems
      |        |         |
      v        v         v
 Rendering  Physics  Persistence
      |        |         |
      +--------+---------+
               |
               v
        Core and Platform

Editor -----------------> Public engine and domain APIs
VR integration ---------> Input, interaction, and rendering boundaries
Training layer ---------> Mechanical state and observed domain events
```

The arrows indicate permitted high-level dependency direction, not ownership in every case.

---

## 5. Executables and Composition Roots

### 5.1 Sandbox

The Sandbox is the initial development executable. It composes implemented subsystems, hosts focused demonstrations, and provides a controlled environment for incremental development.

The Sandbox must not become the permanent owner of reusable engine behavior. Functionality that belongs to the engine or a project should migrate to the appropriate module once its responsibility becomes clear.

### 5.2 Editor

The editor executable will compose the runtime, renderer, project content, and Dear ImGui tools. It may own editor session state, selection, windows, command history, and authoring workflows.

The editor must not own essential world, interaction, mechanical, scenario, or persistence rules.

### 5.3 Project Runtime

A standalone project runtime may be introduced when the first vertical slice must run independently of the editor. It will compose engine modules and project-specific code without Dear ImGui dependencies.

### 5.4 Headless Validation

A headless executable or test harness will compose supported world, mechanical, network, scenario, and validation systems without GLFW, OpenGL, Dear ImGui, or VR dependencies.

Headless compatibility is an architectural constraint from the beginning even though the executable is delivered later in the roadmap.

---

## 6. Proposed Module Boundaries

The names below describe architectural roles. Except for already established targets, names and target granularity remain provisional until implementation justifies them.

| Conceptual module | Primary responsibility | Must not depend on |
|---|---|---|
| `ForgeSim::Core` | Narrow foundational utilities, diagnostics, identifiers, and common infrastructure | GLFW, OpenGL, physics SDK, VR SDK, Dear ImGui, project logic |
| Platform abstraction, if justified | Platform-neutral window, event, and input boundaries | Renderer, world, mechanics, editor |
| `ForgeSim::PlatformGLFW` | GLFW-backed windowing, native events, input collection, and OpenGL context support | World, mechanics, scenarios, editor |
| Runtime | Application coordination, clocks, update scheduling, and lifecycle contracts | Dear ImGui and project-specific policy |
| World/Scene | Object identity, components, transforms, hierarchy, lifecycle, and system access | Editor UI, GLFW, VR SDK, training policy |
| Renderer | Render resources, cameras, submission, OpenGL implementation, and render diagnostics | Editor UI, mechanics policy, scenarios |
| Physics integration | Physics-world ownership, bodies, shapes, queries, constraints, and synchronization | Editor UI, gameplay rules, training rules |
| Interaction | Device-independent intents, sources, affordances, targeting, grabbing, and tool-use coordination | Dear ImGui and project-specific progression |
| Mechanics | Parts, assemblies, connections, condition, compatibility, validated operations, and functional state | Editor UI, game progression, training procedures |
| Assets/Persistence | Asset identity, loading boundaries, schemas, serialization, and reconstruction | Dear ImGui and project-specific UI |
| Editor | Authoring UI, selection, command history, inspectors, viewports, and debug panels | — |
| Functional networks | Connected-system graphs and domain-specific propagation | Editor UI, game progression, training assessment |
| Scenario/Training | Objectives, procedures, faults, observations, telemetry, and assessment | Editor UI and rendering implementation details |
| VR integration | Headset/controller tracking, hand input adaptation, stereoscopic presentation integration, and haptics | Mechanical policy and training procedure logic |
| Project code | Content-specific behavior, game rules, progression, and demonstration logic | Editor internals unless implementing editor extensions |

No module should be created solely to match this table. The dependency rule is more important than the number of targets.

---

## 7. Core

`ForgeSim::Core` should remain small and stable.

Expected responsibilities include:

- Assertions and diagnostic contracts
- Logging interfaces and common log categories
- Stable identifier primitives
- Basic timing types that do not own application policy
- Small utilities with broad, demonstrated use

Core must not become a miscellaneous dumping ground. Mathematics, events, resource management, reflection, or service-location facilities should not automatically be placed in Core; each requires a concrete use and ownership model.

---

## 8. Platform and Input

The platform layer owns native window and input integration. The GLFW implementation currently creates the window and OpenGL context and receives platform callbacks.

Platform responsibilities include:

- Window creation and destruction
- Native event pumping
- Framebuffer and window size reporting
- Raw keyboard, pointer, and controller state collection
- Graphics-context creation required by the current renderer

Platform callbacks must be translated into ForgeSim-owned data before reaching higher-level systems. Higher-level code must not require GLFW types in its public interfaces.

Raw input is not gameplay or interaction intent. A separate action layer will map device state to named actions, and the interaction system will interpret those actions in world context.

Whether a standalone platform-neutral target is required is provisional. The project should not introduce an abstract window interface until at least one consuming boundary requires it.

---

## 9. Runtime and Time Domains

The runtime coordinates subsystem lifecycle and update order. It does not own all subsystem implementation details.

### 9.1 Time Domains

ForgeSim will distinguish at least:

- **Real time:** monotonic elapsed time used for frame measurement and external timing
- **Simulation time:** scaled, pausable time advanced in fixed increments
- **Presentation time:** frame-driven rendering and visual interpolation where used
- **Editor time:** editor UI updates that may continue while simulation is paused

Systems must receive the time information they require rather than querying arbitrary global clocks.

### 9.2 Initial Frame Sequence

```text
Pump platform events
Collect input state
Calculate real frame duration
Accumulate simulation time
Run zero or more fixed simulation steps
Prepare presentation state
Render
Update editor UI when present
Present the frame
```

The exact placement of editor UI rendering may follow the renderer integration, but editor activity must not advance simulation implicitly.

### 9.3 Fixed-Step Contract

The runtime will:

- Use a configured fixed simulation interval
- Permit zero or more simulation steps per rendered frame
- Bound catch-up work to avoid an uncontrolled spiral of death
- Support pause, resume, single-step, and time scaling
- Expose dropped or clamped time through diagnostics

Exact interval, catch-up limit, interpolation policy, and determinism guarantees require an ADR.

---

## 10. World and Scene

The world is the authoritative container for active objects and systems. A scene is the persistent authored description used to construct or restore relevant world state.

For the initial implementation, ForgeSim should avoid maintaining two complete parallel object models. The runtime world and serialized scene may use different representations at their boundaries, but they should describe the same object identities and authored properties.

### 10.1 World Objects

World objects require:

- Stable identity
- Optional human-readable name
- Lifecycle state
- Components or equivalent composable data
- Optional transform and hierarchy membership

The exact entity/component implementation is provisional. A full archetype ECS is not assumed. The first model should prioritize understandable ownership, safe mutation, testing, serialization, and sufficient performance for the vertical slice.

### 10.2 Hierarchy

Transform hierarchy represents spatial parent/child relationships. It must not automatically imply ownership, simulation activation, mechanical connection, or serialization scope.

Mechanical assemblies may use hierarchy for presentation, but their connection graph is a distinct domain concept.

### 10.3 Stable Identity

Persistent references must use stable identifiers. Runtime handles may include generation or validity information to detect stale access. Human-readable names are not unique identity.

---

## 11. Rendering

The initial renderer uses OpenGL. It exists to visualize interactive worlds and demonstrate graphics-engineering techniques without attempting to become a general-purpose AAA renderer.

### 11.1 Boundary

OpenGL calls and types must remain within the renderer implementation and platform context setup. World, mechanics, scenarios, and editor-independent project logic must not issue OpenGL calls.

### 11.2 Initial Responsibilities

- Graphics-resource ownership
- Buffers and vertex input
- Shaders and shader diagnostics
- Textures, meshes, and basic materials
- Cameras and view/projection data
- Render submission
- Debug rendering
- Frame and render statistics

### 11.3 Abstraction Depth

The renderer will initially provide narrow RAII wrappers and engine-facing rendering concepts. It will not pretend to be backend-independent before a concrete second backend exists.

World components should describe renderable intent or reference render assets. Renderer-owned GPU objects must not be serialized directly.

Render interpolation between simulation states is optional until motion quality requires it. If introduced, presentation state must not become authoritative simulation state.

---

## 12. Physics Integration

Physics is a foundational service for ForgeSim's interaction and mechanical goals.

### 12.1 Responsibilities

- Physics-world creation and lifetime
- Rigid bodies and collision shapes
- Collision filtering
- Queries and contacts
- Fixed, hinge, slider, and other justified constraints
- Sleeping and activation
- Debug visualization and statistics
- Explicit synchronization with world state

### 12.2 Update Ownership

Physics advances within the fixed simulation step. The runtime owns scheduling; the physics integration owns the physics world and SDK resources.

### 12.3 Synchronization

Transform authority must be explicit:

- Static objects are authored by the world and represented in physics.
- Dynamic objects are normally authoritative in physics during simulation.
- Kinematic objects are driven by explicit world or interaction targets.
- Editor manipulation while paused requires a defined synchronization path.

### 12.4 Physics Is Not Mechanical Truth

Contact or proximity alone does not mean a part is installed. A constraint alone does not define compatibility, fastening, condition, or function. The Mechanics subsystem records those durable facts and creates or removes physical constraints through the physics boundary as needed.

The physics library choice and wrapper depth require an ADR.

---

## 13. Interaction

Interaction translates user intent into validated requests against world and domain systems.

### 13.1 Interaction Model

The model distinguishes:

- **Source:** desktop pointer, player controller, tracked hand, or automated test driver
- **Intent:** select, grab, release, use, push, pull, rotate, attach, or remove
- **Target:** world object or interaction point
- **Affordance:** operations the target supports under current conditions
- **Execution:** validated world, physics, or mechanical-domain operation
- **Feedback:** visual, audio, animation, or haptic response

### 13.2 Tools

A tool exposes capabilities rather than hard-coding knowledge of every compatible object. Targets expose affordances and requirements. Mechanical domain rules determine whether an operation is valid and what state transition it causes.

### 13.3 Desktop and VR

Desktop interaction is the initial development and accessibility harness. VR adapts tracked hands and controllers into the same interaction intents and consumes the same affordances and domain operations.

VR-specific hand posing, reach, haptics, and two-handed constraints belong to VR or presentation integration, not the Mechanics subsystem.

---

## 14. Mechanical Domain

The Mechanics subsystem is the primary ForgeSim-specific domain layer shared by game and professional simulation projects.

### 14.1 Definitions and Instances

- A **part definition** describes reusable authored properties, compatible connections, supported operations, assets, and default state.
- A **part instance** has stable identity and records its current condition, installation state, connections, and project-specific overrides.
- An **assembly** groups related part instances and exposes validation or functional state.
- A **subassembly** is an assembly used as part of a larger assembly without losing its internal structure.

### 14.2 Connections

A mechanical connection describes a durable relationship between compatible attachment points. It may create a physics constraint, a functional-network edge, a transform relationship, or some combination of these.

Connection type, compatibility, installation requirements, and removal requirements are domain data. They must not be inferred only from hierarchy or physical overlap.

### 14.3 Condition and Faults

Parts may expose condition, wear, damage, calibration, or fault state as justified by the project. Fault injection must use the same validated domain APIs as other state changes rather than modifying private component state directly.

### 14.4 Domain Operations

Examples include:

- Install or remove a part
- Attach or disconnect a connection
- Tighten or loosen a fastener
- Repair or replace a component
- Change an operational control

Domain operations validate preconditions, modify authoritative state, produce observable results, and coordinate required physics or network changes.

Domain operations are not inherently editor commands and are not automatically undoable.

### 14.5 Functional State

An assembly's functional state may be derived from part presence, compatibility, condition, connection state, functional networks, and current operating inputs. Expensive evaluation should become incremental only after profiling identifies a need.

The initial part/component representation, attachment model, and operation-result contract require ADRs.

---

## 15. Functional Networks

Functional networks represent connected systems such as electrical power, mechanical power, fluids, fuel, coolant, signals, or control logic.

Each network domain may define its own node, port, edge, state, and propagation rules while using shared graph infrastructure only where concrete commonality exists.

Networks must:

- Reference stable part or port identities
- Update independently of editor UI
- Expose diagnostic topology and state
- Serialize authored connections
- Respond to mechanical installation, removal, condition, and fault changes
- Avoid requiring rigid-body physics to represent nonphysical flow or logic

ForgeSim will implement one network domain before generalizing the infrastructure. A universal network framework must not be designed solely from hypothetical requirements.

---

## 16. Assets, Projects, and Persistence

### 16.1 Projects

A project identifies content, configuration, scenes, part definitions, and project-specific code or data. Editor session preferences are not automatically part of runtime project content.

### 16.2 Asset Identity

Assets use stable identifiers or handles. Filesystem paths may locate source content but should not be the only durable identity once asset references and moves must be supported.

### 16.3 Source Data and Runtime Resources

ForgeSim distinguishes:

- Source assets and authored documents
- Serialized project and scene data
- Imported or processed runtime data, if later required
- Live runtime resources such as GPU objects and physics shapes

Live runtime resources are reconstructed and are not serialized directly.

### 16.4 Serialization

Persistent formats must support:

- Schema versioning
- Stable object and asset references
- World hierarchy and authored components
- Part definitions and instances
- Assemblies and connections
- Functional-network definitions
- Scenario and training definitions when introduced

Serialization should be human-readable during early development where doing so improves debugging and version control. Format and library selection require an ADR.

Deserialization must validate input and report actionable errors. Round-trip tests must verify reconstruction of representative content.

---

## 17. Editor and Commands

### 17.1 Editor Role

The editor owns:

- Dear ImGui integration
- Docking and window layout
- Selection and editor session state
- Scene viewport and hierarchy presentation
- Property and domain inspectors
- Debug panels and visualization controls
- Authoring workflows
- Editor command history

The editor invokes public runtime and domain APIs. It must not contain the only implementation of world mutation, assembly validation, persistence, or scenario execution.

### 17.2 Editor Commands

Editor commands represent reversible authoring operations. Examples include:

- Create or delete an object
- Modify an authored property
- Change a transform
- Reparent an object
- Author or remove a mechanical connection

A command records enough information to execute and reverse its authoring change. Dear ImGui types must not appear in command interfaces.

### 17.3 Domain Operations Versus Commands

Editor commands may call domain operations, but the concepts remain separate:

- **Input actions** express user intent.
- **Domain operations** validate and mutate runtime or authored domain state.
- **Editor commands** provide reversible authoring history.

Runtime gameplay must not depend on editor command history.

### 17.4 In-World Construction

Player construction is runtime behavior, not reuse of the Dear ImGui editor. It may share snapping, validation, part definitions, attachment rules, and domain operations with editor authoring while providing its own interaction and presentation.

---

## 18. Scenario and Training Layer

Scenario and training systems are application-level orchestration built above shared world and mechanical state.

### 18.1 Scenario Runtime

A scenario may define:

- Initial state
- Events
- Conditions
- Actions
- Objectives
- Outcomes
- Faults
- State transitions

Scenario execution must not depend on its visual editor.

### 18.2 Procedures and Assessment

A procedure observes domain operations and relevant state transitions. It may evaluate required steps, ordering, allowed alternatives, timing, safety constraints, and final state.

Procedure state must not be embedded into general mechanical components. The same assembly should remain usable without training logic.

### 18.3 Telemetry

Domain operations and significant state changes should produce structured observations suitable for diagnostics, assessment, record/replay, or networking. This does not imply one unrestricted global event bus.

Event ownership, ordering, lifetime, and delivery semantics must be explicit. The observation/event model requires an ADR before it becomes a broad public API.

---

## 19. VR Integration

VR is an integration layer over platform tracking, input actions, interaction, rendering, animation, and feedback.

Responsibilities include:

- Headset and controller tracking
- Tracked-hand or controller pose adaptation
- Stereoscopic view configuration
- Hand representation and pose selection
- Stable one-handed and two-handed grasp behavior
- Haptic feedback where supported
- VR diagnostics and performance measurement

VR SDK types must not enter Mechanics, Scenario, or general World public interfaces. The VR runtime and dependency strategy require an ADR.

---

## 20. Game and Training Separation

ForgeSim distinguishes three kinds of state and policy:

| Concern | Example | Owner |
|---|---|---|
| Mechanical truth | A part is installed, damaged, connected, or transmitting power | Mechanics and functional networks |
| Game policy | The repair awards currency or unlocks an upgrade | Project/game code |
| Training policy | The part was removed in an allowed sequence and the trainee passed | Scenario/training layer |

Game or training systems may observe and request changes through public APIs. They must not redefine core mechanical truth privately.

---

## 21. Headless Execution

Headless execution must be possible for systems that do not inherently require presentation or tracked hardware.

The headless composition must be capable of supporting, as implemented:

- World and scene loading
- Fixed-step updates
- Mechanical-domain operations
- Functional-network evaluation
- Scenario execution
- Content and workflow validation
- Machine-readable test results

Rendering, GLFW, Dear ImGui, audio, and VR must remain optional dependencies from the perspective of these systems.

Physics may be available in headless mode if the selected physics integration supports it. Tests that depend on physics must document tolerances and platform limitations rather than claiming stronger determinism than the implementation provides.

---

## 22. Determinism and Replay

ForgeSim favors repeatable and testable behavior, but full cross-platform bitwise determinism is not assumed.

The initial determinism contract is:

- Fixed-step systems receive explicit time steps.
- Simulation logic must not depend on render frame rate.
- Random behavior must be seedable when repeatability is required.
- Iteration order that affects outcomes must be intentional.
- Wall-clock and editor state must not implicitly affect simulation results.
- Physics-dependent tests must use documented tolerances.

Record/replay may record input, domain operations, observations, state snapshots, or a combination. The required fidelity and authority model must be decided before implementation.

---

## 23. Ownership and Lifetime

### 23.1 General Rules

- Executable composition roots own top-level subsystem instances.
- Subsystems own their implementation resources.
- World objects use handles or stable IDs rather than exposing owning pointers broadly.
- GPU resources are owned by the renderer.
- Physics resources are owned by the physics integration.
- Native window and context resources are owned by the platform implementation.
- Editor widgets do not own runtime objects merely because they display them.

### 23.2 Initialization and Shutdown

Dependencies must be initialized before their consumers and destroyed after them. Shutdown must not depend on global-static destruction order.

### 23.3 Global Access

Global mutable service locators should be avoided. Systems should receive required dependencies explicitly through construction, initialization, or scoped execution context.

---

## 24. Errors, Diagnostics, and Observability

Expected failures such as invalid content, missing assets, incompatible connections, or unavailable devices should produce structured errors or operation results rather than assertions alone.

Assertions are appropriate for violated programmer invariants. Recoverable runtime and authoring failures must remain diagnosable in Release configurations.

Each major subsystem should expose suitable diagnostics:

- Runtime timing and catch-up behavior
- Renderer resource and frame statistics
- Physics bodies, shapes, contacts, and constraints
- Interaction source, target, and affordance state
- Mechanical connections, condition, and validation failures
- Functional-network topology and propagation state
- Scenario and procedure state

Diagnostic presentation belongs to the editor or debug-rendering layer; diagnostic data belongs to the relevant subsystem.

---

## 25. Testing Strategy

Testing is continuous rather than a late roadmap phase.

### 25.1 Unit and Domain Tests

- Stable identifier and handle behavior
- Transform and hierarchy operations
- Clock and fixed-step accumulation
- Mechanical compatibility and operation validation
- Assembly completeness and functional-state evaluation
- Functional-network propagation
- Scenario conditions, actions, and assessment

### 25.2 Serialization Tests

- Round-trip reconstruction
- Stable reference resolution
- Invalid and missing data handling
- Schema migration once more than one schema version exists

### 25.3 Integration Tests

- World and physics lifecycle
- Constraint creation and destruction
- Interaction-to-domain-operation flow
- Tool use and mechanical state transitions
- Save, reload, and continue workflows
- Headless scenario validation

### 25.4 Manual and Visual Verification

Rendering, VR comfort, hand behavior, tool usability, and presentation quality require manual verification in addition to automated tests.

---

## 26. Dependency Strategy

External dependencies should be adopted deliberately when they provide mature infrastructure that is expensive or risky to reproduce.

Current or expected categories include:

- GLFW for initial windowing and input integration
- An OpenGL function loader
- Dear ImGui for editor UI
- A mathematics library if justified
- A physics library
- Image and model loading libraries
- A serialization library if justified
- A VR runtime integration layer
- A testing framework

Each major dependency must be evaluated for:

- License and redistribution terms
- Maintenance activity and platform support
- API stability
- Build-system impact
- Transitive dependencies
- Debugging and profiling support
- Ability to isolate it behind a ForgeSim-owned boundary
- Long-term replacement cost

Major dependency selections should be captured in ADRs.

---

## 27. CMake and Build Architecture

ForgeSim-owned targets use C++20 and should expose namespaced aliases such as `ForgeSim::Core` and `ForgeSim::PlatformGLFW` where appropriate.

Build rules include:

- Target-scoped sources, includes, compile features, definitions, and dependencies
- No reliance on global include directories or compiler flags
- Public dependencies only when they are part of a target's public interface
- Private third-party dependencies where implementation boundaries permit
- Platform-specific source selection inside platform implementation targets
- Tests that link only the modules required by the behavior under test
- Runtime modules that can build without editor dependencies
- Headless-compatible modules that can build without presentation dependencies

The directory structure may evolve, but target dependency direction is authoritative over folder appearance.

---

## 28. Provisional Dependency Direction

The following is the intended dependency direction. It should be enforced through CMake target links as modules become concrete.

```text
Core

PlatformGLFW ------> Core
Runtime -----------> Core
World -------------> Core
Renderer ----------> Core
Physics -----------> Core, World-facing contracts
Interaction -------> Core, World, Physics-facing contracts
Mechanics ---------> Core, World, Physics-facing contracts
Persistence -------> Core, World, Mechanics data contracts
FunctionalNetworks -> Core, World/Mechanics identities
ScenarioTraining --> Core, World, Mechanics, FunctionalNetworks
VRIntegration -----> Core, Runtime-facing input, Interaction, Renderer-facing contracts
Editor ------------> Runtime, World, Renderer, Physics, Interaction,
                     Mechanics, Persistence, FunctionalNetworks, ScenarioTraining
ProjectCode -------> Public runtime and domain APIs
Executables -------> Required implementations and project code
```

This diagram does not permit upward dependencies merely because a lower module appears on the same line as a higher module. Cycles between targets are prohibited. If two modules appear to require each other, their contracts, ownership, or target boundaries must be reconsidered.

---

## 29. Explicit Architectural Non-Goals

- A monolithic engine library containing every subsystem
- A universal graphics abstraction designed without a second backend
- A universal functional-network framework designed before one concrete domain works
- An editor dependency in runtime or mechanical logic
- Mechanical installation inferred only from collision or hierarchy
- Training procedure state embedded in reusable mechanical components
- Runtime gameplay implemented through editor undo/redo history
- VR device types exposed throughout the engine
- Full cross-platform bitwise determinism without a demonstrated requirement
- Premature multithreading, job systems, plugins, or scripting
- Using Core as a repository for unrelated utilities

---

## 30. Architecture Decision Records

The following decisions should be captured as ADRs as their roadmap milestones approach:

1. ForgeSim product scope and shared game/training foundation
2. Application lifecycle, composition, and runtime ownership
3. Fixed-timestep contract and time domains
4. World/entity/component representation
5. Stable identifier and runtime-handle model
6. OpenGL abstraction depth and render submission model
7. Physics library and integration boundary
8. Transform authority and physics synchronization
9. Device-independent input and interaction model
10. Part, assembly, attachment, and connection representation
11. Domain-operation validation and result model
12. Serialization format, schema versioning, and asset identity
13. Editor command scope and undo/redo behavior
14. First functional-network domain and graph representation
15. Scenario observation and event semantics
16. VR runtime and dependency strategy
17. Multiplayer authority and replication model before networking begins
18. Replay fidelity and recording model before replay begins

An ADR should state context, constraints, considered alternatives, decision, consequences, and status. Superseded decisions should remain in history and point to their replacements.

---

## 31. Near-Term Architectural Focus

The current implementation has established a Sandbox executable, GLFW window, OpenGL context, framebuffer clearing, resize handling, and normal shutdown.

The next architectural work should remain limited to the active roadmap needs:

1. Confirm executable and subsystem lifecycle ownership.
2. Define the initial runtime loop and time-domain contract.
3. Preserve the existing `ForgeSim::Core` and `ForgeSim::PlatformGLFW` boundary.
4. Introduce rendering abstractions narrowly around concrete OpenGL requirements.
5. Delay final world, physics, interaction, and mechanical APIs until their milestones provide concrete use cases.

This approach preserves the long-term direction without front-loading the entire engine architecture into the Foundation milestone.
