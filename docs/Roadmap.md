# ForgeSim Roadmap

## Project Objective

Develop ForgeSim incrementally into a polished real-time simulation framework and editor demonstrating modern C++, graphics programming, simulation architecture, tools development, and software-engineering practices.

The immediate objective is a focused portfolio-quality implementation rather than a general-purpose replacement for existing commercial game engines.

---

# Phase 0 — Project Foundation

## Repository and Build System

- [ ] Establish Git repository
- [ ] Establish project directory structure
- [ ] Configure CMake
- [ ] Configure compiler warnings
- [ ] Establish Debug and Release configurations
- [ ] Integrate initial dependencies
- [ ] Create basic application executable

## Engineering Infrastructure

- [ ] Logging
- [ ] Assertions
- [ ] Basic profiling/timing utilities
- [ ] Coding conventions
- [ ] Architecture documentation

**Milestone:** ForgeSim builds cleanly and launches a minimal application.

---

# Phase 1 — Application and Rendering Foundation

## Platform

- [ ] Window creation
- [ ] OpenGL context
- [ ] Input handling
- [ ] Application lifecycle
- [ ] Main loop

## Rendering

- [ ] Shader abstraction
- [ ] Vertex/index buffers
- [ ] Vertex array abstraction
- [ ] Basic mesh representation
- [ ] Transform support
- [ ] Camera
- [ ] Texture loading
- [ ] Basic material representation

**Milestone:** ForgeSim renders a navigable 3D scene through its own rendering abstractions.

---

# Phase 2 — Simulation Foundation

- [ ] Implement simulation clock
- [ ] Implement fixed-timestep accumulator
- [ ] Separate simulation update from rendering
- [ ] Pause simulation
- [ ] Resume simulation
- [ ] Single-step simulation
- [ ] Simulation speed controls

**Milestone:** Simulation state updates independently from rendering frame rate.

---

# Phase 3 — Scene System

- [ ] Scene representation
- [ ] Object identity
- [ ] Object naming
- [ ] Transform representation
- [ ] Parent/child hierarchy
- [ ] Object creation/deletion
- [ ] Component/behavior architecture
- [ ] Scene traversal

**Milestone:** ForgeSim can represent and manipulate a structured simulation scene.

---

# Phase 4 — Editor Foundation

- [ ] Integrate Dear ImGui
- [ ] Establish editor layout
- [ ] Scene viewport
- [ ] Scene hierarchy panel
- [ ] Property inspector
- [ ] Object selection
- [ ] Transform editing
- [ ] Simulation controls

**Milestone:** Users can inspect and modify a running ForgeSim scene through an integrated editor.

---

# Phase 5 — Command and Undo/Redo System

- [ ] Define command interface
- [ ] Command execution
- [ ] Command history
- [ ] Undo
- [ ] Redo
- [ ] Property modification commands
- [ ] Object creation/deletion commands
- [ ] Transform commands
- [ ] Hierarchy modification commands

Editor functionality should invoke commands without making the command system dependent upon Dear ImGui.

**Milestone:** Major editor operations support undo and redo.

---

# Phase 6 — Persistence

- [ ] Select serialization approach
- [ ] Serialize scene
- [ ] Deserialize scene
- [ ] Save project/scene
- [ ] Load project/scene
- [ ] Serialize object properties
- [ ] Serialize hierarchy

**Milestone:** A simulation scene can be saved, closed, reopened, and reconstructed.

---

# Phase 7 — Scenario System

- [ ] Define scenario representation
- [ ] Define events
- [ ] Define conditions
- [ ] Define actions
- [ ] Implement scenario execution
- [ ] Implement basic scenario graph
- [ ] Build scenario editor
- [ ] Serialize scenario definitions
- [ ] Create example scenario

**Milestone:** Users can construct and execute interactive simulation logic without hard-coding the scenario into the application.

---

# Phase 8 — Debugging and Simulation Tools

- [ ] Debug rendering
- [ ] Runtime statistics
- [ ] Frame timing
- [ ] Simulation timing
- [ ] Entity/object inspection
- [ ] Logging console
- [ ] Scenario-state visualization

**Milestone:** ForgeSim provides useful tools for understanding and debugging running simulations.

---

# Phase 9 — Portfolio Demonstration

Develop at least one polished scenario demonstrating the framework.

The demonstration should exercise:

- Rendering
- Scene hierarchy
- Simulation
- Editor tools
- Scenario logic
- Commands
- Undo/redo
- Persistence
- Debug visualization

Produce supporting material:

- [ ] Screenshots
- [ ] Architecture diagrams
- [ ] Technical write-up
- [ ] Demo video
- [ ] Build instructions
- [ ] Portfolio description

**Milestone:** ForgeSim can be presented as a cohesive engineering project rather than a collection of isolated graphics demonstrations.

---

# Potential Future Development

These features are intentionally outside the initial core scope and may be explored later.

## Simulation

- Deterministic replay
- Recording
- Timeline system
- External data sources
- Physics
- Simulation networking
- Distributed simulation

## Rendering

- Advanced lighting
- Shadows
- Post-processing
- GPU profiling
- Instancing
- Model/scene import pipeline
- Alternative rendering APIs

## Architecture

- Job system
- Multithreading
- Plugin system
- Scripting
- Data-oriented systems
- Headless runtime

## Tooling

- Asset browser
- Visual graph improvements
- Profiling interface
- Simulation recording/replay UI
- Custom editor extensions

## Applications

Potential applications include:

- Training simulations
- Interactive visualization
- Scenario prototyping
- Engineering demonstrations
- Research simulations
- Educational simulations
- Game and interactive application prototyping
- Specialized commercial simulation tools

---

# Scope Principle

ForgeSim should not attempt to compete feature-for-feature with Unreal Engine, Unity, or other mature general-purpose engines.

Development should prioritize systems that demonstrate expertise in:

1. Modern C++
2. Real-time simulation
3. Graphics programming
4. Engine architecture
5. Tools programming
6. Software design
7. Performance-conscious development

A smaller number of polished, well-designed systems is preferable to a large number of incomplete features.
