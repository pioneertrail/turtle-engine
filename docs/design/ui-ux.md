# UI/UX Design Documentation

## Overview
This document outlines the user interface and user experience design principles for Silent Forge: Rebellion of Aethelgard. It covers UI components, information architecture, accessibility features, and visual language.

## Design Principles

### Core UX Pillars
1. **Immersive Minimalism**: Essential information only, with minimal screen clutter
2. **Contextual Awareness**: Relevant information appears when needed
3. **Gesture Feedback**: Clear visual cues for CSL gestures
4. **Temporal Clarity**: Time-based effects are visually distinct
5. **Accessibility First**: Designed to be playable by users with varying abilities

### Visual Language
The UI employs a tribal-technological fusion aesthetic:
- **Geometric Patterns**: Derived from tribal symbols
- **Holographic Elements**: Projected light interfaces
- **Color Coding**: Tribe-specific color schemes
- **Temporal Distortion**: Visual effects for time-affected UI elements

### Information Hierarchy
1. **Primary**: Critical gameplay information (Health, Active Effects)
2. **Secondary**: Important contextual information (Objective Status, Cooldowns)
3. **Tertiary**: Optional/depth information (Inventory Details, Lore)

## UI Components

### HUD Layout
```
+--------------------------------------------+
|  [HEALTH]          [OBJECTIVE]   [COMPASS] |
|                                            |
|  [GESTURE          [TEMPORAL     [AMMO/    |
|   INDICATOR]        EFFECTS]      ENERGY]  |
|                                            |
|                                            |
|                                            |
|                                            |
|                                            |
|                                            |
|                                            |
|  [ACTIVE           [TEAM         [MINI-    |
|   EFFECTS]          STATUS]       MAP]     |
+--------------------------------------------+
```

### Primary Components

#### Health Display
- Curved bar wrapping around the lower left of the screen
- Segmented to show threshold points
- Color gradient from green (full) to red (critical)
- Shield displayed as blue overlay
- Damage pulses and direction indicators

#### Gesture Indicator
- Real-time gesture path visualization
- Confidence meter showing recognition strength
- Cooldown timer for recent gestures
- Quick reference for available gestures

#### Temporal Effects
- Visual representation of active anomalies affecting the player
- Time dilation indicator showing relative time flow
- Countdown timers for duration-based effects
- Intensity meter for effect strength

#### Objective Display
- Current objective text with distance/direction indicator
- Progress tracker for multi-stage objectives
- Subtle objective updates with non-intrusive notifications
- Optional objective markers toggleable by player

### Secondary Components

#### Compass/Navigation
- Upper right directional compass
- Points of interest marked with symbols
- Distance indicators for key locations
- Elevation indicators for multi-level areas

#### Team Status
- Condensed health bars for teammates
- Tribal affiliation icons
- Status effect indicators
- Distance indicators when off-screen

#### Mini-map
- Lower right corner tactical view
- Toggle between different zoom levels
- Anomaly zones highlighted
- Enemy detection based on awareness

#### Ammo/Energy Display
- Weapon charge level visualization
- Available firing modes
- Cooldown indicators
- Energy reserves and regeneration rate

### Menu Screens

#### Main Menu
```
+--------------------------------------------+
|  SILENT FORGE: REBELLION OF AETHELGARD     |
|                                            |
|    [Continue Game]                         |
|    [New Game]                              |
|    [Multiplayer]                           |
|    [Options]                               |
|    [Codex]                                 |
|    [Credits]                               |
|    [Exit]                                  |
|                                            |
|                                            |
+--------------------------------------------+
```

#### Pause Menu
```
+--------------------------------------------+
|  GAME PAUSED                               |
|                                            |
|    [Resume]                                |
|    [Options]                               |
|    [Codex]                                 |
|    [Tutorial]                              |
|    [Return to Main Menu]                   |
|    [Exit Game]                             |
|                                            |
+--------------------------------------------+
```

#### Inventory/Loadout
```
+--------------------------------------------+
|  LOADOUT                    [TRIBAL STATS] |
|                                            |
|  [EQUIPMENT]                [GESTURES]     |
|   - Weapon                   - Khargail    |
|   - Armor                    - Flammil     |
|   - Utility                  - Stasai      |
|   - Consumables              - Annihlat    |
|                                            |
|  [RESOURCES]                [SKILLS]       |
|                                            |
+--------------------------------------------+
```

#### Skill Tree
Three-tab interface with:
- Offensive Tree (red)
- Defensive Tree (blue)
- Utility Tree (green)

Each node shows:
- Skill name and icon
- Brief description
- Cost to unlock
- Prerequisites

#### Codex
Organized into categories:
- Tribes and Characters
- CSL Gestures
- AI Constructs
- Temporal Anomalies
- World Lore
- Tutorial Archive

## Information Flow

### Notification System
- **Critical Alerts**: Center screen, require acknowledgment
- **Important Updates**: Edge notifications, fade after 3 seconds
- **Ambient Information**: Environmental cues and subtle HUD changes
- **Tutorial Tips**: Context-sensitive help that appears when needed

### Feedback Systems

#### Gesture Feedback
1. **Input Stage**: Gesture path visualization as player inputs
2. **Recognition Stage**: Visual confirmation of recognized gesture
3. **Effect Stage**: Tribal-specific visual effect for successful execution
4. **Cooldown Stage**: Timer visualization for reuse availability

#### Combat Feedback
1. **Attack Indicator**: Visual cue when attacking
2. **Hit Confirmation**: Visual/audio feedback on successful hit
3. **Damage Numbers**: Optional floating damage values
4. **Enemy Reaction**: Visual stagger/damage state
5. **Critical Hit**: Enhanced effect for critical damage

#### Navigation Feedback
1. **Interaction Prompts**: Contextual button prompts
2. **Pathfinding**: Subtle path indicators for current objective
3. **Area Transition**: Visual cues when changing zones
4. **Discovery**: Notification for new areas/items found

## Accessibility Features

### Visual Accessibility
- **High Contrast Mode**: Enhanced visibility of UI elements
- **Colorblind Modes**: Alternative color schemes for protanopia, deuteranopia, tritanopia
- **Text Scaling**: Adjustable text size
- **UI Scaling**: Adjustable UI element size
- **Screen Reader Support**: Text-to-speech for UI elements

### Auditory Accessibility
- **Subtitles**: Full subtitling of all dialogue
- **Visual Cues**: Optional visual indicators for important audio cues
- **Volume Controls**: Separate sliders for different audio types
- **Mono Audio**: Option to combine stereo channels

### Motor Accessibility
- **Control Remapping**: Fully customizable controls
- **Simplified Controls**: Option for reduced input complexity
- **Quick Sign Alternative**: Keyboard/button alternatives to gesture inputs
- **Reduced Motion**: Option to minimize screen shake and motion effects
- **Toggle Options**: Hold vs. toggle settings for sustained actions

### Cognitive Accessibility
- **Tutorial Access**: Ability to replay tutorials at any time
- **Objective Tracker**: Clear, persistent objective information
- **Difficulty Options**: Multiple difficulty levels affecting various aspects
- **Game Speed**: Option to reduce game speed during combat
- **Hint System**: Optional contextual hints

## UI States and Transitions

### Game State Transitions
- **Main Menu → Game**: Tribal loading screen with tips
- **Game → Pause**: Time freeze effect with desaturation
- **Area → Area**: Brief transition with location name
- **Game → Cutscene**: Letterbox transition
- **Game → Mission Complete**: Triumphant overlay with stats

### Context-Sensitive UI
The HUD adapts based on contextual factors:

#### Combat Mode
- Enlarged health display
- Prominent gesture feedback
- Enhanced enemy indicators
- Reduced peripheral information

#### Exploration Mode
- Expanded navigation elements
- Resource detection indicators
- Discovery highlights
- Reduced combat elements

#### Stealth Mode
- Minimalist HUD with essential information only
- Awareness indicators
- Sound visualization
- Detection risk meter

## Implementation Guidelines

### Technical Specifications
- **Resolution Support**: 16:9 and 21:9 aspect ratios (1080p, 1440p, 4K)
- **Safe Zones**: All critical UI within 90% safe zone
- **Performance**: UI limited to <5% of frame budget
- **Memory**: UI assets optimized for minimal memory footprint

### Asset Guidelines
- **Icons**: Vector-based for resolution independence
- **Fonts**: Custom "Chronovyan" font family with 5 weights
- **Animation**: Subtle, purpose-driven motion design
- **Audio**: Unique sound design for different UI interactions

### Localization Considerations
- Text containers allow for 30% expansion for translations
- Iconography designed to work across cultures
- Date/time formats adjustable by region
- Text direction support for RTL languages

## User Testing Protocols

### Key Metrics
- **Time to First Action**: How quickly can players perform their first meaningful action?
- **Navigation Errors**: How often do players navigate to the wrong screen?
- **Information Recall**: Can players recall key information from the UI?
- **Eye Tracking**: Where do players look during different game states?
- **Gesture Success Rate**: What percentage of gesture attempts succeed?

### Testing Focus Areas
1. **Gesture Recognition Feedback**: Is it clear when gestures are recognized?
2. **Combat Readability**: Can players track critical information during intense combat?
3. **Temporal Effect Clarity**: Do players understand when they're affected by time anomalies?
4. **Objective Understanding**: Can players easily determine their current objectives?
5. **Accessibility Effectiveness**: Do accessibility options properly address player needs?

## Style Guide

### Typography
- **Main Font**: Chronovyan Sans
- **Header Font**: Chronovyan Display
- **Body Text Size**: 18-24pt (adjustable)
- **Header Text Size**: 30-48pt (adjustable)

### Color Palette

#### UI Base Colors
- **Background**: Dark slate (#1A1C2C)
- **Text**: Off-white (#E9E9E9)
- **Accent**: Cyan (#5BCEFA)
- **Warning**: Amber (#FFB627)
- **Critical**: Crimson (#DC3545)

#### Tribal UI Colors
- **Flammil**: Red-orange (#FF5A1F)
- **Tempskrak**: Electric blue (#3A86FF)
- **Korcha**: Earthy green (#45A29E)
- **Voxumbra**: Deep purple (#8A4FFF)

### Iconography
- **Line Weight**: 2px base (scales with resolution)
- **Style**: Geometric with tribal motifs
- **Corner Radius**: 2px for rectangular elements
- **State Changes**: Color shift + subtle animation

### Animation Guidelines
- **Transitions**: 0.2s for standard, 0.1s for critical
- **Easing**: Ease-out for appearing elements, ease-in for disappearing
- **Hover States**: Subtle scale (105%) + brightness increase
- **Notification Enter**: Slide in + fade
- **Notification Exit**: Fade out

## Example Screens

### Combat HUD
```
+--------------------------------------------+
|  [▓▓▓▓▓▓▓▓-]      [Eliminate Sentries 2/4] |
|                                            |
|  [⟿⟿⟿⟿___]        [⏱ x0.7]       [▓▓▓▓▓▓-] |
|                                            |
|                                            |
|                                            |
|                                            |
|                                            |
|                                            |
|                                            |
|  [🔥 Flammil    [▓▓▓▓▓▓▓▓-]     [▴          |
|   0:05]         [▓▓▓▓▓▓--]      △          |
+--------------------------------------------+
```

### Skill Tree
```
+--------------------------------------------+
|  SKILLS: OFFENSIVE                [45 AP]  |
|                                            |
|          [Plasma      [Enhanced            |
|           Focus]       Flammil]            |
|             ↑             ↑                |
|  [Basic    [Weapon     [Gesture            |
|   Training] Mastery]    Speed]             |
|      ↑         ↑          ↑                |
|  [Damage   [Charging   [Critical           |
|   Boost]    Mastery]    Strikes]           |
|                                            |
+--------------------------------------------+
```

### Tribe Selection
```
+--------------------------------------------+
|  SELECT YOUR TRIBE                         |
|                                            |
|  [FLAMMIL]    [TEMPSKRAK]                  |
|   Fire         Lightning                   |
|   Area DMG     Speed                       |
|   ▓▓▓▓▓▓--     ▓▓▓▓----                    |
|                                            |
|  [KORCHA]     [VOXUMBRA]                   |
|   Earth        Shadow                      |
|   Defense      Stealth                     |
|   ▓▓▓▓▓---     ▓▓▓-----                    |
|                                            |
+--------------------------------------------+
``` 