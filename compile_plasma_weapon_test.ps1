# PowerShell script to compile PlasmaWeaponStandaloneTest
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

# Create a mock ParticleSystem.hpp file
$mockParticleFile = "ParticleSystem.hpp"
@"
// Mock implementation of ParticleSystem for testing
#pragma once
#ifndef PARTICLE_SYSTEM_HPP
#define PARTICLE_SYSTEM_HPP

#include <memory>
#include <iostream>
#include <glm/glm.hpp>

namespace TurtleEngine {
    class ParticleSystem {
    public:
        ParticleSystem(size_t maxParticles = 10000) {}
        virtual ~ParticleSystem() = default;
        
        virtual void emit(const glm::vec3& position, const glm::vec3& velocity,
                         const glm::vec4& color, float size, float life) {
            std::cout << "[MockParticleSystem] Emitted particle at (" 
                     << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
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
"@ | Out-File -FilePath $mockParticleFile -Encoding ASCII

$tempBatch = "temp_compile_plasma_weapon.bat"
@"
@echo off
call "$vcvarsPath"
cd /d "%CD%"
echo Compiling PlasmaWeaponStandaloneTest...
set "INCLUDE_DIRS=/I. /I.\vcpkg\installed\x64-windows\include /I.\src /I.\src\engine /I.\src\engine\combat\include"
set "SOURCE_FILES=src\tests\standalone\PlasmaWeaponStandaloneTest.cpp src\engine\combat\src\PlasmaWeapon.cpp"
cl /nologo /EHsc /std:c++17 %INCLUDE_DIRS% %SOURCE_FILES% /Fe:PlasmaWeaponTestStandalone.exe /D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
if %ERRORLEVEL% EQU 0 (
  echo Compilation successful. Running test...
  PlasmaWeaponTestStandalone.exe
) else (
  echo Compilation failed.
)
"@ | Out-File -FilePath $tempBatch -Encoding ASCII

Write-Host "Running compilation..."
cmd /c $tempBatch > test_results_plasma_weapon.txt 2>&1

Remove-Item -Path $tempBatch -Force 
# Leave mock_particle_system.hpp for later use 