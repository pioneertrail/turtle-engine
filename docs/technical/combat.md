# Combat System Documentation

## Overview
The combat system in Silent Forge: Rebellion of Aethelgard consists of three integrated components that handle weapon mechanics, enemy behavior, and health/damage management. These systems work together to create engaging combat encounters while maintaining balanced gameplay.

## Core Components

### 1. PlasmaWeapon System
The PlasmaWeapon system provides various weapon types with multiple firing modes, charge mechanics, and visual feedback.

#### Key Features
- Multiple firing modes: Burst, Beam, Charged, and Scatter
- Charge mechanics with variable power based on charge time
- Visual feedback through particle effects
- Debug visualization for weapon state
- Integration with the Flammil gesture recognition system

#### Usage Example
```cpp
// Create a plasma weapon with the particle system
auto plasmaWeapon = std::make_unique<PlasmaWeapon>(particleSystem);

// Set firing mode
plasmaWeapon->setFiringMode(PlasmaWeapon::FiringMode::CHARGED);

// Begin charging the weapon
plasmaWeapon->beginCharging();

// Fire the weapon when ready
if (!plasmaWeapon->isCoolingDown()) {
    plasmaWeapon->fire(playerPosition, targetDirection);
}
```

### 2. AIConstruct System
The AIConstruct system manages enemy behavior through a state machine, with different construct types having unique properties and attack patterns.

#### Key Features
- State machine with five states: Idle, Patrol, Attack, Retreat, and Damaged
- Four construct types: Sentry, Hunter, Guardian, and Swarm
- Type-specific properties like movement speed, attack range, and resistances
- Integration with the health system for damage handling
- Debug visualization for state and behavior

#### AI Construct Types
| Type     | Specialization                          | Resistances           |
|----------|----------------------------------------|-----------------------|
| Sentry   | Long-range, stationary                 | Plasma (50%)          |
| Hunter   | Fast, aggressive                       | Physical (20%)        |
| Guardian | High health, defensive                 | Physical (40%), Shield |
| Swarm    | Low health, fast, numerous             | None                  |

#### State Machine Flow
Idle → Patrol → Attack → (Damaged) → Retreat → Idle

### 3. Health System
The HealthSystem manages entity health, damage types, resistances, and visual feedback for damage and healing.

#### Key Features
- Multiple damage types (Physical, Plasma, Energy, Kinetic, Sonic, Temporal, Psychic)
- Resilience component for damage reduction and type-specific resistances
- Shield mechanics for temporary damage absorption
- Callbacks for damage, healing, and death events
- Debug visualization for health state

#### Damage Processing Flow
1. Incoming damage is processed by the Resilience component
2. Shield absorbs damage first if available
3. Flat damage reduction is applied
4. Type-specific resistances modify damage
5. Final damage is applied to current health
6. Visual feedback is generated through particles
7. Callbacks are triggered for damage and possible death

## System Interactions

### Weapon → AI Construct
- PlasmaWeapon generates damage information (DamageInfo)
- AIConstruct receives damage through applyDamage()
- AIConstruct may transition to Damaged state
- Particles visualize the impact

### AI Construct → Health Component
- AIConstruct has an internal HealthComponent
- Damage from weapons is passed to the HealthComponent
- HealthComponent processes damage through resistances
- HealthComponent callbacks notify AIConstruct of damage/death

### Debug Visualization
All three systems support debug visualization for development and testing:
- PlasmaWeapon: Charge level, cooldown status, firing mode
- AIConstruct: Current state, target position, detection ranges
- HealthSystem: Current health, resistances, recent damage

## Performance Considerations
- Particle effects are pooled for optimal performance
- AIConstruct behavior complexity scales based on distance from player
- Health calculations use optimized math for fast damage resolution

## Future Extensions
- Combo system integration with the PlasmaWeapon
- Advanced AI behavior patterns
- Environment damage interactions
- Tribe-specific resistance modifiers 