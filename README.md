# ForgeSim

ForgeSim is an independently developed real-time simulation engine and editor written in modern C++, focused on interactive mechanical systems.

The project is being developed as a foundation for simulation-heavy games, interactive simulations, and training applications where mechanical components, equipment, tools, and other simulated systems can be represented through reusable engine and simulation infrastructure.

ForgeSim is currently in early development. The architecture and feature set will continue to evolve as concrete simulation requirements emerge.

## Vision

ForgeSim is intended to explore an engine architecture in which simulation systems are first-class components rather than application-specific implementations.

Potential applications include:

- Mechanical systems and assemblies
- Vehicle and equipment simulation
- Component interaction, replacement, and failure
- Interactive tools and maintenance workflows
- Procedural training scenarios
- Simulation-heavy games
- Debugging and visualization of simulated systems
- Future immersive and VR interaction

The long-term goal is not to reproduce the feature set of a general-purpose commercial game engine, but to develop reusable systems and tooling around interactive real-time simulation.

## Current Status

ForgeSim is in the early foundation stage.

Current development is focused on establishing a small, testable runtime foundation before higher-level simulation and editor systems are introduced.

Implemented foundation work includes:

- Modern C++20 project structure
- CMake and Ninja build workflow
- GLFW-based window and platform integration
- OpenGL rendering foundation
- Runtime application loop
- `std::chrono::steady_clock`-based timing
- Frame timing and runtime statistics
- Fixed-timestep simulation scheduling
- Bounded fixed-step catch-up
- Deterministic automated tests for core timing behavior
- Debug and Release build/test validation

The repository records the architecture, implementation, testing, and development history as the project evolves.

## Documentation

- [Architecture](docs/Architecture.md)
- [Roadmap](docs/Roadmap.md)

## Technical Direction

Planned areas of development include:

- Mechanical simulation systems
- Entity and scene representation
- OpenGL rendering systems
- Dear ImGui editor interface
- Scene hierarchy
- Property inspection and editing
- UI-independent command architecture
- Undo/redo
- Scenario and behavior systems
- Asset and resource management
- Serialization and project persistence
- Runtime/editor separation
- Debugging and visualization tools
- Profiling and telemetry
- Example mechanical simulation scenarios

Features are intentionally introduced as concrete requirements emerge rather than designing the complete engine architecture in advance.

## Building from Source on Windows

### Prerequisites

ForgeSim currently supports building on Windows with:

- Git
- CMake 3.24 or later
- Ninja
- An x64 MSVC toolchain

Run the following commands from a terminal in which the x64 MSVC developer environment is active:

```bash
git --version
cmake --version
ninja --version
cl
```

These commands should all succeed before configuring ForgeSim.

An internet connection is required during the first configuration so CMake can retrieve GLFW. GLAD's generated source and headers are included in the repository; Python and Jinja2 are not required.

### Clone the Repository

```bash
git clone https://github.com/L0932/ForgeSim.git
cd ForgeSim
```

### Debug Configuration

Configure and build:

```bash
cmake -S . -B out/build/x64-Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build out/build/x64-Debug --parallel
```

Run the tests:

```bash
ctest --test-dir out/build/x64-Debug --output-on-failure
```

Launch the Sandbox:

```bash
./out/build/x64-Debug/apps/Sandbox/ForgeSimSandbox.exe
```

### Release Configuration

Configure and build:

```bash
cmake -S . -B out/build/x64-Release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build out/build/x64-Release --parallel
```

Run the tests:

```bash
ctest --test-dir out/build/x64-Release --output-on-failure
```

Launch the Sandbox:

```bash
./out/build/x64-Release/apps/Sandbox/ForgeSimSandbox.exe
```

## Development Priorities

The current development sequence is broadly focused on:

1. Project and build infrastructure
2. Application and platform foundation
3. Rendering foundation
4. Timing and fixed-timestep simulation
5. Entity and scene representation
6. Editor integration
7. Scene hierarchy and property inspection
8. Command and undo/redo architecture
9. Simulation and mechanical systems
10. Scenario systems
11. Serialization and project persistence
12. Debugging and profiling tools
13. Example simulation scenarios

This sequence is expected to evolve as implementation exposes new requirements.

## Project Goals

ForgeSim serves several related goals:

- Develop reusable infrastructure for interactive real-time simulation
- Explore architecture for mechanical and systems-oriented simulation
- Develop modern C++ engine and tools systems
- Strengthen graphics and rendering engineering
- Explore editor and workflow tooling
- Develop deterministic and fixed-timestep simulation infrastructure
- Build debugging, visualization, and profiling tools for simulation
- Provide a foundation for future simulation applications and experiments
- Demonstrate engineering work relevant to simulation, engine, graphics, and tools development
- Potentially support future open-source, educational, consulting, or commercial work

## Development Approach

ForgeSim is developed incrementally around concrete engineering requirements.

Architecture, module boundaries, technical direction, and acceptance criteria are evaluated as the project develops rather than attempting to fully design the engine in advance.

AI-assisted tools may be used during development for research, review, validation, and implementation assistance. All committed material remains the project owner's responsibility and is expected to be understood, reviewed, tested, and maintainable.

AI-generated output is not assumed to be correct and must satisfy the same architectural and verification requirements as any other contribution.

## Independent Development

ForgeSim is a personal project developed independently of any employer, client, or contracting engagement.

The project's design, planning, architecture, source code, documentation, and other repository materials are developed using personal resources and maintained separately from work performed for employers or clients.

No confidential information, proprietary material, source code, datasets, work product, or other restricted materials belonging to an employer or client are intended to be incorporated into this repository.

## Repository History

The repository's Git history records the ongoing implementation and evolution of ForgeSim.

Architecture and implementation decisions may change as the project develops. The Git history should be considered the authoritative development record for material committed to the repository.

## Ownership

Unless otherwise explicitly stated, ForgeSim and original material contained in this repository are independently authored personal work.

Third-party libraries and dependencies remain subject to their respective licenses.

## License

No public software license has been granted at this stage.