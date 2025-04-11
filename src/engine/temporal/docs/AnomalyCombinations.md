# Temporal Anomaly Combinations Design

## Overview
The temporal anomaly system in Silent Forge manages time-based effects on entities, allowing for diverse gameplay scenarios involving time manipulation. Temporal anomalies can overlap, requiring a systematic approach to handling effect combinations.

## Anomaly Types
The system supports six primary anomaly types, each with distinct effects:

- **RIFT**: Causes teleportation/displacement
- **STASIS**: Stops affected entities completely
- **DILATION**: Slows down affected entities
- **ACCELERATION**: Speeds up affected entities
- **REVERSAL**: Reverses movement direction
- **INSTABILITY**: Applies random time effects

## Combination Rules
When multiple anomalies affect an entity simultaneously, the following priority rules apply:

1. **STASIS** overrides all other effects (highest priority)
2. **REVERSAL** inverts the direction of movement
3. **DILATION** and **ACCELERATION** are averaged to determine a combined time modifier
4. **INSTABILITY** introduces randomness to the combined result
5. **RIFT** applies immediate position changes

The `TemporalCollisionHandler` class implements these rules, calculating the final time modification by merging effects based on their type, strength, and priority.

## Implementation
The core implementation consists of:

- `TemporalAnomalySystem`: Manages creation and lifecycle of anomalies
- `TemporalAnomaly`: Represents an individual anomaly instance
- `TemporalCollisionHandler`: Handles collisions between overlapping anomalies
- `AnomalyEffect`: Defines the properties of an effect (type, time distortion, etc.)

## Validation
The system includes validation for effect parameters:
- Radius must be positive
- Duration must be positive
- Time distortion must be finite and within reasonable bounds (|value| ≤ 100)

## Simplified Reference Implementation
A standalone demo (`src/demos/simple_temporal_demo.cpp`) demonstrates core concepts:
- **Anomaly Types**: All six types implemented with visual feedback
- **Combination Rules**: STASIS overrides all; DILATION and ACCELERATION average; REVERSAL inverts direction
- **Features**: ASCII visualization, interactive controls, error handling for radius, duration, and time distortion
- **Usage**: Compile with `g++ -std=c++17` or build via CMake with `SF_BUILD_TESTS=ON`

This demo simplifies the full `TemporalCollisionHandler` logic but illustrates basic effect interactions in a standalone, dependency-free environment.

## Example Usage
To see these combinations in action:

```powershell
# Run the simple demo (standalone, no dependencies)
.\run-temporal-demo.ps1 -demo simple

# Run the improved demo (requires GLM and optional OpenCV)
.\run-temporal-demo.ps1 -demo improved
```

## Future Enhancements
Planned enhancements to the anomaly combination system include:
- Weighted effect prioritization
- Custom combination behaviors for specific anomaly pairs
- Extended visualization of combined effects
- User-defined combination rules 