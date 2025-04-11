# Game Systems Documentation

## Overview
This document details the interconnected systems that drive Silent Forge: Rebellion of Aethelgard. It explains how different gameplay components interact with each other and how designers can use these systems to create varied and engaging experiences.

## System Interactions Map

```
+--------------------+          +--------------------+
| CSL Gesture System |--------->| Tribal Effects     |
+--------------------+          +--------------------+
        |                                |
        v                                v
+--------------------+          +--------------------+
| Combat System      |<-------->| Temporal Anomalies |
+--------------------+          +--------------------+
        |                                |
        v                                v
+--------------------+          +--------------------+
| Progression System |<-------->| Mission System     |
+--------------------+          +--------------------+
```

## CSL Gesture System

### Core Components
- **Gesture Recognizer**: Translates player input into recognized gestures
- **Confidence Calculator**: Determines how accurately a gesture was performed
- **Effect Mapper**: Maps gestures to appropriate effects based on tribe
- **Visual Feedback**: Provides immediate feedback for player actions

### Design Parameters
| Parameter | Value Range | Effect |
|-----------|-------------|--------|
| Minimum Confidence | 0.6-0.9 | Threshold for successful gesture recognition |
| Speed Sensitivity | 0.5-2.0 | Multiplier for gesture execution speed influence |
| Cooldown Base | 0.2-5.0s | Base cooldown time between gestures |
| Quick Sign Power | 30-70% | Power percentage for keyboard/controller quick signs |

### Integration Points
- **Combat System**: Gestures trigger weapon modes and attacks
- **Tribal Effects**: Different tribes produce unique effects for each gesture
- **Temporal System**: Some gestures create or interact with temporal anomalies
- **UI System**: Visual representation of available gestures and cooldowns

### Design Guidelines
- Gestures should be distinct enough to avoid false recognitions
- Add visual cues that match the tribe's aesthetic for each gesture
- Maintain consistent response times (<0.5s) for all gestures
- Balance quick sign convenience with the power of full gestures

## Combat System

### Core Components
- **Plasma Weapon System**: Manages weapon states, firing modes, and effects
- **AI Construct System**: Controls enemy behavior, types, and capabilities
- **Health System**: Manages damage, healing, and entity state
- **Hitbox System**: Handles collision detection for attacks and entities

### Weapon Types and Parameters
| Weapon Type | Charge Time | Damage Range | Energy Usage | Special Properties |
|-------------|-------------|--------------|--------------|-------------------|
| Burst | 0.2-0.5s | 10-30/hit | Low | Rapid fire, high mobility |
| Beam | Sustained | 5-15/tick | High | Continuous damage, reduced mobility |
| Charged | 1.0-3.0s | 50-150 | Medium | Single powerful shot, charge can be held |
| Scatter | 0.5-1.0s | 8-20/particle | Medium | Wide area coverage, random spread |

### AI Construct Behaviors
| State | Transition Conditions | Actions | Notes |
|-------|----------------------|---------|-------|
| Idle | Player detected, Damage taken | Minimal movement, Occasional scan | Low resource usage state |
| Patrol | Player detected, Reached patrol point | Move between points, Scan surroundings | Medium aggression |
| Attack | Player in range, Barrier removed | Fire weapons, Position for attack | High aggression |
| Retreat | Health below threshold, Overwhelmed | Move away from player, Find cover | Self-preservation |
| Damaged | Taking significant damage | Brief stun or reaction, Visual feedback | Temporary state |

### Health and Damage Parameters
| Parameter | Range | Description |
|-----------|-------|-------------|
| Base Health | 50-500 | Starting health of an entity |
| Damage Types | 7 types | Physical, Plasma, Energy, Kinetic, Sonic, Temporal, Psychic |
| Resistance Values | 0.0-1.0 | Percentage damage reduction per type |
| Shield Capacity | 0-200 | Temporary damage absorption before health |
| Flat Reduction | 0-10 | Fixed damage reduction applied before resistance |

### Integration Points
- **CSL System**: Gestures trigger specific weapon modes and attacks
- **Temporal System**: Anomalies affect combat speed, damage, and movement
- **Progression System**: Combat performance influences affinity and rewards
- **Particle System**: Visual effects for weapons, damage, and health changes

### Design Guidelines
- Balance AI types to create varied and challenging encounters
- Create clear visual distinction between damage types and resistances
- Ensure weapon charge time balances with its damage output
- Design encounters that encourage different weapon modes

## Temporal Anomaly System

### Core Components
- **Anomaly Generator**: Creates temporal effects in the game world
- **Entity Modifier**: Applies temporal effects to entities within range
- **Visual Renderer**: Displays temporal distortions and effects
- **Physics Modifier**: Alters physics calculations within anomalies

### Anomaly Types and Effects
| Type | Time Distortion | Physics Effect | Visual Signature | Entity Impact |
|------|----------------|----------------|------------------|---------------|
| Rift | Variable | Unstable gravity | Blue-purple tears | Unpredictable |
| Stasis | 0.0 (stopped) | Frozen objects | Gold crystalline | Movement/attack pause |
| Dilation | 0.2-0.5 (slow) | Increased mass | Deep blue waves | Slowed actions |
| Acceleration | 1.5-3.0 (fast) | Decreased mass | Bright white | Increased speed |
| Reversion | -1.0 (reverse) | Inverted forces | Green spiral | Reversed actions |

### Anomaly Parameters
| Parameter | Range | Description |
|-----------|-------|-------------|
| Radius | 1.0-50.0 units | Area of effect for the anomaly |
| Duration | 1.0-60.0 seconds | How long the anomaly persists |
| Intensity | 0.0-1.0 | Strength of the temporal effect |
| Stability | 0.0-1.0 | How consistent the effect is (lower = more chaotic) |

### Integration Points
- **CSL System**: Specific gestures generate controlled anomalies
- **Combat System**: Anomalies affect attack speed, damage, and movement
- **Level Design**: Anomalies can be used as environmental hazards or puzzle elements
- **AI System**: AI constructs react to and sometimes manipulate anomalies

### Design Guidelines
- Use anomalies as both challenges and tools for the player
- Create visually distinctive effects for each anomaly type
- Balance powerful anomaly effects with limited duration or area
- Design encounters that encourage creative use of temporal effects

## Tribal Effects System

### Core Components
- **Tribe Manager**: Tracks player tribe affiliation and attributes
- **Effect Mapper**: Transforms generic gestures into tribe-specific effects
- **Visual Styler**: Applies appropriate visual style to effects based on tribe
- **Synergy Calculator**: Determines additional effects when multiple tribes combine

### Tribe Specializations
| Tribe | Element | Color Scheme | Strength | Weakness |
|-------|---------|--------------|----------|----------|
| Flammil | Fire/Plasma | Red-Orange | Area Damage | Single Target |
| Tempskrak | Lightning | Blue-White | Speed | Durability |
| Korcha | Earth | Green-Brown | Durability | Mobility |
| Voxumbra | Shadow | Purple-Black | Stealth | Direct Combat |

### Gesture Effects by Tribe
| Gesture | Flammil | Tempskrak | Korcha | Voxumbra |
|---------|---------|-----------|--------|----------|
| Khargail | Fire Bolt | Chain Lightning | Stone Projectile | Shadow Strike |
| Flammil | Fire Wave | Lightning Storm | Earth Rupture | Void Field |
| Stasai | Heat Shield | Speed Aura | Stone Armor | Shadow Meld |
| Annihlat | Inferno | Thunderclap | Earthquake | Abyssal Pull |

### Integration Points
- **CSL System**: Gestures are interpreted based on player's tribe
- **Combat System**: Tribal effects determine damage types and visual style
- **Progression System**: Tribe affinity unlocks more powerful variations
- **Multiplayer System**: Different tribes can create synergy effects

### Design Guidelines
- Maintain consistent visual language for each tribe's effects
- Ensure each tribe has unique strengths while remaining balanced
- Create meaningful differences in how each tribe approaches combat
- Design tribal upgrades that enhance the core identity of each tribe

## Progression System

### Core Components
- **Affinity Tracker**: Monitors progress in tribal abilities
- **Skill Tree Manager**: Handles unlocking and upgrading abilities
- **Equipment System**: Manages gear acquisition and improvement
- **Knowledge Database**: Tracks discovered lore and techniques

### Progression Metrics
| Metric | Acquisition | Usage | Cap |
|--------|-------------|-------|-----|
| Affinity Points | Mission completion, Combat performance | Skill unlocks | Uncapped |
| Gesture Experience | Using specific gestures | Gesture power improvement | Level 10 per gesture |
| Resources | Gathering, Mission rewards | Crafting, Upgrades | 999 per type |
| Knowledge Fragments | Exploration, Special objectives | Lore unlocks, Special abilities | 100% per region |

### Skill Trees Structure
1. **Offensive Tree**:
   - Tier 1: Basic damage improvements
   - Tier 2: Special attack modifications
   - Tier 3: Cooldown reductions and combos
   - Tier 4: Ultimate abilities

2. **Defensive Tree**:
   - Tier 1: Health and resistance improvements
   - Tier 2: Shield and recovery abilities
   - Tier 3: Counter techniques
   - Tier 4: Survival abilities

3. **Utility Tree**:
   - Tier 1: Movement enhancements
   - Tier 2: Resource gathering
   - Tier 3: Anomaly interactions
   - Tier 4: Special traversal and detection

### Integration Points
- **Combat System**: Character stats improve with progression
- **Tribal System**: Affinity unlocks tribe-specific enhancements
- **Mission System**: Progression gates access to new areas and missions
- **Temporal System**: Advanced progression enables better anomaly control

### Design Guidelines
- Balance skill trees to avoid mandatory paths
- Provide meaningful power increases with each upgrade
- Create interesting choices within each tree
- Ensure progression feels rewarding throughout the entire game

## Mission System

### Core Components
- **Mission Manager**: Tracks available and completed missions
- **Objective System**: Manages mission goals and progress
- **Reward Calculator**: Determines mission rewards based on performance
- **Difficulty Scaler**: Adjusts mission challenge based on player progression

### Mission Structure
1. **Preparation Phase**:
   - Mission briefing
   - Loadout selection
   - Team formation (multiplayer)

2. **Execution Phase**:
   - Zone navigation
   - Objective completion
   - Combat encounters
   - Optional objectives

3. **Extraction Phase**:
   - Return to extraction point
   - Survive final challenges
   - Secure gathered resources

### Objective Types
| Type | Description | Completion Metric | Reward Scale |
|------|-------------|-------------------|--------------|
| Elimination | Defeat specific enemies | Target count | Enemy power level |
| Collection | Gather specific items | Item count | Item rarity |
| Defense | Protect target | Survival time/health | Defense duration |
| Exploration | Discover locations | Area percentage | Area danger level |
| Stabilization | Control anomalies | Anomaly count | Anomaly intensity |
| Escort | Guide NPC/object | Delivery status | Route difficulty |

### Integration Points
- **Combat System**: Missions provide combat encounters
- **Temporal System**: Missions may involve specific anomaly interactions
- **Progression System**: Mission rewards feed into player advancement
- **Tribal System**: Some missions may be tribe-specific

### Design Guidelines
- Variety in mission types within each area
- Clear communication of objectives and rewards
- Optional objectives for risk/reward decisions
- Difficulty curve that matches player progression

## Multiplayer System

### Core Components
- **Session Manager**: Handles player connections and session state
- **Synchronization System**: Keeps game state consistent between players
- **Role Manager**: Tracks player roles and team composition
- **Shared Progression**: Manages rewards and advancement for the team

### Player Interactions
| Interaction | Mechanism | Effect | Limitations |
|-------------|-----------|--------|------------|
| Revive | Proximity interaction | Restore fallen teammate | Vulnerable during process |
| Resource Share | Inventory transfer | Send resources to teammates | Limited by inventory space |
| Combo Attacks | Synchronized gestures | Enhanced effects | Requires timing coordination |
| Tribal Synergy | Different tribes working together | Unique combined effects | Requires positioning |

### Team Composition
- **Balanced**: One of each tribe for maximum synergy
- **Specialized**: Multiple same-tribe members for amplified effects
- **Role-Based**: Players specializing in damage, support, control

### Integration Points
- **CSL System**: Synchronized gestures between players
- **Combat System**: Team tactics and combined attacks
- **Tribal System**: Cross-tribe synergy effects
- **Mission System**: Scaled difficulty and rewards for teams

### Design Guidelines
- Make cooperation meaningful without being mandatory
- Provide roles that cater to different playstyles
- Create synergies that reward coordination
- Balance content for both solo and multiplayer experiences

## System Tuning Variables

### Global Balance Parameters
These variables allow for adjusting the overall game balance:

| Parameter | Default Value | Description |
|-----------|---------------|-------------|
| DamageFactor | 1.0 | Global multiplier for all damage |
| HealthFactor | 1.0 | Global multiplier for entity health |
| ResourceDropRate | 1.0 | Rate at which resources are acquired |
| XpGainRate | 1.0 | Speed of progression through affinity levels |
| AnomalyIntensity | 1.0 | Global strength of temporal effects |
| GestureEaseOfUse | 0.8 | How forgiving gesture recognition is |
| AIDifficulty | 1.0 | Global multiplier for AI capability |

### Design Example: Creating a Tactical Encounter

```
Encounter: "Temporal Laboratory Defense"

Setup:
1. Place Stasis anomaly (radius 10.0) in center of room
2. Position 2x Sentry constructs guarding entrance
3. Position 1x Guardian construct near objective
4. Create Acceleration anomaly (radius 5.0) near flanking route
5. Place 3x Swarm spawners with timed activation

Player Options:
- Use Stasai gesture to extend existing Stasis anomaly
- Trigger Flammil gesture through Acceleration anomaly for amplified effect
- Perform Khargail on Guardian's shield generators to disable
- Use room's terminal to temporarily disable Swarm spawners

Tactical Elements:
- Sentries have high plasma resistance → incentivize physical damage
- Stasis field can trap enemies but also player projectiles
- Acceleration field provides faster movement but reduces aim precision
- Guardian shields regenerate → require coordinated burst damage
``` 