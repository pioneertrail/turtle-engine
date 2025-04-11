# System Architecture

## Overview
TurtleEngine is a modular game engine designed for Silent Forge: Rebellion of Aethelgard. This document outlines the high-level architecture, system interactions, and design philosophy behind the engine.

## System Layers

The engine is organized into the following layers:

```
+-------------------------+
|        Game Layer       |
+-------------------------+
|    Gameplay Systems     |
+-------------------------+
|    Core Engine Layer    |
+-------------------------+
|      Platform Layer     |
+-------------------------+
```

### Platform Layer
- **Responsibility**: Hardware abstraction and platform-specific implementations
- **Components**: Window handling, input devices, file system, threading
- **Design Goal**: Isolate platform-specific code to enable cross-platform development

### Core Engine Layer
- **Responsibility**: Foundational systems that don't depend on gameplay specifics
- **Components**: Renderer, resource management, memory allocation, math utilities
- **Design Goal**: Provide stable, optimized interfaces for higher-level systems

### Gameplay Systems
- **Responsibility**: Game-specific mechanics and features
- **Components**: Combat, temporal anomalies, gestures, AI behavior
- **Design Goal**: Implement gameplay features using Core Engine interfaces

### Game Layer
- **Responsibility**: Game-specific content and logic
- **Components**: Levels, missions, progression, UI
- **Design Goal**: Create engaging content using Gameplay Systems

## Core Systems Architecture

### Rendering System
```
+----------------+    +----------------+    +----------------+
|  Render Queue  |<-->| Render Backend |<-->|  GPU Resources |
+----------------+    +----------------+    +----------------+
        ^                     ^
        |                     |
+----------------+    +----------------+
| Scene Graph    |    | Material System|
+----------------+    +----------------+
        ^                     ^
        |                     |
+----------------+    +----------------+
| Game Objects   |    | Asset System   |
+----------------+    +----------------+
```

### Memory Management
```
+----------------+    +----------------+    +----------------+
| Memory Tracker |<-->| Memory Pools   |<-->| Allocators     |
+----------------+    +----------------+    +----------------+
        ^                     ^                     ^
        |                     |                     |
+-------v--------+    +-------v--------+    +-------v--------+
| Engine Systems |    | Resource Cache |    | Object Factory |
+----------------+    +----------------+    +----------------+
```

### Event System
```
+----------------+    +----------------+    +----------------+
| Event Manager  |<-->| Event Queue    |<-->| Event Listeners|
+----------------+    +----------------+    +----------------+
        ^                                           ^
        |                                           |
+-------v--------+                          +-------v--------+
| Event Sources  |                          | Game Systems   |
+----------------+                          +----------------+
```

## Gameplay Systems Architecture

### Combat System
```
+----------------+    +----------------+    +----------------+
| Plasma Weapons |<-->| AI Constructs  |<-->| Health System  |
+----------------+    +----------------+    +----------------+
        ^                     ^                     ^
        |                     |                     |
+-------v--------+    +-------v--------+    +-------v--------+
| Particles      |    | Pathing        |    | Resilience     |
+----------------+    +----------------+    +----------------+
```

### Temporal System
```
+------------------+    +------------------+    +------------------+
| Anomaly System   |<-->| Affected Entities|<-->| Time Controllers |
+------------------+    +------------------+    +------------------+
        ^                        ^                       ^
        |                        |                       |
+-------v-----------+    +-------v-----------+    +------v-----------+
| Visual Effects    |    | Physics Effects   |    | Gameplay Effects |
+-------------------+    +-------------------+    +-------------------+
```

### Input System
```
+------------------+    +------------------+    +------------------+
| Input Manager    |<-->| Gesture System   |<-->| Command System   |
+------------------+    +------------------+    +------------------+
        ^                        ^                       ^
        |                        |                       |
+-------v-----------+    +-------v-----------+    +------v-----------+
| Platform Input    |    | Gesture Database  |    | Action Mapping   |
+-------------------+    +-------------------+    +-------------------+
```

## Core Design Principles

### Component-Based Architecture
TurtleEngine uses a component-based architecture for game objects:

- **Entity**: Basic container for components
- **Component**: Self-contained piece of functionality
- **System**: Processes entities with specific components

```cpp
// Example component usage
Entity player;
player.addComponent<TransformComponent>(glm::vec3(0.0f));
player.addComponent<HealthComponent>(100.0f);
player.addComponent<RenderComponent>("player_model.obj");
```

### Data-Oriented Design
Performance-critical systems use data-oriented design patterns:

- **Data Locality**: Group related data together
- **Cache Optimization**: Structure data for efficient cache usage
- **Parallelization**: Enable parallel processing of data

```cpp
// Example data-oriented particle storage
struct ParticleData {
    std::vector<glm::vec3> positions;    // All positions in one array
    std::vector<glm::vec3> velocities;   // All velocities in one array
    std::vector<float> lifetimes;        // All lifetimes in one array
    std::vector<glm::vec4> colors;       // All colors in one array
    size_t count;                        // Current particle count
};
```

### Service Locator Pattern
Systems discover and communicate with each other through a service locator:

```cpp
// Example service locator usage
auto particleSystem = ServiceLocator::get<ParticleSystem>();
auto audioSystem = ServiceLocator::get<AudioSystem>();
```

## System Interactions

### Combat → Temporal
Combat actions can create temporal anomalies:
- Plasma weapons generate distortion effects
- Certain AI constructs manipulate time fields
- Special weapons have temporal side effects

### Temporal → Physics
Temporal anomalies affect physics simulation:
- Time dilation zones modify physics timestep locally
- Stasis fields pause physics for affected objects
- Reversion anomalies reverse velocity vectors

### Input → Combat
Player input drives combat actions:
- Gesture recognition triggers specific weapon modes
- Input combinations create advanced combat effects
- Context-sensitive inputs react to combat state

## Threading Model

### Main Thread
- Game logic
- Physics update
- AI computation
- Input processing

### Render Thread
- Scene graph traversal
- Render command generation
- GPU communication

### Worker Threads
- Asset loading
- Particle system updates
- Background computations

### Thread Communication
- Lock-free queues
- Command buffers
- Job system

## Asset Pipeline

```
+----------------+    +----------------+    +----------------+
| Raw Assets     |-->| Asset Processor |-->| Engine Assets  |
+----------------+    +----------------+    +----------------+
                             |
                             v
                      +----------------+
                      | Asset Database |
                      +----------------+
                             |
                             v
                      +----------------+
                      | Runtime Loading|
                      +----------------+
```

### Asset Types
- Models (OBJ, FBX, GLTF)
- Textures (PNG, JPG, HDR)
- Shaders (GLSL)
- Sounds (WAV, OGG)
- Animations (FBX, custom)
- Materials (JSON)
- Particle Effects (JSON)

## Configuration System

### Engine Configuration
- Graphics settings
- Audio settings
- Input mappings
- Performance options

### Runtime Configuration
- Dynamic quality scaling
- Memory management policies
- Feature toggling

## Future Architecture Plans

### Planned Improvements
- **Multithreaded Rendering**: Move render submission to separate threads
- **ECS Evolution**: Complete transition to pure Entity-Component-System
- **Networking Layer**: Add multiplayer support
- **Scripting Engine**: Add scripting for gameplay logic

### Technology Upgrades
- **Renderer**: Upgrade to support Vulkan/DX12
- **Physics**: Integrate advanced physics engine
- **Audio**: Implement 3D spatial audio

## Architecture Diagram

```
+-----------------------------------------------------------------+
|                         Game Layer                               |
| +---------------------+  +------------------+  +---------------+ |
| | Game Logic          |  | UI System        |  | Game States   | |
| +---------------------+  +------------------+  +---------------+ |
+-----------------------------------------------------------------+
                                  |
+-----------------------------------------------------------------+
|                       Gameplay Systems                           |
| +---------------------+  +------------------+  +---------------+ |
| | Combat System       |  | Temporal System  |  | AI System     | |
| | - Plasma Weapons    |  | - Anomalies      |  | - Behavior    | |
| | - Heatlh System     |  | - Time Effects   |  | - Pathing     | |
| +---------------------+  +------------------+  +---------------+ |
+-----------------------------------------------------------------+
                                  |
+-----------------------------------------------------------------+
|                       Core Engine Layer                          |
| +---------------------+  +------------------+  +---------------+ |
| | Renderer            |  | Physics          |  | Resource Mgr  | |
| +---------------------+  +------------------+  +---------------+ |
| +---------------------+  +------------------+  +---------------+ |
| | Audio               |  | Input System     |  | Memory Mgr    | |
| +---------------------+  +------------------+  +---------------+ |
| +---------------------+  +------------------+  +---------------+ |
| | Particle System     |  | Event System     |  | Debug System  | |
| +---------------------+  +------------------+  +---------------+ |
+-----------------------------------------------------------------+
                                  |
+-----------------------------------------------------------------+
|                        Platform Layer                            |
| +---------------------+  +------------------+  +---------------+ |
| | Window Management   |  | File System      |  | Threading     | |
| +---------------------+  +------------------+  +---------------+ |
| +---------------------+  +------------------+  +---------------+ |
| | Hardware Detection  |  | Input Devices    |  | Networking    | |
| +---------------------+  +------------------+  +---------------+ |
+-----------------------------------------------------------------+
``` 