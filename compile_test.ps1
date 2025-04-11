# PowerShell script to compile HealthSystemTest
Write-Host "Setting up MSVC environment..."

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

# Create a simplified ParticleSystem.cpp with implementation
$mockImplFile = "mock_particle_system.cpp"
@"
#include "engine/graphics/include/ParticleSystem.hpp"
#include <iostream>

namespace TurtleEngine {
namespace Graphics {

    // Implement the methods that HealthSystem.cpp needs
    void ParticleSystem::emit(const glm::vec3& position, const glm::vec3& velocity,
                             const glm::vec4& color, float size, float life) {
        // Just a stub implementation for the test
        std::cout << "[ParticleSystem] Emitted particle" << std::endl;
    }

} // namespace Graphics
} // namespace TurtleEngine
"@ | Out-File -FilePath $mockImplFile -Encoding ASCII

# Create a temporary batch file to set up the environment and compile
$tempBatch = "temp_compile.bat"
@"
@echo off
call "$vcvarsPath"
cd /d "%CD%"
echo Compiling HealthSystemTest...
set "INCLUDE_DIRS=/I. /I.\vcpkg\installed\x64-windows\include /I.\src /I.\src\engine /I.\src\engine\graphics\include"
set "SOURCE_FILES=src\tests\standalone\HealthSystemTest.cpp src\engine\combat\src\HealthSystem.cpp $mockImplFile"
set "COMPILER_FLAGS=/nologo /EHsc /std:c++17 /DMOCK_IMPLEMENTATION"
cl %COMPILER_FLAGS% %INCLUDE_DIRS% %SOURCE_FILES% /Fe:HealthSystemTest.exe
if %ERRORLEVEL% EQU 0 (
  echo Compilation successful. Running test...
  HealthSystemTest.exe
) else (
  echo Compilation failed.
)
"@ | Out-File -FilePath $tempBatch -Encoding ASCII

# Run the temporary batch file
Write-Host "Running compilation..."
cmd /c $tempBatch

# Clean up
Remove-Item -Path $tempBatch -Force
Remove-Item -Path $mockImplFile -Force 