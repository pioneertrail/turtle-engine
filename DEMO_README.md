# Silent Forge: Phase 1 Demo

## Overview
This demonstration showcases the five core components developed for Phase 1 of "Silent Forge: Rebellion of Aethelgard":

1. **PlasmaWeapon** - Energy weapon system with charging mechanics, multiple firing modes, and particle effects
2. **HealthSystem** - Combat damage system with various damage types, critical hits, and health management
3. **AIConstruct** - Artificial intelligence behavior system with state machine and different construct types
4. **TemporalAnomalySystem** - Time manipulation system with anomalies that affect entity behavior
5. **GestureRecognizer** - Pattern recognition for magical gestures that trigger special abilities

## Running the Demo
To run the demonstration, execute the `compile_and_run_demo.ps1` PowerShell script:

```powershell
.\compile_and_run_demo.ps1
```

The script will compile the project and run the demo, which walks through all the Phase 1 components individually and then demonstrates their integration.

## Demo Structure
The demonstration is broken down into 12 steps:

1. **PlasmaWeapon Demo** - Shows the charging mechanics and different firing modes
2. **HealthSystem Demo** - Demonstrates damage application, critical hits, and health management
3. **AIConstruct Demo** - Shows AI state transitions and behavior patterns
4. **TemporalAnomaly Demo** - Demonstrates time dilation effects and anomaly creation
5. **GestureRecognition Demo** - Shows pattern recognition for different magical gestures
6. **Combat Integration** - Demonstrates the PlasmaWeapon and HealthSystem working together
7. **AI and Temporal Interaction** - Shows how AI behavior is affected by temporal anomalies
8. **Advanced Gesture Combat** - Demonstrates gesture-triggered combat abilities
9. **Temporal Health Interaction** - Shows how temporal effects impact damage over time
10. **AI Damaged State** - Demonstrates AI behavior changes when heavily damaged
11. **Full System Integration** - A comprehensive scenario that uses all five systems together
12. **Demo Complete** - Summary of the demonstration

## Implemented Gestures
The demo includes three magical gestures:

- **Flammil** (forward thrust) - Triggers a quick plasma burst without charging
- **Aereth** (circular motion) - Creates a temporal distortion field
- **Turanis** (lightning bolt) - Provides an energy surge for maximum plasma weapon damage

## Requirements
- Windows 10 or later
- Visual Studio 2022 with C++20 support
- CMake 3.20 or higher

## Notes
This demonstration uses console output to simulate the visual effects and game mechanics. In the actual game implementation, these components would be integrated with the rendering engine and physics systems for a complete gameplay experience. 