# ForgeSim Architecture

## 1. Overview

ForgeSim is an independently developed real-time simulation framework and editor written primarily in modern C++.

The project is intended to provide a modular foundation for building interactive real-time simulations while serving as an exploration of engine architecture, graphics programming, simulation systems, and development tooling.

The architecture is intended to emphasize:

- Clear separation of systems
- Modular and reusable components
- Deterministic simulation where practical
- Separation between runtime and editor functionality
- Testable simulation logic
- Data-driven configuration
- Maintainability and extensibility
- Explicit ownership and lifetime management
- Performance appropriate for real-time applications

This document describes the initial architectural direction. Specific implementation decisions may change as the project develops.

---

## 2. High-Level Architecture

ForgeSim is planned around several major subsystems:

```text
ForgeSim
│
├── Application / Platform
│
├── Core
│   ├── Logging
│   ├── Timing
│   ├── Events
│   └── Utilities
│
├── Simulation
│   ├── Fixed-Timestep Loop
│   ├── Simulation Objects
│   ├── Systems
│   └── Scenario Logic
│
├── Rendering
│   ├── OpenGL Abstraction
│   ├── Shaders
│   ├── Meshes
│   ├── Materials
│   ├── Cameras
│   └── Debug Rendering
│
├── Scene
│   ├── Scene Representation
│   ├── Object Hierarchy
│   ├── Components
│   └── Serialization
│
├── Editor
│   ├── Dear ImGui Integration
│   ├── Scene Hierarchy
│   ├── Inspector
│   ├── Viewport
│   └── Editor Tools
│
├── Commands
│   ├── Command Interface
│   ├── Undo
│   └── Redo
│
├── Scenario
│   ├── Scenario Representation
│   ├── Graph
│   ├── Conditions
│   └── Actions
│
└── Assets
    ├── Resource Management
    ├── Loading
    └── Asset Metadata
```

---

## 3. Application Layer

The application layer owns the main program lifecycle.

Responsibilities are expected to include:

- Application initialization
- Window creation
- Input processing
- Main loop
- Simulation updates
- Rendering
- Editor integration
- Application shutdown

Platform-specific functionality should remain isolated from higher-level simulation and rendering systems where practical.

---

## 4. Simulation Architecture

ForgeSim will use a fixed-timestep simulation model.

Conceptually:

```text
Process Input
     │
     v
Accumulate Frame Time
     │
     v
Run Zero or More Fixed Simulation Steps
     │
     v
Render Current State
```

Simulation state should not depend directly upon editor UI state.

This separation should make simulation systems easier to test, reuse, pause, step, record, and potentially run independently of the editor.

Potential future capabilities include:

- Pause/resume
- Single-step simulation
- Simulation speed control
- Deterministic replay
- State recording
- Headless simulation

These capabilities are exploratory and are not commitments for the initial implementation.

---

## 5. Rendering Architecture

The initial renderer will use OpenGL.

The rendering layer should progressively abstract low-level graphics operations behind engine-level interfaces rather than allowing OpenGL calls to spread throughout unrelated systems.

Planned concepts include:

- Graphics context
- Buffers
- Vertex arrays
- Shaders
- Textures
- Meshes
- Materials
- Cameras
- Renderable objects
- Render commands
- Debug rendering

The initial objective is not to create a general-purpose AAA rendering engine.

The renderer exists to support real-time visualization of simulation environments while providing sufficient depth to demonstrate graphics and engine-programming techniques.

---

## 6. Scene Architecture

A scene represents the objects participating in a simulation.

Objects may contain:

- Identity
- Name
- Transform
- Parent/child relationships
- Components or behaviors
- Rendering information
- Simulation-specific state

The exact object/component architecture remains subject to experimentation.

The scene system should support serialization so simulations can be saved, loaded, and edited.

---

## 7. Editor Architecture

ForgeSim will include an editor built using Dear ImGui.

The editor should operate primarily as a client of underlying engine APIs rather than owning simulation logic itself.

Planned editor tools include:

- Main viewport
- Scene hierarchy
- Property inspector
- Asset browser
- Simulation controls
- Scenario editor
- Debugging tools

The separation between UI and engine operations is intentional so that functionality can potentially be invoked programmatically or through other interfaces in the future.

---

## 8. Command Architecture

Editor modifications should use a UI-independent command system where practical.

Commands may represent operations such as:

- Create object
- Delete object
- Modify property
- Reparent object
- Transform object
- Modify scenario

This architecture is intended to support undo/redo without coupling those operations directly to Dear ImGui.

---

## 9. Scenario System

ForgeSim is intended to support configurable simulation scenarios.

A scenario may eventually describe:

- Initial state
- Entities
- Events
- Conditions
- Actions
- Objectives
- State transitions
- Simulation outcomes

A graph-based representation is planned for expressing scenario logic.

The precise graph architecture will be determined through implementation and experimentation.

---

## 10. Serialization

ForgeSim should eventually support persistent representation of:

- Scenes
- Object properties
- Simulation configuration
- Scenario graphs
- Editor/project configuration

Serialization formats and libraries have not yet been finalized.

Human-readable formats may be preferred during early development for debugging and version-control friendliness.

---

## 11. Asset Management

Assets may include:

- Meshes
- Textures
- Shaders
- Materials
- Simulation configuration
- Scenario definitions

The initial implementation can use straightforward resource management and evolve toward more sophisticated asset tracking if required.

---

## 12. Dependency Strategy

ForgeSim should minimize unnecessary dependencies.

Potential external libraries include:

- GLFW for windowing and input
- GLAD or equivalent for OpenGL function loading
- Dear ImGui for editor UI
- GLM for mathematics
- A serialization library if appropriate

Dependencies should be incorporated deliberately rather than recreating mature infrastructure without a technical reason.

---

## 13. Build System

ForgeSim will use CMake.

The intended project structure should eventually allow separation of:

- Engine/runtime code
- Editor code
- Applications/examples
- Tests
- Third-party dependencies

The exact directory structure may evolve.

---

## 14. Testing

Systems that do not require a graphics context should be designed to permit automated testing where practical.

Potential testing targets include:

- Commands
- Undo/redo
- Scene operations
- Serialization
- Scenario evaluation
- Simulation timing
- Mathematics and utilities

---

## 15. Architectural Principles

### Clarity Over Cleverness

Prefer straightforward systems whose responsibilities are understandable.

### Composition Over Unnecessary Inheritance

Inheritance should be used where it models the problem effectively rather than as the default organizational mechanism.

### Explicit Ownership

Modern C++ ownership semantics and RAII should be used to make resource lifetimes understandable.

### UI-Independent Engine Functionality

Editor interfaces should invoke engine functionality rather than contain essential simulation behavior.

### Incremental Architecture

Abstractions should generally emerge from concrete requirements instead of being created solely for hypothetical future needs.

---

## 16. Future Exploration

Possible long-term areas of investigation include:

- Multithreaded systems
- Physics integration
- Networking
- Replay and recording
- Plugin architecture
- Scripting
- Headless execution
- Remote simulation control
- Large-world simulation
- Alternative rendering backends
- Data-oriented architectures
- Hardware-in-the-loop or external-system integration
- Specialized training and visualization applications

These are exploratory directions rather than commitments to the initial scope.
