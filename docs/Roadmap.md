# ForgeSim Roadmap

## Project Objective

Develop ForgeSim incrementally into a polished, systems-oriented real-time engine and editor for mechanically interactive games and professional simulation applications.

ForgeSim will emphasize:

- Modern C++ and real-time engine architecture
- Physically based rendering and visually credible environments
- Interactive tools, equipment, and machinery
- Physics-supported mechanical interaction
- Data-driven parts, assemblies, and functional systems
- Desktop and VR interaction
- Testable simulation behavior and development tooling
- A reusable foundation shared by games and professional training applications

The first mechanically interactive project will be a focused mechanic-simulator vertical slice demonstrating inspection, diagnosis, tool use, mechanical disassembly, part replacement, reassembly, and functional testing on a compact authored machine.

Polished rendering showcases will validate the asset and rendering pipeline before that mechanical experience is assembled. VR interaction will follow the desktop foundation.

The immediate objective is a portfolio-quality engine and vertical slice. Scope will remain focused on capabilities that support that objective.

---

## Product Direction

ForgeSim is intended to support two related application categories through a shared engine foundation.

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

- Develop toward one coherent mechanic-simulator vertical slice.
- Use polished visual showcases to validate the asset and rendering pipeline before building mechanically interactive scenes.
- Keep showcase-specific art direction and asset preparation separate from reusable engine capabilities.
- Introduce abstractions in response to concrete requirements.
- Keep runtime and simulation systems independent of editor and UI dependencies.
- Keep platform-specific behavior isolated.
- Separate authored data, runtime state, and editor preferences.
- Introduce VR once the initial desktop interaction foundation is usable,
  then develop hands and basic tool interaction against a compact showcase.
- Preserve a path toward multiplayer, player construction, and larger
  systemic worlds without requiring them in the earliest milestones.- Add automated tests and diagnostic tools alongside the systems they validate.
- Measure performance before introducing substantial optimization infrastructure.
- Prefer a smaller number of polished, interoperable systems over many incomplete features.

The roadmap describes capability goals and sequencing. GitHub issues and milestones track implementation details, acceptance criteria, and current priorities.

Completed checkboxes indicate the stated capability exists within its documented scope. They do not imply support for every related production use case.

---

## Infrastructure and Release Engineering

Infrastructure work accompanies the phases it supports.

Build and test automation may be introduced independently of release
automation. Manual packaging must work before automated publishing is added.

Initial automation should validate Windows builds and headless tests.
Later workflows may package tagged releases and prepare release notes.

Public packages must run without the source checkout and include only
assets and dependencies permitted for redistribution.

Phase 15 hardens the release process established earlier; it is not the
first point at which ForgeSim becomes downloadable.

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

Phase 1 established the interactive application lifecycle, monotonic frame timing, bounded fixed-step scheduling, and safe handling of non-drawable framebuffer states.

## Runtime and Platform

- [x] Define application lifecycle ownership
- [x] Keep GLFW behavior within `ForgeSim::PlatformGLFW`
- [x] Establish `main()` as the composition and fatal-error boundary
- [x] Move interactive-loop ownership into `SandboxApplication`
- [x] Prevent repeated entry into the interactive loop
- [x] Implement clean initialization and deterministic shutdown
- [x] Release partially initialized platform resources safely
- [x] Preserve valid OpenGL-context lifetime for graphics resources
- [x] Process platform events through `GlfwWindow`
- [x] Handle close requests before entering an event wait
- [x] Recognize zero-sized framebuffers as non-drawable
- [x] Skip simulation and rendering while non-drawable
- [x] Avoid a busy loop while minimized
- [x] Resume normal operation after restoring the window

## Timing and Fixed-Step Scheduling

- [x] Implement monotonic real-frame timing
- [x] Use explicit `std::chrono` duration types
- [x] Implement a fixed-timestep accumulator
- [x] Configure an approximately 60 Hz fixed interval
- [x] Permit zero, one, or multiple fixed updates per drawable frame
- [x] Separate fixed updates from normal rendering
- [x] Limit catch-up work to four fixed updates per frame
- [x] Report dropped simulation time
- [x] Preserve fractional accumulated time
- [x] Expose interpolation alpha for future presentation use
- [x] Test fixed-step behavior without GLFW, OpenGL, or real sleeping
- [x] Test drawable-framebuffer policy without creating a window
- [x] Validate Debug and Release builds and tests

## Follow-Up Runtime Work

The following capabilities will be introduced when required by later phases:

- Additional runtime-loop test seams
- A dedicated reusable Runtime target
- Simulation pause, resume, and single-step
- Simulation-speed scaling
- Background-simulation and focus policy
- Rendering interpolation

**Milestone:** ForgeSim has explicit lifecycle ownership, monotonic frame timing, deterministic fixed-step scheduling, bounded catch-up, and safe minimized-window behavior.

**Status:** Complete.

---

# Phase 2 — Rendering and Interactive Sandbox Foundation

Establish a navigable 3D Sandbox with explicit graphics-resource ownership, externally authored geometry, and interactive diagnostics.

## Rendering Foundation

- [x] Establish OpenGL function loading
- [x] Establish graphics-resource ownership and lifetime rules
- [x] Implement shader compilation, linking, and diagnostics
- [x] Implement vertex buffers, index buffers, and vertex arrays
- [x] Implement a basic mesh representation
- [x] Load externally authored static glTF geometry
- [x] Report useful model-loading errors
- [x] Implement perspective camera and projection support
- [x] Submit basic objects for rendering

## Interactive Sandbox

- [x] Add desktop camera navigation
- [x] Add keyboard and pointer input handling
- [x] Integrate Dear ImGui for development controls
- [x] Add object selection and picking
- [x] Establish debug-rendering primitives
- [x] Add reference-grid and bounds visualization
- [x] Add runtime diagnostics
- [x] Add shader hot reloading

## Initial Asset Boundary

The initial loader provides representative authored geometry for renderer development.

Persistent asset identity, a project asset database, animation import, and production content processing remain later responsibilities.

**Milestone:** ForgeSim renders a navigable 3D environment containing externally authored static geometry, with interactive controls and diagnostics suitable for continued renderer development.

**Status:** Complete.

---

# Phase 3 — PBR Rendering and Visual Showcases

Establish a reusable material, texture, and scene-asset pipeline capable of rendering visually credible glTF 2.0 content.

Extend that foundation with lighting, shadows, image-quality features, and reproducible showcase scenes suitable for impressive screenshots and recordings.

## Textured Assets and Materials

- [x] Import multiple meshes, primitives, materials, and node transforms
- [x] Import positions, normals, texture coordinates, tangents, and indices
- [x] Decode images and upload textures through RAII-managed resources
- [x] Represent CPU-side material data independently of OpenGL
- [x] Support the glTF metallic-roughness material workflow
- [x] Support base-color, normal, metallic-roughness, emissive, and occlusion textures
- [x] Provide fallbacks for missing optional material data
- [x] Handle missing normals and tangents
- [x] Support alpha modes and double-sided materials
- [ ] Complete authored transform and imported-model pivot correctness

## Lighting and Color

- [x] Perform lighting calculations in linear space
- [x] Apply appropriate sRGB handling to color textures
- [x] Implement HDR rendering, exposure, and tone mapping
- [x] Implement image-based lighting and environment backgrounds
- [x] Implement directional, point, and spot lights
- [x] Add light inspection, validation, and debug visualization
- [ ] Establish a documented scene-scale and unit convention
- [ ] Calibrate light intensity and manual exposure conventions
- [ ] Load configurable default light settings from a data file

## Shadows and Interior Lighting

- [ ] Implement directional-light shadow mapping
- [ ] Implement screen-space ambient occlusion
- [ ] Support baked indirect lighting for static showcase scenes
- [ ] Implement bounded point-light shadow support
- [ ] Implement spotlight shadow mapping

## Image Quality and Scene Effects

- [ ] Implement anti-aliasing
- [ ] Implement bloom
- [ ] Implement manual-focus depth of field
- [ ] Support thin tinted glass
- [ ] Support RGB light cookies for stained-glass projection
- [ ] Implement shadowed directional volumetric lighting
- [ ] Implement localized smoke effects

## Showcase Authoring and Workflow

- [x] Add a PBR material validation gallery
- [x] Save and load scene documents
- [x] Support runtime scene-file operations and unsaved-change handling
- [x] Establish a minimal docked workspace with a rendered Scene viewport
- [x] Support object and light transform gizmos with undo and redo
- [ ] Improve camera-relative reference-grid behavior
- [ ] Report asset-loading stages, progress, and timings
- [ ] Save and restore authored camera and rendering settings
- [ ] Build a reproducible industrial workshop showcase
- [ ] Build a cathedral showcase demonstrating stained glass and lighting
- [ ] Capture clean screenshots and a short walkthrough
- [ ] Record representative loading and rendering performance

## Build and Distribution Infrastructure

- [ ] Establish one authoritative application version in CMake
- [ ] Display the application version in diagnostics
- [ ] Maintain a release-oriented CHANGELOG.md
- [ ] Define CMake installation rules for the executable and runtime files
- [ ] Resolve packaged resources independently of source/build paths
- [ ] Store writable user data outside the installation directory
- [ ] Produce a portable Windows Release package through CPack
- [ ] Include a small redistributable demonstration scene
- [ ] Include required third-party notices
- [ ] Verify the package on a machine without the development environment
- [ ] Publish a versioned preview release with usage instructions

## Architectural Constraints

- Asset parsing and CPU-side asset data remain independent of OpenGL.
- OpenGL resources and reusable rendering behavior remain within the renderer.
- Missing optional material data uses documented fallback values.
- Asset parsing and validation remain headlessly testable where practical.
- The Sandbox composes reusable systems and demonstrates their behavior.
- Rendering passes have explicit ordering, resource ownership, resize behavior, and graphics-state boundaries.
- Scene documents distinguish authored settings from transient editor state.
- New abstractions address immediate responsibilities or dependency boundaries.
- Purchased assets and derived files are distributed only where licensing permits.

## Completion Criteria

- Representative glTF scenes render with correct materials and transforms.
- Shadows and contact shading visibly ground objects in showcase environments.
- Lighting and exposure produce deliberate, repeatable compositions.
- Showcase camera and rendering settings can be restored after reopening.
- Image-quality and scene effects integrate correctly with the docked viewport.
- Existing navigation, selection, diagnostics, and shader hot reloading remain functional.
- Asset-loading and validation behavior has deterministic automated coverage where practical.
- Representative performance measurements identify hardware, build configuration, viewport resolution, and enabled effects.
- Debug and Release builds and tests pass.
- Screenshots or recordings demonstrate the completed visual result.
- Documentation describes supported features, limitations, and asset preparation.

## Scope Boundary

Phase 3 includes the bounded rendering features needed by its showcase scenes.

It does not require skeletal animation, physics integration, VR, a production asset browser, deferred rendering, a render graph, ray tracing, dynamic global illumination, an internal lightmap baker, virtualized geometry, or production asset streaming.

**Milestone:** ForgeSim can render and reproduce polished textured showcase scenes through a reusable PBR asset and rendering pipeline.

**Status:** In progress.

---

# Phase 4 — Editor Usability and Scene Authoring

Make ForgeSim practical for assembling and editing scenes without changing C++ code.

Build on the docked workspace and scene-document foundation established during Phase 3.

## Installation and Release Workflow

- [ ] Produce a Windows installer using the established package layout
- [ ] Verify installation, launch, upgrade, and uninstallation
- [ ] Preserve user scenes and preferences during supported upgrades
- [ ] Link the website download button to a tested public release
- [ ] Document supported operating systems and hardware requirements

## Scene and Asset Authoring

- [ ] Import additional glTF assets into an existing scene
- [ ] Support multiple independent model assets in one scene
- [ ] Create, rename, duplicate, and delete objects and lights
- [ ] Organize objects through parent/child relationships
- [ ] Define clear local-space and world-space transform behavior
- [ ] Inspect and edit supported object, material, and light properties
- [ ] Report missing assets and invalid edits through useful editor feedback

## Commands and History

- [ ] Establish a UI-independent command interface for durable editor mutations
- [ ] Extend undo and redo beyond transform gizmos
- [ ] Support property-edit commands
- [ ] Support object and light creation, duplication, and deletion commands
- [ ] Support hierarchy modification commands
- [ ] Preserve stable selection and identity across supported editing operations
- [ ] Keep editor history separate from runtime domain operations

## Navigation and Presentation

- [ ] Frame the selected object or light
- [ ] Provide predictable navigation speed across scene scales
- [ ] Add custom editor styling and selectable themes
- [ ] Persist editor preferences independently of scene documents
- [ ] Improve Inspector organization, labels, and property controls
- [ ] Preserve reliable workspace reset and layout persistence
- [ ] Provide a minimal asset-import workflow without requiring a production asset browser

## Reliability and Validation

- [ ] Validate scene references and authored property changes
- [ ] Preserve the current scene when opening or importing fails
- [ ] Test commands and scene mutations without Dear ImGui
- [ ] Verify save/open behavior after scene-authoring operations
- [ ] Document the supported authoring workflow and remaining limitations

## Scope Boundary

Phase 4 focuses on practical scene authoring.

Physics authoring, mechanical simulation controls, scenario editing, VR tooling, and a production asset database remain later responsibilities.

Scene identity, hierarchy, and mutation behavior introduced here must remain testable outside the UI and provide a foundation for reusable runtime systems.

**Milestone:** A user can create a scene, import multiple assets, arrange and inspect objects, edit lights and properties, undo meaningful changes, and save and reopen the result without modifying C++ code.

**Status:** Planned.

---

# Phase 5 — World and Component Foundation

Extract and extend the existing scene model into reusable runtime systems capable of supporting simulation.

Build on the identity, naming, transforms, hierarchy, and persistence established by earlier phases.

## World Representation

- [ ] Define the initial reusable world/entity/component model
- [ ] Establish stable runtime identity and persistent-reference rules
- [ ] Move reusable scene behavior out of Sandbox-specific ownership where justified
- [ ] Preserve naming and local/world transform behavior
- [ ] Define component attachment and lookup
- [ ] Define world traversal and system-update boundaries
- [ ] Define ownership and lifetime rules
- [ ] Separate authored state from transient runtime state
- [ ] Support creation and destruction outside editor code

## Runtime Integration

- [ ] Render world objects through renderer-facing data
- [ ] Reconstruct a runtime world from authored scene data
- [ ] Keep runtime world behavior independent of Dear ImGui
- [ ] Define how editor commands invoke reusable world mutations
- [ ] Test identity, hierarchy, transforms, components, and lifecycle operations
- [ ] Preserve the existing editor and showcase workflows

**Milestone:** ForgeSim can represent, update, render, and test a structured world through reusable systems independent of editor code.

**Status:** Planned.

---

# Phase 6 — Physics Foundation

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
- [ ] Integrate simulation pause, resume, and single-step controls

## Physics Diagnostics and Tests

- [ ] Visualize collision shapes and constraints
- [ ] Inspect rigid-body state
- [ ] Add minimal authoring controls for supported physics properties
- [ ] Test creation, destruction, and constraint lifetime
- [ ] Test repeatable fixed-step behavior within documented limits

**Milestone:** ForgeSim supports inspectable and testable rigid-body interaction and constrained mechanical motion.

**Status:** Planned.

---

# Phase 7 — Interaction and Control Foundation

## Input and Interaction Model

- [ ] Define input actions independently of raw devices
- [ ] Define interaction sources, intents, targets, and affordances
- [ ] Implement desktop object targeting
- [ ] Implement grabbing and releasing
- [ ] Implement constrained manipulation
- [ ] Identify interaction sources independently so multiple hands can be supported later
- [ ] Separate interaction requests from validated world mutations

## Interactive Controls

- [ ] Implement momentary buttons and persistent toggle switches
- [ ] Implement constrained levers and rotary controls
- [ ] Define control limits, activation thresholds, and state transitions
- [ ] Connect control changes to simple visible behavior
- [ ] Support desktop manipulation through device-independent operations
- [ ] Test control behavior independently of input devices and rendering

## Tools

- [ ] Define tool capabilities and compatible targets
- [ ] Implement tool pickup and positioning
- [ ] Implement at least two distinct tool actions
- [ ] Provide interaction highlighting and feedback
- [ ] Add interaction-state debugging
- [ ] Inspect supported interaction and tool properties
- [ ] Test interactions without Dear ImGui dependencies

## Interaction Targeting and Highlighting

- [ ] Identify the current interaction target independently of editor selection
- [ ] Define available, targeted, engaged, and unavailable interaction states
- [ ] Highlight the relevant control or movable part
- [ ] Distinguish interaction highlighting from editor selection highlighting
- [ ] Provide feedback for unavailable actions and explain the reason where useful
- [ ] Resolve overlapping targets predictably
- [ ] Clear feedback when targets are removed or become unavailable
- [ ] Expose interaction state to rendering without introducing UI dependencies
- [ ] Test targeting and state transitions independently of rendering

**Milestone:** A user can inspect, grab, manipulate, and operate tools on compatible objects using a desktop interaction harness.

**Status:** Planned.

---

# Phase 8 — VR Hands and Interactive Controls Showcase

Introduce VR as soon as the initial desktop interaction model is usable.

Use a compact industrial control panel to validate headset rendering,
tracked input, world scale, and shared desktop/VR interaction behavior.

## VR Runtime and Rendering

- [ ] Integrate an OpenXR-based VR runtime
- [ ] Isolate XR dependencies from world and interaction logic
- [ ] Implement session lifecycle and headset tracking
- [ ] Render each eye using its supplied view and projection
- [ ] Integrate XR frame timing and frame submission
- [ ] Handle session visibility, focus, and tracking changes
- [ ] Preserve the desktop application path
- [ ] Establish an explicit performance target for the test headset

## Articulated Hands and Direct Interaction

- [ ] Load a suitable licensed hand mesh and skeleton
- [ ] Implement the joint transforms and mesh deformation needed to render articulated hands
- [ ] Keep initial hand posing narrowly scoped without requiring a general animation editor
- [ ] Track controller grip and aim poses
- [ ] Display articulated virtual hands
- [ ] Drive finger poses from available controller inputs
- [ ] Define open, pointing, pressing, and gripping poses
- [ ] Map hand interaction into the shared interaction model
- [ ] Support proximity targeting and interactable highlighting
- [ ] Press buttons through fingertip contact
- [ ] Grab and move constrained switches and levers
- [ ] Grip and rotate knobs and valve handwheels
- [ ] Apply authored hand alignment and grasp poses to controls
- [ ] Release controls reliably when input or tracking is lost
- [ ] Provide visual and haptic interaction feedback
- [ ] Prevent conflicting simultaneous manipulation
- [ ] Retain controller-ray interaction as a fallback

## Basic Tool Interaction

- [ ] Grab, carry, release, and place one handheld tool
- [ ] Apply an authored hand grip pose to the tool
- [ ] Identify and highlight compatible tool targets
- [ ] Engage and disengage a tool at a compatible interaction point
- [ ] Use a bounded alignment or snapping guide
- [ ] Perform one tool action that changes the target's state
- [ ] Provide visual feedback for alignment, engagement, and completion
- [ ] Recover safely from release, tracking loss, or interrupted interaction
- [ ] Support the same tool operation through the desktop interaction path

## Hand and Tool Feedback

- [ ] Highlight controls targeted by a hand or controller ray
- [ ] Distinguish hover feedback from active manipulation
- [ ] Track interaction targets independently for each hand
- [ ] Resolve conflicting attempts to manipulate the same target
- [ ] Highlight compatible targets while holding a tool
- [ ] Show tool alignment, engagement, and completion feedback
- [ ] Keep feedback understandable without relying solely on color
- [ ] Clear transient interaction state after tracking loss or session interruption

## Industrial Controls Showcase

- [ ] Prepare independently movable control parts and authored pivots
- [ ] Adapt the existing button, toggle, lever, and rotary-control behaviors to hand input
- [ ] Connect controls to visible indicators or simple scene behavior
- [ ] Use the same control state and operations on desktop and in VR
- [ ] Validate panel dimensions, reachability, and tracking-space alignment
- [ ] Provide a repeatable demonstration setup

## Validation and Diagnostics

- [ ] Test control behavior independently of XR hardware
- [ ] Inspect interaction targets, control values, and active manipulators
- [ ] Measure frame timing on the target headset
- [ ] Verify safe behavior when tracking or session focus is lost
- [ ] Document supported hardware, runtime, bindings, and limitations

## Scope Boundary

This phase includes articulated hands, direct interaction with industrial
controls, and one handheld tool used at a compatible target.

Initial hands may use controller-driven finger poses and authored grasp
alignment. Tool engagement may use bounded snapping or alignment guides.

Full physical finger simulation, generalized grasp solving, advanced
two-handed tools, and complete mechanical disassembly remain later work.

Optical hand tracking must be explicitly included and validated if
controller-free interaction is required.

**Milestone:** A user can operate industrial controls with articulated
hands in VR, pick up a handheld tool, engage it with a compatible target,
and perform one meaningful tool action.

**Status:** Planned.

---

# Phase 9 — Mechanical Parts and Assemblies

## Mechanical Data Model

- [ ] Distinguish reusable part definitions from part instances
- [ ] Define assemblies and subassemblies
- [ ] Define attachment points and compatibility rules
- [ ] Define connection and constraint types
- [ ] Represent installed, removed, fastened, and connected states
- [ ] Represent part condition, wear, damage, and faults
- [ ] Separate physical behavior from durable logical mechanical state

## Mechanical Interaction Feedback

- [ ] Expose valid installation and removal targets through interaction affordances
- [ ] Indicate compatible attachment points while positioning a part
- [ ] Explain blocked operations, such as remaining fasteners or incompatible parts
- [ ] Update available interactions after assembly state changes
- [ ] Keep compatibility and operation rules independent of highlight rendering

## Assembly Operations

- [ ] Install and remove parts through validated domain operations
- [ ] Implement fastener interaction
- [ ] Enforce meaningful disassembly and assembly constraints
- [ ] Update assembly function when parts or connections change
- [ ] Validate assembly completeness and compatibility
- [ ] Visualize attachment points, connections, and assembly state
- [ ] Inspect and author supported assembly properties
- [ ] Add mechanical authoring commands where appropriate
- [ ] Keep runtime domain operations distinct from editor undo/redo commands
- [ ] Test assembly operations independently of editor and UI code

**Milestone:** ForgeSim supports a small functional machine that can be disassembled, altered, reassembled, and validated.

**Status:** Planned.

---

# Phase 10 — Project Assets and Mechanical Persistence

Extend the existing versioned scene-document foundation to support reusable asset identity and mechanically meaningful authored data.

## Persistence Model

- [ ] Extend schema versioning and migration rules as formats evolve
- [ ] Serialize stable world and asset references
- [ ] Serialize supported components and authored properties
- [ ] Serialize part definitions, assemblies, and connections
- [ ] Define authored mechanical state separately from saved runtime progress
- [ ] Keep transient renderer and physics resources out of authored documents
- [ ] Extend round-trip serialization and validation tests
- [ ] Report unresolved references and incompatible data clearly

## Projects and Assets

- [ ] Define project asset organization and reference-resolution rules
- [ ] Establish asset identifiers and handles
- [ ] Integrate mesh and texture loading with asset identifiers and handles
- [ ] Implement asset-managed material and part-definition loading
- [ ] Reconstruct runtime rendering resources from persistent asset references
- [ ] Distinguish source assets, imported data, and runtime resources
- [ ] Save and load the mechanic test scene
- [ ] Verify project relocation behavior within the supported reference model

**Milestone:** A project containing a mechanical assembly can be saved, closed, reopened, and reconstructed reliably through persistent asset and mechanical references.

**Status:** Planned.

---

# Phase 11 — Mechanic-Simulator Vertical Slice

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
- [ ] Reuse the visual presentation standards established by Phase 3

**Milestone:** ForgeSim delivers a cohesive desktop mechanic-simulator experience rather than a collection of isolated engine demonstrations.

**Status:** Planned.

---

# Phase 12 — Advanced VR Manipulation and Tools

## Advanced Hands and Grasping

- [ ] Extend existing hand poses to the tools and parts required by the repair
- [ ] Support transitions between holding, operating, and releasing tools
- [ ] Support regripping and hand repositioning where required
- [ ] Preserve stable manipulation when hands encounter constrained objects
- [ ] Recover safely from tracking loss and interrupted interactions

## Two-Handed Interaction

- [ ] Stabilize a part or tool with one hand while operating with the other
- [ ] Define primary and supporting hand roles
- [ ] Support deliberate handoff and release behavior
- [ ] Resolve competing manipulation requests predictably
- [ ] Support two-handed tools where required by the vertical slice

## Validation and Usability

- [ ] Complete the selected repair workflow in VR
- [ ] Validate reachability, comfort, precision, and interaction reliability
- [ ] Measure performance during representative repair operations
- [ ] Keep essential feedback understandable without controller haptics
- [ ] Preserve desktop interaction as a development and accessibility harness
- [ ] Preserve desktop interaction as a development and accessibility harness

## Tools and Mechanical Operations

- [ ] Extend basic tool interaction to the tools required by the selected repair
- [ ] Support repeated tool motions and regripping where required
- [ ] Coordinate tool movement with fastening, removal, and installation rules
- [ ] Remove, carry, position, and install mechanically meaningful parts
- [ ] Preserve reliable tool engagement during supported constrained motion
- [ ] Complete multi-step tool operations with clear progress and completion feedback

## Contextual Interaction Feedback

- [ ] Extend existing highlighting to multi-step repair operations
- [ ] Identify the active tool target and supporting-hand target
- [ ] Distinguish alignment, engagement, operation progress, and completion
- [ ] Explain operations blocked by mechanical state
- [ ] Keep highlights readable without obscuring small parts or tool contact points
- [ ] Support optional instructional guidance separately from mechanical validity

**Milestone:** The mechanic vertical slice can be completed in VR using articulated hands and physically credible tool interaction.

**Status:** Planned.

---

# Phase 13 — Functional Networks and Player Construction

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

**Status:** Planned.

---

# Phase 14 — Scenario, Training, and Validation Layer

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

**Status:** Planned.

---

# Phase 15 — Portfolio Release

## Product Polish

- [ ] Stabilize the desktop mechanic vertical slice
- [ ] Stabilize the VR version where supported
- [ ] Improve usability and presentation
- [ ] Complete performance and reliability passes
- [ ] Package a reproducible demonstration build

## Supporting Material

- [ ] Refresh showcase and mechanical-interaction screenshots
- [ ] Publish architecture and dependency diagrams
- [ ] Write a technical overview
- [ ] Record a demo video
- [ ] Verify build and demonstration instructions
- [ ] Prepare a portfolio description
- [ ] Document automated tests and validation results
- [ ] Document third-party dependencies and demonstration-asset requirements

**Milestone:** ForgeSim can be presented as a cohesive systems-oriented engine supporting a polished mechanic game experience and a credible path toward professional simulation.

**Status:** Planned.

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

- Lighting, shadow, and post-processing capabilities beyond the Phase 3 showcase scope
- Skeletal animation and broader animation tooling
- Expanded audio systems
- GPU profiling
- Instancing and batching
- Broader model and scene import support
- Advanced material models where required by demonstration content
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
- Production asset browser and asset database
- Incremental asset processing and caching
- Visual logic and network editors
- Node-based VFX authoring
- Expanded profiling interface
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
- Requiring production-scale asset streaming or rendering infrastructure to complete the visual showcases

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

Visual quality is an explicit engineering outcome during Phase 3. Showcase work should produce reusable rendering capabilities, documented asset workflows, and measurable results.

Later phases should build on that foundation while maintaining a clear path toward the mechanically interactive vertical slice.

A smaller number of polished, interoperable systems is preferable to a larger number of incomplete features.