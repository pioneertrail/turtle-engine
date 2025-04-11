# Debug Visualization Guide

## Overview
Debug visualization in Silent Forge provides real-time visual feedback for developers to diagnose issues, monitor system performance, and validate gameplay mechanics. This guide outlines the available debug visualization tools across all major systems.

## Enabling Debug Visualization

### In Code
Most system components provide a method to enable debug visualization:

```cpp
// For health components
healthComponent->enableDebugVisualization(true);

// For AI constructs
aiConstruct->enableDebugVisualization(true);

// For plasma weapons
plasmaWeapon->enableDebugVisualization(true);
```

### At Runtime
Press F3 to toggle global debug visualization modes:
- Basic (F3): Show basic health and state information
- Detailed (F3 + Shift): Show detailed system information
- Trace (F3 + Ctrl): Show tracing information (paths, trajectories)

## Combat System Debug Visuals

### Plasma Weapon Debug
Toggle with `plasmaWeapon->enableDebugVisualization(true)`

**Visual Elements:**
- Charge level indicator (colored bar)
- Cooldown timer visualization
- Firing mode display
- Particle count information
- Damage range indicators
- Heat/power color mapping

**Example Output:**
```
[PLASMA] Mode: CHARGED | Charge: 78% | Cooldown: 0.0s | Particles: 34
```

### AI Construct Debug
Toggle with `aiConstruct->enableDebugVisualization(true)`

**Visual Elements:**
- Current state (color-coded outline)
- Detection radius (circle)
- Attack radius (circle)
- Path visualization (for patrol state)
- Health bar
- Target vector (arrow)
- Type identifier

**State Colors:**
- Idle: Blue
- Patrol: Green
- Attack: Red
- Retreat: Yellow
- Damaged: Purple

**Example Output:**
```
[AI][HUNTER] State: ATTACK | Health: 64% | Target: Player1 | Range: 8.2m
```

### Health System Debug
Toggle with `healthComponent->enableDebugVisualization(true)`

**Visual Elements:**
- Health bar (green to red gradient)
- Shield indicator (blue overlay)
- Resistances display (small icons)
- Recent damage events (floating numbers)
- Critical hit indicators (bright flash)

**Example Output:**
```
[HEALTH] Current: 76/100 | Shield: 25 | Resist: P=0.4, T=0.2 | Last: -14 PLASMA
```

## Temporal Anomaly Debug Visuals

**Visual Elements:**
- Anomaly radius (sphere with time distortion effect)
- Influence strength (color intensity)
- Affected entities highlighting
- Time dilation factor display
- Stability indicators

**Example Output:**
```
[ANOMALY][RIFT] Dilation: 0.4x | Radius: 12m | Duration: 7.3s | Affected: 3
```

## Input/Gesture Debug Visuals

**Visual Elements:**
- Hand position tracking (lines)
- Gesture recognition confidence (color)
- Timing information (millisecond display)
- Gesture history (fading trails)

**Example Output:**
```
[GESTURE] Type: Flammil | Confidence: 0.83 | Time: 428ms | Velocity: 4.2
```

## Performance Visualization

Enable with F4 to show performance metrics:

**Visual Elements:**
- FPS counter (with min/max/average)
- Frame time graph
- System-specific timing breakdowns
- Memory usage information
- Particle count
- Draw call counter

**Example Output:**
```
FPS: 62 (min:58, avg:61, max:65) | Frame: 16.1ms | Draw Calls: 142 | Particles: 324
```

## Logging Debug Information

All debug information displayed visually is also logged to:
- Console output (immediate)
- `logs/debug_visualization.log` (persistent)
- CSV data files for post-processing (when enabled)

## Creating Custom Debug Visualizers

To add custom debug visualization to new components:

1. Add a boolean toggle for enabling debug visualization
2. Create a debug info string in the component
3. Update the debug info in the update method
4. Register the debug visualizer with the global visualization system:

```cpp
DebugVisManager::registerComponent(this, [this](DebugVisRenderer& renderer) {
    if (!m_debugVisualizationEnabled) return;
    
    renderer.drawText(m_position, m_debugStateMessage);
    renderer.drawCircle(m_position, m_detectionRange, COLOR_DETECTION);
    // Additional visualization elements...
});
```

## Best Practices

1. **Color Consistency**: Use consistent colors across systems
   - Health: Green to Red
   - Energy/Plasma: Blue/Cyan
   - Warning/Critical: Yellow/Orange
   - Error: Red
   - Info: White

2. **Performance**: Disable expensive visualizations in release builds
   - Use `#ifdef DEBUG_BUILD` for visualization code
   - Batch similar visualization elements
   - Use distance-based detail reduction

3. **Clarity**: Keep text readable and concise
   - Use abbreviations consistently
   - Position text to avoid overlap
   - Fade out less important information
   - Group related information 