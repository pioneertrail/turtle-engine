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
- GLEW
- GLFW3
- GLM

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

## Project Structure

```
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

## License

This project is licensed under the MIT License - see the LICENSE file for details. 