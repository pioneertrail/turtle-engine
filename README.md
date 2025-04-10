<<<<<<< HEAD
# Turtle Engine

A lightweight OpenGL-based rendering engine with interactive features and performance monitoring.

## Features

- OpenGL 3.3 core profile rendering
- Interactive shape manipulation
- Real-time performance metrics
- Cross-platform support (Windows, Linux)
- Modern C++ implementation
- CMake-based build system

## Prerequisites

- CMake 3.10 or higher
- C++17 compatible compiler
- OpenGL 3.3 capable graphics card
=======
# TurtleEngine

Engine for Silent Forge: Rebellion of Aethelgard

## Project Overview
This repository contains the source code for TurtleEngine, the custom game engine powering *Silent Forge: Rebellion of Aethelgard*. It includes core systems for rendering, physics, audio, and the unique Chronosign Language (CSL) gesture recognition system.

## Current Status (As of April 2025)
- **Phase 1: Core Systems** - In Progress
  - **Milestone 1.1: CSL Animation System** - In Progress
    - Basic gesture recognition for Khargail and Flammil implemented and validated.
    - Initial work on tribe-specific visual effects underway, focusing on Flammyx integration for the Flammil gesture (collaboration with Elena).
    - See `docs/project/roadmap.md` and `docs/project/tasks.md` for details.
  - **Milestone 1.2: Combat Mechanics** - In Progress (Marcus)
    - Hitbox detection and combo transition logging initiated.
  - **Milestone 1.3: Temporal Anomaly Framework** - Pending

## Hardware Requirements

### Minimum Requirements
- OpenGL 4.0 compatible graphics card
- 2GB RAM
- Windows 10/11 or Linux
- CMake 3.10 or higher
- C++17 compatible compiler

### Recommended Requirements
- OpenGL 4.0+ compatible graphics card
- 4GB RAM
- Modern CPU with good single-thread performance
- SSD storage

## Dependencies

- OpenGL 4.0
>>>>>>> feature/step2-latency-opt
- GLEW
- GLFW3
- GLM

<<<<<<< HEAD
## Building

### Windows with Visual Studio

```bash
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Debug
```

### Linux

```bash
mkdir build
cd build
cmake ..
make
```

## Running

After building, you can find the executables in:
- Windows: `build/bin/Debug/`
- Linux: `build/bin/`

### Main Application
Run `TurtleEngineApp` to start the interactive demo.

### Tests
Run `RenderTests` to execute the test suite.

## Controls

- **Movement**: WASD keys
- **Colors**: 
  - R: Red
  - G: Green
  - B: Blue
- **Size**: 
  - +: Increase size
  - -: Decrease size
- **Performance**: 
  - Tab: Toggle performance metrics

## Performance Metrics

The engine includes real-time performance monitoring:
- FPS (Frames Per Second)
- Frame time statistics (Average, Min, Max)
- 2-second sampling window (120 frames at 60 FPS)
=======
## Building the Project

1. Clone the repository:
```bash
git clone https://github.com/yourusername/turtle-engine.git
cd turtle-engine
```

2. Initialize and update submodules:
```bash
git submodule update --init --recursive
```

3. Create build directory:
```bash
mkdir build && cd build
```

4. Configure with CMake:
```bash
cmake ..
```

5. Build the project:
```bash
cmake --build .
```
>>>>>>> feature/step2-latency-opt

## Project Structure

```
<<<<<<< HEAD
src/
├── engine/
│   ├── include/         # Header files
│   │   ├── Engine.hpp
│   │   ├── Window.hpp
│   │   ├── Renderer.hpp
│   │   └── InputManager.hpp
│   └── src/            # Implementation files
│       ├── Engine.cpp
│       ├── Window.cpp
│       ├── Renderer.cpp
│       └── InputManager.cpp
├── tests/              # Test files
│   └── src/
│       └── RenderTests.cpp
└── main.cpp            # Main application entry point

shaders/               # GLSL shaders
├── test.vert
└── test.frag
```

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request
=======
turtle-engine/
├── src/
│   ├── engine/         # Core engine components
│   ├── main.cpp        # Entry point
│   └── tests/          # Test suite
├── shaders/            # GLSL shaders
└── deps/              # External dependencies
```

## Features

- Hardware-aware OpenGL context configuration
- Performance monitoring
- Basic rendering pipeline
- Input handling
- Scene management

## Controls

- ESC: Exit application
- F1: Toggle performance metrics
- WASD: Camera movement (when implemented)
- Mouse: Camera rotation (when implemented)

## Performance Optimization

The engine automatically detects hardware capabilities and adjusts its behavior accordingly:
- Texture size limits
- Uniform buffer limits
- Vertex attribute limits
- Shader complexity
>>>>>>> feature/step2-latency-opt

## License

This project is licensed under the MIT License - see the LICENSE file for details. 