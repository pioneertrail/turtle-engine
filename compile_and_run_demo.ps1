# PowerShell script to compile and run Silent Forge Phase 1 Demo

Write-Host "==== Silent Forge: Phase 1 Demo Compiler ===="

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

# Create build directory if it doesn't exist
$buildDir = "build"
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
    Write-Host "Created build directory"
}

# Set up environment
Write-Host "Setting up MSVC build environment..."
$envSetup = "cmd.exe /c `"$vcvarsPath`" && set"
$env_vars = cmd /c "$envSetup"

foreach ($line in $env_vars) {
    if ($line -match "^(.*?)=(.*)$") {
        $name = $matches[1]
        $value = $matches[2]
        Set-Item -Path "env:$name" -Value "$value"
    }
}

# Run CMake to generate project files
Write-Host "Running CMake..."
$cmakeCmd = "cmake -B build -S . -G `"Visual Studio 17 2022`" -A x64"
Invoke-Expression $cmakeCmd

# Build the project
Write-Host "Building project..."
$buildCmd = "cmake --build build --config Release"
Invoke-Expression $buildCmd

# Check if build was successful
if ($LASTEXITCODE -eq 0) {
    Write-Host "Build successful! Running the demo..."
    
    # Run the demo
    $exePath = Join-Path $buildDir "Release\SilentForge.exe"
    if (Test-Path $exePath) {
        Write-Host "Starting Silent Forge Phase 1 Demo..."
        & $exePath
    } else {
        Write-Host "Error: Could not find the compiled executable at $exePath"
        exit 1
    }
} else {
    Write-Host "Build failed with exit code $LASTEXITCODE"
    exit 1
} 