# Performance Optimization Guide

## Overview
Performance is a critical aspect of Silent Forge, with specific targets for different platforms. This document outlines optimization strategies, performance targets, and tools for ensuring the game runs smoothly across all supported hardware.

## Performance Targets

### Console Targets (PS5/Xbox Series X/S)
- **Resolution**: 4K
- **Framerate**: 60 FPS stable
- **Visual Quality**: High
- **Load Times**: < 2 seconds

### PC Targets

#### Minimum Specs (GTX 1660, Intel i5-8400, 16GB RAM)
- **Resolution**: 1080p
- **Framerate**: 60 FPS
- **Visual Quality**: Medium
- **Load Times**: < 5 seconds

#### Recommended Specs (RTX 3070, Intel i7-10700K, 32GB RAM)
- **Resolution**: 1080p-4K
- **Framerate**: 144 FPS at 1080p, 60 FPS at 4K
- **Visual Quality**: High/Ultra
- **Load Times**: < 3 seconds

## Performance Budget

### Frame Budget (16.67ms for 60 FPS)
- **Rendering**: 8ms
- **Gameplay Logic**: 4ms
- **Physics**: 2ms
- **Animation**: 1.5ms
- **Audio**: 0.5ms
- **Reserve**: 0.67ms

### Memory Budget
- **Textures**: 40% of available RAM
- **Meshes**: 20% of available RAM
- **Audio**: 10% of available RAM
- **Gameplay**: 15% of available RAM
- **Reserve**: 15% of available RAM

## Optimization Strategies

### Rendering Optimizations

#### Level of Detail (LOD)
- Implement distance-based LOD system for all models
- Reduce polygon count by 50% for medium distance
- Use billboard sprites for very distant objects
- Configure LOD distances based on platform capabilities

**Example:**
```cpp
// Set up LOD levels for a model
model->setLodLevels({
    { 0.0f, 10.0f, "high_poly_mesh.obj" },    // High poly up to 10 units
    { 10.0f, 50.0f, "medium_poly_mesh.obj" }, // Medium from 10-50 units
    { 50.0f, 100.0f, "low_poly_mesh.obj" },   // Low from 50-100 units
    { 100.0f, INFINITY, "billboard.obj" }     // Billboard beyond 100 units
});
```

#### Occlusion Culling
- Implement PVS (Potentially Visible Set) system
- Use occlusion queries to avoid rendering occluded objects
- Pre-compute visibility for static level geometry
- Configure occlusion thresholds based on platform

#### Shader Complexity
- Implement shader LOD system
- Reduce complexity for distant objects
- Use custom shader permutations for different quality settings
- Pre-warm shader cache to avoid runtime compilation

### Particle System Optimizations

#### Particle Budget
- **Low-end**: Max 1000 particles
- **Medium**: Max 2500 particles
- **High-end**: Max 5000 particles

#### Optimization Techniques
- Implement particle pooling to avoid allocation
- Use GPU instancing for particle rendering
- Implement distance-based culling
- Reduce emission rate based on performance
- Use billboarded quads instead of 3D meshes when possible

**Example:**
```cpp
// Configure particle system based on performance level
ParticleSystemSettings settings;
settings.maxParticles = determineParticleBudget();
settings.useGPUInstancing = true;
settings.useCulling = true;
settings.cullingDistance = determineParticleCullingDistance();
settings.usePooling = true;

particleSystem->configure(settings);
```

### Combat System Optimizations

#### PlasmaWeapon Optimizations
- Pool particle effects to avoid runtime allocation
- Use LOD for particle effects based on distance
- Batch similar plasma effects to reduce draw calls
- Use simplified physics for distant projectiles

#### AIConstruct Optimizations
- Implement AI LOD system (reduce update frequency for distant AI)
- Use simpler behavior trees for units outside player view
- Batch AI updates across multiple frames
- Implement spatial partitioning for efficient querying

**Example:**
```cpp
// AI update frequency based on distance
float getUpdateFrequency(float distanceToPlayer) {
    if (distanceToPlayer < 10.0f) return 1.0f;       // Full rate when close
    else if (distanceToPlayer < 30.0f) return 0.5f;  // Half rate when medium
    else if (distanceToPlayer < 60.0f) return 0.25f; // Quarter rate when far
    else return 0.1f;                                // Minimal updates when very far
}
```

#### Health System Optimizations
- Batch damage calculations when possible
- Optimize particle effects for damage visualization
- Use approximated physics for damage reactions
- Reduce callback frequency for high-frequency damage

### Memory Optimizations

#### Asset Loading
- Implement asynchronous loading system
- Use compressed textures (BC7 for high quality, BC1-BC5 for others)
- Implement texture streaming for open areas
- Unload unused assets when memory pressure is high

#### Data Structures
- Use memory pools for frequently allocated objects
- Minimize cache misses with data-oriented design
- Align data structures to cache lines (64 bytes)
- Use flat arrays instead of linked structures when possible

**Example:**
```cpp
// Memory pool for particle data
template<typename T>
class MemoryPool {
public:
    MemoryPool(size_t initialSize = 1024) {
        m_pool.reserve(initialSize);
    }
    
    T* allocate() {
        if (m_freeIndices.empty()) {
            m_pool.push_back(T());
            return &m_pool.back();
        } else {
            size_t index = m_freeIndices.back();
            m_freeIndices.pop_back();
            return &m_pool[index];
        }
    }
    
    void deallocate(T* ptr) {
        size_t index = ptr - m_pool.data();
        m_freeIndices.push_back(index);
    }
    
private:
    std::vector<T> m_pool;
    std::vector<size_t> m_freeIndices;
};
```

## Profiling and Monitoring

### Profiling Tools
- **CPU Profiling**: Built-in TurtleEngine profiler
- **GPU Profiling**: RenderDoc, NVIDIA Nsight
- **Memory Profiling**: Visual Studio Memory Profiler, Valgrind
- **Custom Tools**: Frame time graph, system-specific timing

### Built-in Performance Monitoring
Press F4 during gameplay to display performance metrics:
- FPS counter with min/avg/max
- Frame time breakdown by system
- Memory usage overview
- Draw call counter
- Particle count

### Performance Logging
When enabled, performance data is logged to:
- `logs/performance_[timestamp].csv` for later analysis
- `logs/performance_spikes.log` for investigating performance drops

## Optimization Workflow

1. **Identify** bottlenecks through profiling
2. **Analyze** specific system causing performance issues
3. **Implement** targeted optimizations
4. **Measure** impact of changes
5. **Iterate** until performance targets are met

### Example Optimization Flow

```
1. Profile shows high frame time (25ms)
2. Breakdown shows particle system taking 12ms
3. Implement particle pooling and count reduction
4. Re-measure frame time (now 18ms)
5. Next bottleneck is shader complexity (5ms)
...
```

## Platform-Specific Optimizations

### PlayStation 5
- Leverage GPU compute for particle systems
- Use haptic feedback to mask visual simplifications
- Take advantage of high-speed SSD for streaming

### Xbox Series X/S
- Use mesh shaders for complex geometry
- Implement variable rate shading for performance
- Take advantage of sampler feedback for texturing

### PC Specific
- Implement resolution scaling
- Provide extensive graphics options
- Support DLSS/FSR for higher-end GPUs
- Allow CPU/GPU work balancing settings

## Profiling Markers

Add performance markers in code to enable precise profiling:

```cpp
void update() {
    PROFILE_SCOPE("AISystem.Update");
    
    {
        PROFILE_SCOPE("AISystem.Decision");
        updateDecisions();
    }
    
    {
        PROFILE_SCOPE("AISystem.Movement");
        updateMovement();
    }
    
    {
        PROFILE_SCOPE("AISystem.Combat");
        updateCombat();
    }
}
```

## Performance Regression Testing

Automated performance tests run nightly to detect regressions:
- Compare against baseline on reference hardware
- Alert if performance drops by more than 5%
- Generate detailed report of affected systems
- Track performance trends over time

## Further Reading
- [Data-Oriented Design Principles](docs/further/data-oriented-design.md)
- [GPU Optimization Techniques](docs/further/gpu-optimization.md)
- [Memory Management Best Practices](docs/further/memory-management.md) 