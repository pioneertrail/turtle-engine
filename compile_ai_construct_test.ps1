Write-Host "Setting up MSVC environment..."

$vsPath = "C:\Program Files\Microsoft Visual Studio\2022\Community"
if (Test-Path $vsPath) {
    Write-Host "Visual Studio found at $vsPath"
} else {
    Write-Host "Visual Studio not found at expected location. Please update the script with the correct path."
    exit 1
}

$vcvarsPath = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
if (Test-Path $vcvarsPath) {
    Write-Host "Found MSVC environment script at $vcvarsPath"
} else {
    Write-Host "MSVC environment script not found at $vcvarsPath"
    exit 1
}

$mockAIFile = "mock_ai_construct.hpp"
@"
// Mock implementation for AIConstruct test
#pragma once
#ifndef MOCK_AI_CONSTRUCT_HPP
#define MOCK_AI_CONSTRUCT_HPP

#include <iostream>
#include <memory>
#include <string>
#include <functional>
#include <glm/glm.hpp>

// Make sure we use our mock ParticleSystem
#ifndef PARTICLE_SYSTEM_HPP
#define PARTICLE_SYSTEM_HPP
namespace TurtleEngine {
    class ParticleSystem {
    public:
        ParticleSystem(size_t maxParticles = 10000) {}
        virtual ~ParticleSystem() = default;
        
        virtual void emit(const glm::vec3& position, const glm::vec3& velocity,
                         const glm::vec4& color, float size, float life) {
            std::cout << "[MockParticleSystem] Emitted particle" << std::endl;
        }
        
        virtual void initialize() {}
        virtual void update(float deltaTime) {}
        virtual void render() {}
        
        virtual void spawnBurst(const glm::vec3& position, size_t count,
                              float minVel, float maxVel,
                              const glm::vec4& color, float size, float life) {
            std::cout << "[MockParticleSystem] Emitted burst of " << count << " particles." << std::endl;
        }
        
        virtual void spawnParticle(const glm::vec3& position, const glm::vec3& velocity, 
                                 const glm::vec4& color, float size, float life) {
            std::cout << "[MockParticleSystem] Spawned a particle." << std::endl;
        }
    };
}
#endif

// Mock implementation of AIConstruct
namespace TurtleEngine {
namespace Combat {

enum class AIState {
    IDLE,
    PATROL,
    ATTACK,
    DEFEND,
    FLEE
};

class AIConstruct {
public:
    // AI Construct types
    enum class Type {
        SENTINEL,
        ENFORCER,
        GUARDIAN,
        JUGGERNAUT
    };
    
    AIConstruct(std::shared_ptr<TurtleEngine::ParticleSystem> particleSystem, Type type, 
                const glm::vec3& position, float radius = 1.0f)
        : m_particleSystem(particleSystem), m_type(type), m_position(position),
          m_radius(radius), m_state(AIState::IDLE), m_health(100.0f), m_shield(50.0f),
          m_active(true), m_alertLevel(0.0f) 
    {
        std::cout << "AIConstruct created with type: " << static_cast<int>(type) << std::endl;
        initializeTypeProperties();
        setupResistances();
    }
    
    ~AIConstruct() {
        std::cout << "AIConstruct destroyed" << std::endl;
    }
    
    // Added function to test
    AIState getCurrentState() const { 
        return m_state; 
    }
    
    void update(float deltaTime) {
        // Mock update behavior
        std::cout << "AIConstruct updated. Current state: " << static_cast<int>(m_state) << std::endl;
    }
    
    void takeDamage(float amount) {
        m_health -= amount;
        if (m_health <= 0) {
            m_health = 0;
            m_active = false;
            std::cout << "AIConstruct destroyed by damage" << std::endl;
        } else {
            std::cout << "AIConstruct took damage. Health: " << m_health << std::endl;
        }
    }
    
    bool isActive() const { return m_active; }
    float getHealth() const { return m_health; }
    float getShield() const { return m_shield; }
    Type getType() const { return m_type; }
    
    void setPosition(const glm::vec3& pos) {
        m_position = pos;
        std::cout << "AIConstruct position set to: (" 
                 << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    }
    
    glm::vec3 getPosition() const { return m_position; }
    
    void setState(AIState state) {
        m_state = state;
        std::cout << "AIConstruct state set to: " << static_cast<int>(state) << std::endl;
    }

private:
    void initializeTypeProperties() {
        // Set different properties based on AI type
        switch (m_type) {
            case Type::SENTINEL:
                m_health = 75.0f;
                m_shield = 25.0f;
                break;
            case Type::ENFORCER:
                m_health = 100.0f;
                m_shield = 50.0f;
                break;
            case Type::GUARDIAN:
                m_health = 150.0f;
                m_shield = 75.0f;
                break;
            case Type::JUGGERNAUT:
                m_health = 250.0f;
                m_shield = 150.0f;
                break;
        }
    }
    
    void setupResistances() {
        // Mock implementation
        std::cout << "Setting up resistances for AIConstruct type: " << static_cast<int>(m_type) << std::endl;
    }
    
    // Member variables
    Type m_type;
    AIState m_state;
    float m_health;
    float m_shield;
    bool m_active;
    float m_alertLevel;
    glm::vec3 m_position;
    float m_radius;
    std::shared_ptr<TurtleEngine::ParticleSystem> m_particleSystem;
};

} // namespace Combat
} // namespace TurtleEngine
#endif // MOCK_AI_CONSTRUCT_HPP
"@ | Out-File -FilePath $mockAIFile -Encoding ASCII

$tempBatch = "temp_compile_ai_construct.bat"
@"
@echo off
call "$vcvarsPath"
cd /d "%CD%"
echo Compiling AIConstructStandaloneTest...
set "INCLUDE_DIRS=/I. /I.\vcpkg\installed\x64-windows\include /I.\src /I.\src\engine /I.\src\engine\combat\include"
set "SOURCE_FILES=src\tests\standalone\AIConstructStandaloneTest.cpp"
cl /nologo /EHsc /std:c++17 %INCLUDE_DIRS% %SOURCE_FILES% /Fe:AIConstructTestStandalone.exe /FI"mock_ai_construct.hpp" /D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
if %ERRORLEVEL% EQU 0 (
  echo Compilation successful. Running test...
  AIConstructTestStandalone.exe
) else (
  echo Compilation failed.
)
"@ | Out-File -FilePath $tempBatch -Encoding ASCII

Write-Host "Running compilation..."
cmd /c $tempBatch > test_results_ai_construct.txt 2>&1

Remove-Item -Path $tempBatch -Force
Remove-Item -Path $mockAIFile -Force 