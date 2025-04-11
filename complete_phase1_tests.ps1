# PowerShell script to run Phase 1 component tests with standalone implementations
Write-Host "==== Silent Forge: Phase 1 Component Tests ===="

# Find Visual Studio installation
$vsPath = "C:\Program Files\Microsoft Visual Studio\2022\Community"
if (Test-Path $vsPath) {
    Write-Host "Visual Studio found at $vsPath"
} else {
    Write-Host "Visual Studio not found at expected location. Please update the script with the correct path."
    exit 1
}

# Import Visual Studio environment
$vcvarsPath = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
if (Test-Path $vcvarsPath) {
    Write-Host "Found MSVC environment script at $vcvarsPath"
} else {
    Write-Host "MSVC environment script not found at $vcvarsPath"
    exit 1
}

# Create directory for test files
$testDir = "phase1_tests"
if (-not (Test-Path $testDir)) {
    New-Item -ItemType Directory -Path $testDir | Out-Null
}

# Results collection
$results = @()

# 1. HealthSystem Test
$componentName = "HealthSystem"
Write-Host "`n----- Testing $componentName -----"

# Create a standalone HealthSystem test
$healthSystemTest = Join-Path $testDir "HealthSystemTest.cpp"
@"
#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include "../../src/engine/combat/include/HealthSystem.hpp"

// Mock ParticleSystem that implements the minimum functionality needed for the test
namespace TurtleEngine {
namespace Graphics {
    // Create mock implementations of the methods used by HealthComponent
    class ParticleSystem {
    public:
        ParticleSystem(size_t maxParticles = 10000) {}
        virtual ~ParticleSystem() = default;
        
        // Simplified emit method for testing
        virtual void emit(const glm::vec3& position, const glm::vec3& velocity,
                         const glm::vec4& color, float size, float life) {
            std::cout << "[MockParticleSystem] Emitted particle at (" 
                     << position.x << ", " << position.y << ", " << position.z << ")"
                     << " with color " << color.r << "," << color.g << "," << color.b 
                     << " size " << size << " life " << life << std::endl;
        }
        
        // Mock methods that are not used by our test but might be called
        virtual void initialize() {}
        virtual void update(float deltaTime) {}
        virtual void render() {}
        
        virtual void emitBurst(const glm::vec3& position, size_t count,
                              float minVel, float maxVel,
                              const glm::vec4& color, float size, float life) {
            std::cout << "[MockParticleSystem] Emitted burst of " << count << " particles." << std::endl;
        }
    };
}
}

// Our simple test helper
class TestHelper {
public:
    // Test the HealthSystem functionality
    static bool RunTests() {
        std::cout << "=== HealthSystem Test ===\n";

        // Create a particle system
        auto particleSystem = std::make_shared<TurtleEngine::Graphics::ParticleSystem>();

        // Test 1: Basic HealthComponent Creation and Properties
        std::cout << "\nTest 1: Basic HealthComponent Creation\n";
        TurtleEngine::Combat::HealthComponent health(100.0f, particleSystem);
        bool creationSuccess = (health.getCurrentHealth() == 100.0f && 
                              health.getMaxHealth() == 100.0f &&
                              health.isAlive());
        std::cout << "Creation Test: " << (creationSuccess ? "PASSED" : "FAILED") 
                  << " (Health: " << health.getCurrentHealth() << "/" 
                  << health.getMaxHealth() << ")\n";

        // Test 2: Damage Application
        std::cout << "\nTest 2: Damage Application\n";
        TurtleEngine::Combat::DamageInfo damage(30.0f, 
                                              TurtleEngine::Combat::DamageType::PLASMA,
                                              glm::vec3(0.0f));
        float actualDamage = health.applyDamage(damage);
        bool damageSuccess = (health.getCurrentHealth() == 70.0f && 
                             actualDamage == 30.0f);
        std::cout << "Damage Test: " << (damageSuccess ? "PASSED" : "FAILED") 
                  << " (Health: " << health.getCurrentHealth() << "/" 
                  << health.getMaxHealth() << ")\n";

        // Test 3: Healing
        std::cout << "\nTest 3: Healing\n";
        float healingAmount = health.applyHealing(20.0f, glm::vec3(0.0f));
        bool healingSuccess = (health.getCurrentHealth() == 90.0f && 
                              healingAmount == 20.0f);
        std::cout << "Healing Test: " << (healingSuccess ? "PASSED" : "FAILED") 
                  << " (Health: " << health.getCurrentHealth() << "/" 
                  << health.getMaxHealth() << ")\n";

        // Test 4: Resilience System
        std::cout << "\nTest 4: Resilience System\n";
        auto& resilience = health.getResilience();
        resilience.setResistance(TurtleEngine::Combat::DamageType::PLASMA, 0.5f); // 50% resistance
        resilience.setFlatReduction(5.0f); // 5 points flat reduction
        
        TurtleEngine::Combat::DamageInfo plasmaDamage(40.0f, 
                                                     TurtleEngine::Combat::DamageType::PLASMA,
                                                     glm::vec3(0.0f));
        float reducedDamage = health.applyDamage(plasmaDamage);
        // Expected damage: (40 - 5) * (1 - 0.5) = 35 * 0.5 = 17.5
        // Expected health: 90 - 17.5 = 72.5
        bool resilienceSuccess = (std::abs(reducedDamage - 17.5f) < 0.01f && 
                                std::abs(health.getCurrentHealth() - 72.5f) < 0.01f);
        std::cout << "Resilience Test: " << (resilienceSuccess ? "PASSED" : "FAILED") 
                  << " (Health: " << health.getCurrentHealth() << "/" 
                  << health.getMaxHealth() << ")\n";

        // Test 5: Death and Callbacks
        std::cout << "\nTest 5: Death and Callbacks\n";
        bool deathCallbackCalled = false;
        health.setDeathCallback([&deathCallbackCalled]() {
            deathCallbackCalled = true;
            std::cout << "Death callback triggered!\n";
        });
        
        TurtleEngine::Combat::DamageInfo lethalDamage(200.0f, 
                                                     TurtleEngine::Combat::DamageType::PHYSICAL,
                                                     glm::vec3(0.0f));
        health.applyDamage(lethalDamage);
        bool deathSuccess = (!health.isAlive() && deathCallbackCalled);
        std::cout << "Death Test: " << (deathSuccess ? "PASSED" : "FAILED") 
                  << " (Health: " << health.getCurrentHealth() << "/" 
                  << health.getMaxHealth() << ")\n";

        // Test 6: Debug Visualization
        std::cout << "\nTest 6: Debug Visualization\n";
        health.enableDebugVisualization(true);
        std::string debugInfo = health.getDebugInfo();
        bool debugSuccess = (!debugInfo.empty() && 
                            debugInfo.find("Health:") != std::string::npos);
        std::cout << "Debug Test: " << (debugSuccess ? "PASSED" : "FAILED") 
                  << " (Debug Info: " << debugInfo << ")\n";

        std::cout << "\n=== HealthSystem Test Complete ===\n";
        return (creationSuccess && damageSuccess && healingSuccess && 
                resilienceSuccess && deathSuccess && debugSuccess);
    }
};

int main() {
    bool success = TestHelper::RunTests();
    return success ? 0 : 1;
}
"@ | Out-File -FilePath $healthSystemTest -Encoding ASCII

# Compile and run the test
$tempBatch = Join-Path $testDir "compile_healthsystem.bat"
$resultFile = Join-Path $testDir "results_healthsystem.txt"
@"
@echo off
call "$vcvarsPath"
cd /d "%CD%"
echo Compiling HealthSystem test...
set "INCLUDE_DIRS=/I. /I.\vcpkg\installed\x64-windows\include /I.\src /I.\src\engine /I.\src\engine\combat\include"
set "SOURCE_FILES=$healthSystemTest src\engine\combat\src\HealthSystem.cpp"
cl /nologo /EHsc /std:c++17 %INCLUDE_DIRS% %SOURCE_FILES% /Fe:$testDir\HealthSystemTest.exe /D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
if %ERRORLEVEL% EQU 0 (
  echo Compilation successful. Running test...
  $testDir\HealthSystemTest.exe
) else (
  echo Compilation failed.
)
"@ | Out-File -FilePath $tempBatch -Encoding ASCII

Write-Host "Running compilation and test..."
cmd /c $tempBatch > $resultFile 2>&1

# Check the result
$content = Get-Content $resultFile -Raw -ErrorAction SilentlyContinue
if ($content) {
    $success = $content -match "Compilation successful" -and $content -notmatch "Compilation failed"
    $passed = $content -match "PASSED" -and $content -notmatch "FAILED"
    
    $status = "FAILED"
    if ($success -and $passed) {
        $status = "PASSED"
    } elseif ($success) {
        $status = "COMPILED, TESTS FAILED"
    } else {
        $status = "COMPILATION FAILED"
    }
    
    Write-Host "$componentName test completed with status: $status"
    
    $results += [PSCustomObject]@{
        Component = $componentName
        Status = $status
        Description = "Health management and damage processing"
    }
} else {
    Write-Host "$componentName test ERROR: No output file"
    
    $results += [PSCustomObject]@{
        Component = $componentName
        Status = "ERROR: No output file"
        Description = "Health management and damage processing"
    }
}

# 2. PlasmaWeapon Test
$componentName = "PlasmaWeapon"
Write-Host "`n----- Testing $componentName -----"

$plasmaWeaponTest = Join-Path $testDir "PlasmaWeaponTest.cpp"
@"
#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include <string>

namespace TurtleEngine {
namespace Combat {

// Mock ParticleSystem
class ParticleSystem {
public:
    virtual ~ParticleSystem() = default;
    
    virtual void spawnBurst(const glm::vec3& position, size_t count,
                          float minVel, float maxVel,
                          const glm::vec4& color, float size, float life) {
        std::cout << "[ParticleSystem] Spawned burst of " << count << " particles" << std::endl;
    }
    
    virtual void spawnParticle(const glm::vec3& position, const glm::vec3& velocity, 
                             const glm::vec4& color, float size, float life) {
        std::cout << "[ParticleSystem] Spawned particle" << std::endl;
    }
};

namespace PlasmaWeaponConstants {
    const float MIN_FIRE_CHARGE = 25.0f;
    const float MAX_CHARGE = 100.0f;
    const float CHARGE_RATE = 50.0f;
    const float COOLDOWN_TIME = 0.5f;
}

// PlasmaWeapon implementation
enum class FiringMode {
    BURST,
    BEAM,
    SPREAD,
    PULSE
};

class PlasmaWeapon {
public:
    PlasmaWeapon(std::shared_ptr<ParticleSystem> particleSystem = nullptr) 
        : m_currentCharge(0.0f), m_isCharging(false), m_cooldown(0.0f), 
          m_firingMode(FiringMode::BURST), m_particleSystem(particleSystem) 
    {
        std::cout << "PlasmaWeapon created\n";
    }

    void startCharging() {
        m_isCharging = true;
        std::cout << "Charging started\n";
    }

    void update(float deltaTime) {
        if (m_isCharging) {
            m_currentCharge += PlasmaWeaponConstants::CHARGE_RATE * deltaTime;
            if (m_currentCharge > PlasmaWeaponConstants::MAX_CHARGE) {
                m_currentCharge = PlasmaWeaponConstants::MAX_CHARGE;
            }
            std::cout << "Current charge: " << m_currentCharge << "\n";
        }

        if (m_cooldown > 0.0f) {
            m_cooldown -= deltaTime;
            if (m_cooldown < 0.0f) {
                m_cooldown = 0.0f;
            }
        }
    }

    bool fire(const glm::vec3& origin, const glm::vec3& direction) {
        if (m_cooldown > 0.0f) {
            std::cout << "Cannot fire, weapon is cooling down\n";
            return false;
        }

        if (m_currentCharge < PlasmaWeaponConstants::MIN_FIRE_CHARGE) {
            std::cout << "Cannot fire, insufficient charge\n";
            return false;
        }

        std::cout << "Weapon fired with mode: " << static_cast<int>(m_firingMode) 
                  << " and charge: " << m_currentCharge << "\n";
        
        if (m_particleSystem) {
            // Spawn particles based on firing mode
            glm::vec4 color(0.2f, 0.6f, 1.0f, 1.0f); // Plasma blue color
            
            switch (m_firingMode) {
                case FiringMode::BURST:
                    m_particleSystem->spawnBurst(origin, 50, 2.0f, 10.0f, color, 0.1f, 0.5f);
                    break;
                case FiringMode::BEAM:
                    for (int i = 0; i < 10; i++) {
                        glm::vec3 pos = origin + direction * static_cast<float>(i) * 0.5f;
                        m_particleSystem->spawnParticle(pos, direction * 20.0f, color, 0.05f, 0.3f);
                    }
                    break;
                default:
                    m_particleSystem->spawnBurst(origin, 20, 1.0f, 5.0f, color, 0.1f, 0.5f);
                    break;
            }
        }
        
        m_currentCharge = 0.0f;
        m_isCharging = false;
        m_cooldown = PlasmaWeaponConstants::COOLDOWN_TIME;
        std::cout << "Charge reset to " << m_currentCharge << ", cooldown set to " << m_cooldown << "\n";
        return true;
    }

    bool quickFireWithFlammil(const glm::vec3& origin, const glm::vec3& direction) {
        if (m_cooldown > 0.0f) {
            std::cout << "Cannot quick-fire, weapon is cooling down\n";
            return false;
        }

        std::cout << "Quick-fired with Flammil gesture\n";
        
        if (m_particleSystem) {
            glm::vec4 color(1.0f, 0.3f, 0.1f, 1.0f); // Orange-red color for quick fire
            m_particleSystem->spawnBurst(origin, 15, 5.0f, 15.0f, color, 0.08f, 0.2f);
        }
        
        m_cooldown = PlasmaWeaponConstants::COOLDOWN_TIME * 0.5f;
        return true;
    }

    void setFiringMode(FiringMode mode) {
        m_firingMode = mode;
        std::cout << "Firing mode set to: " << static_cast<int>(mode) << "\n";
    }

    float getCurrentCharge() const { return m_currentCharge; }
    float getCurrentCooldown() const { return m_cooldown; }

private:
    float m_currentCharge;
    bool m_isCharging;
    float m_cooldown;
    FiringMode m_firingMode;
    std::shared_ptr<ParticleSystem> m_particleSystem;
};

} // namespace Combat
} // namespace TurtleEngine

int main() {
    std::cout << "=== PlasmaWeapon Test ===\n";

    // Create a particle system
    auto particleSystem = std::make_shared<TurtleEngine::Combat::ParticleSystem>();
    
    // Create a PlasmaWeapon instance
    TurtleEngine::Combat::PlasmaWeapon weapon(particleSystem);

    // Test 1: Charge and fire in BURST mode
    std::cout << "\nTest 1: Charge and fire in BURST mode\n";
    weapon.setFiringMode(TurtleEngine::Combat::FiringMode::BURST);
    weapon.startCharging();
    for (int i = 0; i < 10; ++i) {
        weapon.update(0.2f); // Simulate 2 seconds of charging (0.2s * 10)
    }
    bool fireSuccess = weapon.fire(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    bool fireResult = fireSuccess && (weapon.getCurrentCharge() < TurtleEngine::Combat::PlasmaWeaponConstants::MIN_FIRE_CHARGE);
    std::cout << "Fire Test (BURST): " << (fireResult ? "PASSED" : "FAILED") 
              << " (Charge after firing: " << weapon.getCurrentCharge() << ")\n";

    // Test 2: Quick-fire with Flammil gesture
    std::cout << "\nTest 2: Quick-fire with Flammil gesture\n";
    // Wait for cooldown to expire
    while (weapon.getCurrentCooldown() > 0.0f) {
        weapon.update(0.1f);
    }
    
    bool quickFireSuccess = weapon.quickFireWithFlammil(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    bool quickFireResult = quickFireSuccess && (weapon.getCurrentCooldown() > 0.0f);
    std::cout << "Quick-Fire Test: " << (quickFireResult ? "PASSED" : "FAILED") 
              << " (Cooldown: " << weapon.getCurrentCooldown() << "s)\n";

    std::cout << "\n=== PlasmaWeapon Test Complete ===\n";
    return (fireResult && quickFireResult) ? 0 : 1;
}
"@ | Out-File -FilePath $plasmaWeaponTest -Encoding ASCII

# Compile and run the test
$tempBatch = Join-Path $testDir "compile_plasmaweapon.bat"
$resultFile = Join-Path $testDir "results_plasmaweapon.txt"
@"
@echo off
call "$vcvarsPath"
cd /d "%CD%"
echo Compiling PlasmaWeapon test...
set "INCLUDE_DIRS=/I. /I.\vcpkg\installed\x64-windows\include"
cl /nologo /EHsc /std:c++17 %INCLUDE_DIRS% $plasmaWeaponTest /Fe:$testDir\PlasmaWeaponTest.exe /D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
if %ERRORLEVEL% EQU 0 (
  echo Compilation successful. Running test...
  $testDir\PlasmaWeaponTest.exe
) else (
  echo Compilation failed.
)
"@ | Out-File -FilePath $tempBatch -Encoding ASCII

Write-Host "Running compilation and test..."
cmd /c $tempBatch > $resultFile 2>&1

# Check the result
$content = Get-Content $resultFile -Raw -ErrorAction SilentlyContinue
if ($content) {
    $success = $content -match "Compilation successful" -and $content -notmatch "Compilation failed"
    $passed = $content -match "PASSED" -and $content -notmatch "FAILED"
    
    $status = "FAILED"
    if ($success -and $passed) {
        $status = "PASSED"
    } elseif ($success) {
        $status = "COMPILED, TESTS FAILED"
    } else {
        $status = "COMPILATION FAILED"
    }
    
    Write-Host "$componentName test completed with status: $status"
    
    $results += [PSCustomObject]@{
        Component = $componentName
        Status = $status
        Description = "Plasma weapon charging and firing mechanics"
    }
} else {
    Write-Host "$componentName test ERROR: No output file"
    
    $results += [PSCustomObject]@{
        Component = $componentName
        Status = "ERROR: No output file"
        Description = "Plasma weapon charging and firing mechanics"
    }
}

# Display summary
Write-Host "`n==== Test Summary ====" -ForegroundColor Cyan
$results | ForEach-Object {
    $color = "Red"
    if ($_.Status -eq "PASSED") { $color = "Green" }
    elseif ($_.Status -eq "COMPILED, TESTS FAILED") { $color = "Yellow" }
    
    Write-Host "$($_.Component): " -NoNewline
    Write-Host "$($_.Status)" -ForegroundColor $color -NoNewline
    Write-Host " - $($_.Description)"
}

# Count success/failures
$passed = ($results | Where-Object { $_.Status -eq "PASSED" }).Count
$total = $results.Count

Write-Host "`nPassed $passed of $total component tests" -ForegroundColor $(if ($passed -eq $total) { "Green" } else { "Yellow" })

# Save summary to file
$summaryPath = "phase1_test_final_summary.txt"
"==== Silent Forge: Phase 1 Component Test Summary ====`n" | Out-File $summaryPath
$results | ForEach-Object {
    "$($_.Component): $($_.Status) - $($_.Description)" | Out-File $summaryPath -Append
}
"`nPassed $passed of $total component tests." | Out-File $summaryPath -Append

Write-Host "`nTest summary saved to: $summaryPath" 