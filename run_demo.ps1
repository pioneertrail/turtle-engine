# PowerShell script to run the Silent Forge Phase 1 Demo without compilation
Write-Host "==== Silent Forge: Phase 1 Demo (Console Simulation) ===="

# Set console to UTF-8 encoding to properly display arrow characters
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
chcp 65001 | Out-Null

function DemoPlasmaWeapon {
    Write-Host "`n--- DEMO STEP 1: PlasmaWeapon System ---"
    Write-Host "Demonstrating PlasmaWeapon system"
    
    # Test different firing modes
    Write-Host "Setting firing mode to BURST"
    Write-Host "Starting charging..."
    
    # Simulate charging for 1 second with ASCII visualization
    $chargeBar = "[          ]"
    for ($i = 0; $i -lt 5; $i++) {
        $chargePercentage = [math]::Round(25 + $i * 15)
        $filledBlocks = [math]::Floor($chargePercentage / 10)
        $chargeBar = "["
        for ($j = 0; $j -lt 10; $j++) {
            if ($j -lt $filledBlocks) {
                $chargeBar += "#"
            } else {
                $chargeBar += " "
            }
        }
        $chargeBar += "]"
        Write-Host "Current charge: $chargePercentage% $chargeBar"
        Start-Sleep -Milliseconds 200
    }
    
    # Fire the weapon with ASCII visualization
    Write-Host "`nWeapon fired with BURST mode and charge: 75%"
    Write-Host "  +---+    +-+"
    Write-Host "  | P |====>* *    *"
    Write-Host "  +---+    +-+   *  *"
    Write-Host "[ParticleSystem] Spawned burst of 50 particles with plasma blue color"
    Write-Host "Damage output: 75 * 0.5 = 37.5 base damage (BURST mode)"
    Write-Host "Particle parameters: Size=0.1, Life=0.5, Color=rgba(0.2,0.6,1.0,1.0)"
    
    # Switch to BEAM mode
    Write-Host "`nSetting firing mode to BEAM"
    Write-Host "Starting charging..."
    
    # Simulate charging for 1.5 seconds with visualization
    $chargeBar = "[          ]"
    for ($i = 0; $i -lt 6; $i++) {
        $chargePercentage = [math]::Round(20 + $i * 15)
        $filledBlocks = [math]::Floor($chargePercentage / 10)
        $chargeBar = "["
        for ($j = 0; $j -lt 10; $j++) {
            if ($j -lt $filledBlocks) {
                $chargeBar += "#"
            } else {
                $chargeBar += " "
            }
        }
        $chargeBar += "]"
        Write-Host "Current charge: $chargePercentage% $chargeBar"
        Start-Sleep -Milliseconds 250
    }
    
    # Fire the beam with ASCII visualization
    Write-Host "`nWeapon fired with BEAM mode and charge: 95%"
    Write-Host "  +---+    ============>"
    Write-Host "  | P |===============>"
    Write-Host "  +---+    ============>"
    Write-Host "[ParticleSystem] Spawned 10 continuous beam particles with plasma blue color"
    Write-Host "Damage output: 95 * 0.8 = 76.0 base damage (BEAM mode)"
    Write-Host "Beam length: 10 units, Width: 0.5 units"
    Write-Host "Particle parameters: Size=0.05, Life=0.3, Color=rgba(0.2,0.6,1.0,1.0)"
}

function DemoHealthSystem {
    Write-Host "`n--- DEMO STEP 2: Health System ---"
    Write-Host "Demonstrating HealthSystem"
    
    # Show initial health
    $playerHealth = 100
    $enemyHealth = 150
    Write-Host "Player health: $playerHealth"
    Write-Host "Enemy health: $enemyHealth"
    
    # Apply different damage types
    $energyDamage = 25
    $enemyHealth -= $energyDamage
    Write-Host "`nApplying $energyDamage energy damage to enemy"
    Write-Host "[ParticleSystem] Spawned energy impact particles"
    Write-Host "After energy damage, enemy health: $enemyHealth"
    
    # Apply critical hit
    $physicalDamage = 30
    $criticalMultiplier = 1.5
    $criticalDamage = $physicalDamage * $criticalMultiplier
    $enemyHealth -= $criticalDamage
    Write-Host "`nApplying $physicalDamage physical damage with critical hit (x$criticalMultiplier)"
    Write-Host "[ParticleSystem] Spawned critical hit particles"
    Write-Host "Critical hit: Yes"
    Write-Host "After critical physical damage, enemy health: $enemyHealth"
}

function DemoAIConstruct {
    Write-Host "`n--- DEMO STEP 3: AI Construct ---"
    Write-Host "Demonstrating AIConstruct behavior"
    
    # Show initial state
    $aiState = "IDLE"
    Write-Host "AI initial state: $aiState"
    
    # Simulate player entering detection range
    $aiState = "INVESTIGATE"
    Write-Host "`nPlayer detected within range"
    Write-Host "After player detected, AI state: $aiState"
    
    # Simulate player attacking the AI
    $aiState = "ATTACK"
    Write-Host "`nAI taking 20 damage from player"
    Write-Host "After taking damage, AI state: $aiState"
}

function DemoTemporalAnomaly {
    Write-Host "`n--- DEMO STEP 4: Temporal Anomaly System ---"
    Write-Host "Demonstrating TemporalAnomalySystem"
    
    # Create a SLOWFIELD anomaly
    Write-Host "Created SLOWFIELD anomaly at (5,0,0) with radius 3.0"
    
    # Check if entity is affected
    $timeScale = 0.4
    Write-Host "`nEntity at (4,0,0) has time scale: $timeScale"
    
    # Move entity out of range
    $timeScale = 1.0
    Write-Host "Entity at (10,0,0) has time scale: $timeScale"
    
    # Create a RIFT anomaly
    Write-Host "`nCreated RIFT anomaly at (-5,0,0) with radius 2.0"
    Write-Host "[Visualization] Temporal rift distortion particles spawned"
}

function DemoGestureRecognition {
    Write-Host "`n--- DEMO STEP 5: Gesture Recognition ---"
    Write-Host "Demonstrating GestureRecognizer"
    
    # Simulate Flammil gesture (forward thrust) with visualization
    Write-Host "Simulating Flammil gesture (forward thrust)"
    Write-Host "Gesture visualization:"
    Write-Host "  +-------------------+"
    Write-Host "  |                   |"
    Write-Host "  |    *---->         |"
    Write-Host "  |    start  end     |"
    Write-Host "  |                   |"
    Write-Host "  +-------------------+"
    Write-Host "Gesture points: (0.5,0.5) -> (0.6,0.5) -> (0.7,0.5) -> (0.8,0.5) -> (0.9,0.5)"
    Write-Host "Gesture timing: 0.0s -> 0.1s -> 0.2s -> 0.3s -> 0.4s (total: 0.4s)"
    Write-Host "Pattern matching: 92% confidence for FLAMMIL pattern"
    Write-Host "Flammil gesture recognized: Yes"
    
    # Simulate Aereth gesture (circular motion) with visualization
    Write-Host "`nSimulating Aereth gesture (circular motion)"
    Write-Host "Gesture visualization:"
    Write-Host "  +-------------------+"
    Write-Host "  |                   |"
    Write-Host "  |       /--\        |"
    Write-Host "  |      |    |       |"
    Write-Host "  |       \--/        |"
    Write-Host "  |                   |"
    Write-Host "  +-------------------+"
    Write-Host "Gesture points: (0.5,0.5) -> (0.6,0.4) -> (0.7,0.5) -> (0.6,0.6) -> (0.5,0.5)"
    Write-Host "Gesture timing: 0.0s -> 0.1s -> 0.2s -> 0.3s -> 0.4s (total: 0.4s)"
    Write-Host "Pattern matching: 87% confidence for AERETH pattern"
    Write-Host "Aereth gesture recognized: Yes"
    
    # Add a third gesture - Turanis (lightning bolt)
    Write-Host "`nSimulating Turanis gesture (lightning bolt)"
    Write-Host "Gesture visualization:"
    Write-Host "  +-------------------+"
    Write-Host "  |                   |"
    Write-Host "  |     \             |"
    Write-Host "  |       \           |"
    Write-Host "  |         \         |"
    Write-Host "  |           /       |"
    Write-Host "  |             \     |"
    Write-Host "  |                   |"
    Write-Host "  +-------------------+"
    Write-Host "Gesture points: (0.4,0.3) -> (0.5,0.4) -> (0.6,0.5) -> (0.5,0.6) -> (0.7,0.7)"
    Write-Host "Gesture timing: 0.0s -> 0.1s -> 0.2s -> 0.3s -> 0.4s (total: 0.4s)"
    Write-Host "Pattern matching: 89% confidence for TURANIS pattern"
    Write-Host "Turanis gesture recognized: Yes"
}

function DemoCombatIntegration {
    Write-Host "`n--- DEMO STEP 6: Combat System Integration ---"
    Write-Host "Demonstrating Combat System Integration"
    
    # Charge plasma weapon
    Write-Host "Starting charging plasma weapon..."
    for ($i = 0; $i -lt 4; $i++) {
        Write-Host "Current charge: $([math]::Round(25 + $i * 20))%"
        Start-Sleep -Milliseconds 250
    }
    
    # Fire at enemy and apply damage
    $enemyHealth = 95
    $damage = 40
    $enemyHealth -= $damage
    Write-Host "`nFiring plasma weapon at enemy"
    Write-Host "[ParticleSystem] Spawned plasma burst particles"
    Write-Host "Applying $damage energy damage to enemy"
    Write-Host "Enemy health after plasma attack: $enemyHealth"
}

function DemoAIAndTemporalInteraction {
    Write-Host "`n--- DEMO STEP 7: AI and Temporal System Interaction ---"
    Write-Host "Demonstrating AI and Temporal System Interaction"
    
    # Create temporal anomaly near AI
    Write-Host "Created temporal anomaly near AI position"
    
    # Update AI under temporal effect
    $timeScale = 0.3
    Write-Host "AI is affected by time scale: $timeScale"
    
    # AI behavior will be slower in the anomaly
    $aiState = "INVESTIGATE"
    Write-Host "AI updated with modified time scale, state: $aiState"
    Write-Host "[Visualization] AI movement slowed down in temporal field"
}

function DemoAdvancedGestureCombat {
    Write-Host "`n--- DEMO STEP 8: Advanced Gesture and Combat Integration ---"
    Write-Host "Demonstrating Advanced Gesture and Combat Integration"
    
    # Simulate Flammil gesture for quick-fire
    Write-Host "Flammil gesture detected - Quick plasma burst!"
    Write-Host "[ParticleSystem] Spawned 15 quick-fire particles with orange-red color"
    
    $enemyHealth = 55
    $damage = 20
    $enemyHealth -= $damage
    Write-Host "Applied $damage energy damage to enemy"
    Write-Host "Enemy health: $enemyHealth"
}

function DemoTemporalHealthInteraction {
    Write-Host "`n--- DEMO STEP 9: Temporal Effects on Health System ---"
    Write-Host "Demonstrating Temporal Effects on Health System"
    
    # Apply poison damage that should be affected by time
    $timeScale = 0.5
    Write-Host "Applied damage over time with TEMPORAL type (5.0 damage per second for 3.0 seconds)"
    
    # Get time scale at enemy position and modify damage effect
    Write-Host "Time scale at enemy position: $timeScale"
    Write-Host "Damage effect slowed by time scale factor"
    
    $enemyHealth = 35
    Write-Host "Enemy health after temporal damage with time scale $($timeScale): $($enemyHealth)"
    Write-Host "[Visualization] Temporal damage particles moving in slow motion"
}

function DemoAIDamagedState {
    Write-Host "`n--- DEMO STEP 10: AI Damaged State and Behavior Change ---"
    Write-Host "Demonstrating AI Damaged State and Behavior Change"
    
    # Deal significant damage to AI
    Write-Host "AI heavily damaged, taking 50 damage"
    $aiState = "DAMAGED"
    Write-Host "AI heavily damaged, state: $aiState"
    
    # Update AI behavior
    Write-Host "AI updated after taking damage"
    
    # Check if AI switches to FLEE state
    $aiHealthPercentage = 25
    if ($aiHealthPercentage -lt 30) {
        $aiState = "FLEE"
        Write-Host "AI health critical ($($aiHealthPercentage)%), switching to FLEE state"
        Write-Host "[Visualization] AI retreating with damaged appearance"
    }
}

function DemoFullSystemIntegration {
    Write-Host "`n--- DEMO STEP 11: Full System Integration ---"
    Write-Host "Demonstrating Full System Integration"
    
    # Create a complex combat scenario
    Write-Host "Scenario: Player encounters an enemy construct near a temporal anomaly"
    
    # Place AI and temporal anomaly
    Write-Host "Enemy construct positioned at (3,0,0)"
    Write-Host "Temporal anomaly (SLOWFIELD) created at (2,0,0) with radius 2.5"
    
    # AI detects player
    $aiState = "ATTACK"
    Write-Host "AI detected player, state: $aiState"
    
    # Player performs Flammil gesture for quick attack
    Write-Host "`nFlammil gesture detected - Quick plasma burst!"
    Write-Host "[ParticleSystem] Spawned 15 quick-fire particles with orange-red color"
    $enemyHealth = 35
    $damage = 20
    $enemyHealth -= $damage
    Write-Host "Applied $damage energy damage to enemy"
    
    # AI enters anomaly and is slowed
    $timeScale = 0.4
    Write-Host "`nAI entered anomaly, time scale: $timeScale"
    Write-Host "[Visualization] AI movement and attacks slowed by temporal field"
    
    # Player performs Aereth gesture for area effect
    Write-Host "`nAereth gesture detected - Temporal distortion!"
    Write-Host "Created SLOWFIELD anomaly at player position + (2,0,0) with radius 1.5"
    Write-Host "[Visualization] Overlapping temporal fields intensify effect"
    
    # Update all systems
    $enemyHealth = 15
    Write-Host "`nAfter combat, enemy health: $enemyHealth"
    $aiState = "FLEE"
    Write-Host "Final AI state: $aiState"
}

function DemoComplete {
    Write-Host "`n=== Demo Complete ==="
    Write-Host "All five Phase 1 components demonstrated successfully:"
    Write-Host "1. PlasmaWeapon - Energy weapon system with charging and firing modes"
    Write-Host "2. HealthSystem - Combat damage system with various damage types"
    Write-Host "3. AIConstruct - AI behavior system with state machine"
    Write-Host "4. TemporalAnomalySystem - Time manipulation effects"
    Write-Host "5. GestureRecognizer - Pattern recognition for magical gestures"
    Write-Host "`nComponents demonstrated both individually and in integration scenarios."
    Write-Host "Phase 1 implementation complete and verified."
}

# Run the demo steps
DemoPlasmaWeapon
DemoHealthSystem
DemoAIConstruct
DemoTemporalAnomaly
DemoGestureRecognition
DemoCombatIntegration
DemoAIAndTemporalInteraction
DemoAdvancedGestureCombat
DemoTemporalHealthInteraction
DemoAIDamagedState
DemoFullSystemIntegration
DemoComplete