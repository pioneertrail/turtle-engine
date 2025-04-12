# TurtleEngine Temporal Demo - Documentation

## Overview
TurtleEngine is a custom game engine featuring a temporal anomaly system, particle effects, and OpenGL-based rendering. The Temporal Scene Demo showcases these features through interactive physics simulations with time distortion effects.

## Recent Fixes
- **OpenGL Header Resolution**: Fixed GLAD/GLFW header inclusion conflicts by implementing consistent inclusion order
- **Build System**: Restructured CMake configuration for proper component linking and dependency management
- **Logger Implementation**: Corrected include guards and namespace issues in the Logger module
- **Temporal System Integration**: Successfully integrated temporal physics with particle effects
- **Namespace Conflict Resolution**: Fixed namespace conflict between `TurtleEngine::TemporalAnomaly` and `TurtleEngine::Temporal::TemporalAnomaly` by properly restoring the `Temporal` namespace in all temporal system components

## Project Structure
```
TurtleEngine/
├── src/
│   ├── engine/
│   │   ├── include/           # Engine headers
│   │   │   ├── utils/         # Utility classes (Logger, etc.)
│   │   │   └── GlIncludes.hpp # Centralized OpenGL includes
│   │   ├── temporal/
│   │   │   ├── include/       # Temporal system headers
│   │   │   └── src/           # Temporal system implementation
│   │   └── src/               # Engine core implementation
│   └── demos/
│       ├── temporal_scene_demo.cpp  # Main demo file
│       └── temporal_gesture_blaster.cpp
├── shaders/                   # GLSL shaders for rendering
│   ├── particle.vert
│   └── particle.frag
├── textures/                  # Assets for rendering
│   └── particle.png
└── CMakeLists.txt             # Build configuration
```

## Git Development Workflow

TurtleEngine follows a streamlined git workflow for solo development:

### Branch Structure
- **new-main**: Primary development branch for most changes
- **feature/XXX**: Only used for experimental features or major changes
  - Example: `feature/particle-collision` for implementing a new particle collision system
- **bugfix/XXX**: Used for complex bug fixes that require isolation
  - Example: `bugfix/temporal-physics` for fixing incorrect time distortion calculations
- **refactor/XXX**: Used for significant code reorganization
  - Example: `refactor/renderer-pipeline` for restructuring the rendering pipeline

### Commit Message Format
```
<type>(<scope>): <subject>
```

#### Example Commit Messages:
```
feat(temporal): add time reversal anomaly type
fix(renderer): correct particle texture coordinate calculation
docs(demo): update temporal demo documentation with new controls
```

### Development Process
1. For most changes, work directly on `new-main`
2. Create descriptive commits following the format above
3. Push changes frequently to the remote repository
4. Only create feature branches for experimental or major changes
5. Document all changes in the appropriate documentation files

### Documentation Updates
- Document new features in header files using Doxygen-style comments
- Update demo documentation when adding new features or controls
- Add entries to the "Recent Fixes" section for significant changes

## Build Instructions

### Prerequisites
- CMake 3.20+
- Visual Studio 2022 (Windows) or GCC 10+ (Linux)
- vcpkg with the following packages:
  - glm
  - glfw3
  - glad
  - opengl

> **Note**: The following build instructions are Windows-specific. For Linux or macOS, please refer to the platform-specific sections at the end of this document.

### Building the Project (Windows)
```powershell
# Clone repository if not already done
git clone https://github.com/yourusername/TurtleEngine.git
cd TurtleEngine

# Create and enter build directory
mkdir -Force build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build . --config Debug

# Run the demo
cd bin/Debug
./temporal_scene_demo.exe
```

## Temporal Demo Features
- **Orbiting Objects**: Red spheres affected by temporal anomalies, blue spheres immune to time effects
- **Temporal Anomalies**: Create areas that slow down or speed up time for affected objects
- **Particle Effects**: Visualize time distortion with dynamic particle systems
- **Interactive Physics**: Objects change speed and color based on temporal influence

## Controls
- **Left Mouse Click**: Create new temporal anomaly
- **ESC**: Exit the demo

## Troubleshooting

### OpenGL Header Issues
If you encounter GLAD/OpenGL header conflicts:
```
error C1189: #error: OpenGL header already included, remove this include, glad already provides it
```

Ensure all engine files include headers in the correct order:
1. First include `GlIncludes.hpp`
2. Then include other engine headers
3. Finally include standard library headers

### Identifying Errors with the Logger
The Logger system is instrumental in debugging issues:
- Check console output for detailed error messages from the Logger
- Look for specific error codes or descriptions that identify the source of the problem
- The Logger provides different severity levels (TRACE, DEBUG, INFO, WARNING, ERROR, CRITICAL)
- Example Logger output for common issues:
  ```
  [ERROR] TemporalCollisionHandler: Cannot detect entity-anomaly collisions without anomaly system
  [WARNING] Shader: Unable to load texture file particle.png: File not found
  ```

### Build Issues
- Verify vcpkg is properly installed and the path to `vcpkg.cmake` is correct
- Check that all required packages are installed: `vcpkg install glm glfw3 glad --triplet x64-windows`
- Clear CMake cache if issues persist: `Remove-Item -Recurse -Force build` then rebuild

### Running the Demo
- Ensure shader and texture files are in the correct locations relative to the executable
- If textures don't load, check the console output for file path errors

## Development Notes
- The temporal system and particle effects are closely integrated - modifications to one may affect the other
- When creating new components, follow the established pattern of including `GlIncludes.hpp` first in implementation files
- The Logger system provides detailed output for debugging - check console logs when troubleshooting

## Future Improvements
Prioritized by implementation order:

### High Priority
- Camera movement implementation (WASD controls)
- More varied temporal anomaly types

### Medium Priority
- Enhanced particle effect variety
- Performance optimizations for large numbers of particles

### Low Priority
- Advanced lighting effects
- Audio feedback for temporal events 