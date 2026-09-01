# ForgeSim Roadmap

## Project Objective

Develop ForgeSim incrementally into a polished, systems-oriented real-time engine and editor for mechanically interactive games and professional simulation applications.

ForgeSim will emphasize:

- Modern C++ and real-time engine architecture
- Interactive tools, equipment, and machinery
- Physics-supported mechanical interaction
- Data-driven parts, assemblies, and functional systems
- Desktop and VR interaction
- Testable simulation behavior and development tooling
- A reusable foundation shared by games and professional training applications

The first project will be a focused mechanic-simulator vertical slice demonstrating inspection, diagnosis, tool use, mechanical disassembly, part replacement, reassembly, functional testing, and high-quality VR interaction on a compact authored machine.

The immediate objective is a portfolio-quality engine and vertical slice, not a feature-for-feature replacement for Unreal Engine, Unity, or another mature general-purpose engine.

---

## Product Direction

ForgeSim is intended to support two related application categories through a shared engine foundation:

### Games and Interactive Experiences

- Mechanic and repair simulations
- Physics-based mechanical construction
- Machines, vehicles, and assemblies
- Logic, power, fluid, and control systems
- Larger systemic and automation-oriented games over time

### Professional Simulation and Training

- Equipment and maintenance training
- Procedural instruction and assessment
- Fault diagnosis and operational workflows
- VR-based tool and machinery interaction
- Telemetry, replay, validation, and instructor-facing tools

Mechanical state must remain independent of game-specific progression and training-specific procedure logic. The same machine model should be usable by either application type where practical.

---

## Development Strategy

- Develop one coherent mechanic-simulator vertical slice throughout the roadmap.
- Introduce abstractions in response to concrete requirements from that slice.
- Keep runtime and simulation systems independent of editor and UI dependencies.
- Keep platform-specific behavior isolated.
- Preserve a path toward VR, multiplayer, player construction, and larger systemic worlds without requiring them in the earliest milestones.
- Add automated tests and diagnostic tools alongside the systems they validate.
- Prefer a smaller number of polished, interoperable systems over many incomplete features.

---

# Phase 0 — Project Foundation

## Repository and Build System

- [x] Establish Git repository
- [x] Establish project directory structure
- [x] Configure CMake
- [x] Configure compiler warnings
- [x] Establish Debug and Release configurations
- [x] Establish namespaced ForgeSim targets
- [x] Integrate initial dependencies deliberately
- [x] Create a basic Sandbox executable
- [x] Verify a reproducible clean-checkout build

## Engineering Infrastructure

- [x] Logging
- [x] Assertions
- [x] Automated test foundation
- [x] Basic profiling and timing utilities
- [ ] Coding conventions *(Engineering Hygiene — Low priority, non-blocking)*
- [x] Architecture documentation
- [ ] Architecture decision record structure *(Engineering Hygiene — Low priority, non-blocking)*

## Current Application Baseline

- [x] Create a GLFW window
- [x] Create an OpenGL context
- [x] Clear the framebuffer with a recognizable color
- [x] Handle framebuffer resizing correctly
- [x] Close the application normally

**Milestone:** ForgeSim builds cleanly, launches a minimal application, and has sufficient engineering infrastructure for incremental development.

**Status:** Complete. Remaining Engineering Hygiene items are valid backlog work but do not block this milestone.

---

# Phase 1 — Runtime, Platform, and Timing Foundation

Phase 1 technical implementation is prioritized ahead of the non-blocking Engineering Hygiene items remaining from Phase 0.

## Runtime and Platform

- [ ] Define application lifecycle ownership
- [ ] Complete platform-neutral window and input boundaries
- [ ] Keep GLFW-specific behavior within `ForgeSim::PlatformGLFW`
- [ ] Implement event processing
- [ ] Implement clean initialization and shutdown sequencing
- [ ] Establish the main runtime loop

## Time Domains

- [ ] Implement monotonic frame timing
- [ ] Implement a simulation clock
- [ ] Implement a fixed-timestep accumulator
- [ ] Separate simulation updates from rendering updates
- [ ] Define maximum catch-up behavior
- [ ] Pause and resume simulation
- [ ] Single-step simulation
- [ ] Support simulation speed control
- [ ] Test fixed-timestep behavior without a graphics context

**Milestone:** ForgeSim runs a testable fixed-step simulation independently of rendering frame rate and platform-specific code.

---

# Phase 2 — Rendering Foundation

## OpenGL Rendering

- [x] Establish OpenGL function loading
- [ ] Add graphics-resource ownership and lifetime rules
- [ ] Implement shader abstraction and diagnostics
- [ ] Implement vertex and index buffers
- [ ] Implement vertex array abstraction
- [ ] Implement basic mesh representation
- [ ] Select a narrow initial format for externally authored static meshes
- [ ] Load one externally authored static mesh into the runtime mesh representation
- [ ] Support the minimum vertex attributes required by the initial renderer
- [ ] Report useful model-loading errors
- [ ] Implement texture loading
- [ ] Implement basic material representation
- [ ] Implement camera and projection support
- [ ] Implement render submission for basic objects

## Rendering Diagnostics

- [ ] Add OpenGL debug output in development builds
- [ ] Add basic render statistics
- [ ] Add frame timing instrumentation
- [ ] Establish debug-rendering primitives

## Initial Model-Loading Boundary

The initial model loader exists to provide representative authored geometry for renderer development. It will support one deliberately narrow static-mesh format and only the attributes required by the current renderer.

This work does not establish the final asset database, project asset organization, persistent asset identity, animation import, or a general-purpose content pipeline. Those responsibilities remain in Phase 7 or later.

**Milestone:** ForgeSim renders a navigable 3D test environment containing externally authored static geometry through narrowly scoped OpenGL abstractions.

---

# Phase 3 — World and Scene Foundation

## World Representation

- [ ] Define the initial world/entity/component model
- [ ] Implement stable object identity
- [ ] Implement object naming
- [ ] Implement transform representation
- [ ] Implement parent/child hierarchy
- [ ] Implement object creation and deletion
- [ ] Implement component attachment and lookup
- [ ] Implement world traversal and system updates
- [ ] Define ownership and lifetime rules
- [ ] Separate authored state from transient runtime state

## Scene Integration

- [ ] Render world objects through renderer-facing data
- [ ] Add a controllable desktop camera
- [ ] Create a small test scene containing multiple objects
- [ ] Test identity, hierarchy, transform, and lifecycle operations

**Milestone:** ForgeSim can represent, update, render, and test a structured world without depending on editor code.

---

# Phase 4 — Physics Foundation

## Physics Integration

- [ ] Evaluate and select a physics approach or library
- [ ] Define physics-world ownership and update timing
- [ ] Integrate rigid bodies and collision shapes
- [ ] Implement collision filtering
- [ ] Implement ray casts and shape queries
- [ ] Implement fixed, hinge, and slider constraints
- [ ] Synchronize world transforms and physics state explicitly
- [ ] Define kinematic and dynamic object behavior
- [ ] Add sleeping and activation support

## Physics Diagnostics and Tests

- [ ] Visualize collision shapes and constraints
- [ ] Inspect rigid-body state
- [ ] Test creation, destruction, and constraint lifetime
- [ ] Test repeatable fixed-step behavior within documented limits

**Milestone:** ForgeSim supports inspectable and testable rigid-body interaction and constrained mechanical motion.

---

# Phase 5 — Interaction and Tool Foundation

## Input and Interaction Model

- [ ] Define input actions independently of raw devices
- [ ] Define interaction sources, intents, targets, and affordances
- [ ] Implement desktop object targeting
- [ ] Implement grabbing and releasing
- [ ] Implement constrained manipulation
- [ ] Support one-handed and two-handed interaction semantics
- [ ] Separate interaction requests from validated world mutations

## Tools

- [ ] Define tool capabilities and compatible targets
- [ ] Implement tool pickup and positioning
- [ ] Implement at least two distinct tool actions
- [ ] Provide interaction highlighting and feedback
- [ ] Add interaction-state debugging
- [ ] Test interactions without Dear ImGui dependencies

**Milestone:** A user can inspect, grab, manipulate, and operate tools on compatible objects using a desktop interaction harness.

---

# Phase 6 — Mechanical Parts and Assemblies

## Mechanical Data Model

- [ ] Distinguish reusable part definitions from part instances
- [ ] Define assemblies and subassemblies
- [ ] Define attachment points and compatibility rules
- [ ] Define connection and constraint types
- [ ] Represent installed, removed, fastened, and connected states
- [ ] Represent part condition, wear, damage, and faults
- [ ] Separate physical behavior from durable logical mechanical state

## Assembly Operations

- [ ] Install and remove parts through validated domain operations
- [ ] Implement fastener interaction
- [ ] Enforce meaningful disassembly and assembly constraints
- [ ] Update assembly function when parts or connections change
- [ ] Validate assembly completeness and compatibility
- [ ] Visualize attachment points, connections, and assembly state
- [ ] Test assembly operations independently of editor and UI code

**Milestone:** ForgeSim supports a small functional machine that can be disassembled, altered, reassembled, and validated.

---

# Phase 7 — Persistence and Asset Foundation

## Persistence Model

- [ ] Select a serialization approach
- [ ] Establish schema versioning rules
- [ ] Serialize stable object and asset references
- [ ] Serialize scenes and world hierarchy
- [ ] Serialize components and authored properties
- [ ] Serialize part definitions, assemblies, and connections
- [ ] Keep transient runtime and physics state out of authored data where appropriate
- [ ] Implement round-trip serialization tests

## Projects and Assets

- [ ] Define project and scene organization
- [ ] Establish asset identifiers and handles
- [ ] Integrate early mesh and texture loading with asset identifiers and handles
- [ ] Implement asset-managed material and part-definition loading
- [ ] Reconstruct runtime rendering resources from persistent asset references
- [ ] Distinguish source assets from runtime resources
- [ ] Save and load the mechanic test scene

**Milestone:** A project containing a mechanical assembly can be saved, closed, reopened, and reconstructed reliably.

---

# Phase 8 — Editor and Command Foundation

## Editor Inspection

- [ ] Integrate Dear ImGui without introducing runtime dependencies on it
- [ ] Establish editor layout and viewport
- [ ] Implement world hierarchy panel
- [ ] Implement property inspector
- [ ] Implement object selection
- [ ] Add runtime and simulation controls
- [ ] Add physics, interaction, and assembly inspection tools

## Commands and Editing

- [ ] Define a UI-independent editor command interface
- [ ] Implement command execution and history
- [ ] Implement undo and redo
- [ ] Add property and transform commands
- [ ] Add object creation and deletion commands
- [ ] Add hierarchy modification commands
- [ ] Add mechanical authoring commands where appropriate
- [ ] Route durable editor mutations through commands
- [ ] Keep runtime domain operations distinct from editor undo/redo commands

**Milestone:** Users can inspect and author a mechanical scene through the editor, with major authoring operations supporting undo and redo.

---

# Phase 9 — Mechanic-Simulator Vertical Slice

Develop a compact authored machine or subsystem that demonstrates the shared engine foundation.

## Required Experience

- [ ] Inspect the machine and its parts
- [ ] Identify a faulty or worn component
- [ ] Select and use appropriate tools
- [ ] Remove fasteners or connections
- [ ] Disassemble parts in a meaningful order
- [ ] Replace or repair the faulty component
- [ ] Reassemble the machine
- [ ] Perform a functional test
- [ ] Provide clear visual, audio, and state feedback

## Engineering Requirements

- [ ] Exercise rendering, world, physics, interaction, and assembly systems
- [ ] Exercise persistence and editor tooling
- [ ] Include automated validation of the completed assembly
- [ ] Include debugging views for mechanical and interaction state
- [ ] Profile the complete vertical slice

**Milestone:** ForgeSim delivers a cohesive desktop mechanic-simulator experience rather than a collection of isolated engine demonstrations.

---

# Phase 10 — VR Interaction

## VR Runtime

- [ ] Evaluate and select the VR integration approach
- [ ] Isolate VR-platform dependencies from the general runtime
- [ ] Implement headset and controller tracking
- [ ] Render stereoscopic views correctly
- [ ] Meet an explicit VR frame-time target

## Articulated Hands and Tools

- [ ] Implement articulated hand representation
- [ ] Map tracked input to hand poses
- [ ] Implement stable grasp poses
- [ ] Implement one-handed and two-handed tool use
- [ ] Support constrained mechanical interactions in VR
- [ ] Add haptic and audiovisual feedback where available
- [ ] Validate reachability, comfort, and interaction reliability
- [ ] Preserve desktop interaction as a development and accessibility harness

**Milestone:** The mechanic vertical slice can be completed in VR using articulated hands and physically credible tool interaction.

---

# Phase 11 — Functional Networks and Player Construction

## Functional Networks

- [ ] Define a general graph model for connected systems
- [ ] Implement one initial network domain, such as electrical connectivity or mechanical power
- [ ] Propagate state changes efficiently through connected components
- [ ] Implement sensors, controls, and instrumentation for the selected domain
- [ ] Visualize network topology and runtime state
- [ ] Serialize network definitions and connections

## In-World Construction

- [ ] Separate player construction from editor authoring
- [ ] Place and remove parts during gameplay
- [ ] Snap compatible attachment points
- [ ] Connect supported functional networks
- [ ] Validate constructed assemblies
- [ ] Save and reload player-built creations
- [ ] Test and operate constructed machines

**Milestone:** A player can construct, connect, save, and operate a small functional machine using runtime systems rather than editor-only functionality.

---

# Phase 12 — Scenario, Training, and Validation Layer

## Scenario Runtime

- [ ] Define scenario state, events, conditions, and actions
- [ ] Define objectives and outcomes
- [ ] Implement scenario execution independently of editor UI
- [ ] Serialize scenario definitions
- [ ] Implement fault injection through mechanical-domain APIs

## Procedure and Assessment

- [ ] Define procedures without embedding them in mechanical components
- [ ] Observe relevant domain operations and state transitions
- [ ] Evaluate required steps, ordering, and allowed deviations
- [ ] Record performance events and outcomes
- [ ] Provide instructor-facing state inspection where appropriate

## Headless Validation

- [ ] Run supported world and scenario logic without GLFW, OpenGL, or Dear ImGui
- [ ] Validate scenes, assemblies, and scenarios automatically
- [ ] Produce machine-readable validation results
- [ ] Add regression tests for representative workflows

**Milestone:** The same mechanical foundation supports an authored professional-training workflow with fault injection, assessment, telemetry, and automated validation.

---

# Phase 13 — Portfolio Release

## Product Polish

- [ ] Stabilize the desktop mechanic vertical slice
- [ ] Stabilize the VR version where supported
- [ ] Improve usability and presentation
- [ ] Complete performance and reliability passes
- [ ] Package a reproducible demonstration build

## Supporting Material

- [ ] Screenshots
- [ ] Architecture and dependency diagrams
- [ ] Technical write-up
- [ ] Demo video
- [ ] Build instructions
- [ ] Portfolio description
- [ ] Documented automated tests and validation results

**Milestone:** ForgeSim can be presented as a cohesive systems-oriented engine supporting a polished mechanic game experience and a credible path toward professional simulation.

---

# Long-Term Expansion

These capabilities are part of ForgeSim's intended direction but are not commitments for the first portfolio release.

## Larger Mechanical and Systemic Games

- More mechanical elements and connection types
- Multiple functional network domains
- Resource processing and production chains
- Automation and programmable control
- Larger vehicles and machines
- Persistent systemic worlds
- Simulation relevance, sleeping, and level-of-detail strategies
- Sharing or importing player-built creations

## Multiplayer

- Authoritative session model
- Stable network identity
- Input, command, and state replication
- Physics ownership and synchronization
- Cooperative construction and operation
- Instructor and trainee roles
- Session recording and review

## Rendering and Presentation

- Advanced lighting and shadows
- Post-processing
- Animation system
- Audio system
- GPU profiling
- Instancing and batching
- Broader model and scene import pipeline beyond the initial static-mesh format
- Specialized VR rendering optimizations
- Alternative rendering backends only when justified

## Simulation and Training

- Deterministic or authoritative replay within documented limits
- Timeline and recording tools
- Additional fault and damage models
- External data sources
- Hardware-in-the-loop integration
- Remote simulation control
- Distributed simulation where justified

## Engine Architecture and Tooling

- Job system and selective multithreading
- Scripting or project-level behavior extension
- Plugin or extension model
- Data-oriented optimizations where profiling justifies them
- Asset browser
- Visual logic and network editors
- Profiling interface
- Custom editor extensions

---

# Explicit Non-Goals for the Initial Roadmap

- Competing feature-for-feature with Unreal Engine, Unity, or another mature general-purpose engine
- Supporting every genre of game or simulation
- Building a complete vehicle-construction sandbox before validating a small assembly
- Simulating every mechanical behavior exclusively through rigid-body physics
- Providing every functional network type in the first release
- Shipping production multiplayer before the single-user mechanical foundation is proven
- Designing a backend-independent renderer without a concrete second-backend requirement
- Introducing major dependencies or abstractions solely for hypothetical future needs

---

# Scope Principle

Development should prioritize systems that demonstrate expertise in:

1. Modern C++
2. Real-time engine and simulation architecture
3. Graphics and physics programming
4. Mechanical and systems modeling
5. VR and interaction programming
6. Tools and editor development
7. Testable, data-driven software design
8. Performance-conscious development

A smaller number of polished, interoperable systems is preferable to a larger number of incomplete features.
