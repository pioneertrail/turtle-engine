# TurtleEngine

A modern OpenGL-based rendering engine optimized for various hardware configurations.

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
- GLEW
- GLFW3
- GLM

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

## Project Structure

```
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

## License

This project is licensed under the MIT License - see the LICENSE file for details. 