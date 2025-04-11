# Script to clean and reset vcpkg dependencies
Write-Host "Cleaning and resetting vcpkg dependencies..."

# Navigate to project root directory
$projectRoot = $PSScriptRoot

# Check if vcpkg exists
if (-Not (Test-Path "$projectRoot\vcpkg")) {
    Write-Host "vcpkg directory not found. Cloning fresh copy..."
    
    # Clone vcpkg repository
    git clone https://github.com/microsoft/vcpkg.git
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Failed to clone vcpkg. Make sure git is installed."
        exit 1
    }
} else {
    Write-Host "Found existing vcpkg directory."
    
    # Check if triplet directories exist to delete them without removing the entire vcpkg folder
    if (Test-Path "$projectRoot\vcpkg\installed") {
        Write-Host "Removing installed packages..."
        Remove-Item -Recurse -Force "$projectRoot\vcpkg\installed"
    }
    
    if (Test-Path "$projectRoot\vcpkg\packages") {
        Write-Host "Removing package cache..."
        Remove-Item -Recurse -Force "$projectRoot\vcpkg\packages"
    }
    
    if (Test-Path "$projectRoot\vcpkg\buildtrees") {
        Write-Host "Removing build trees..."
        Remove-Item -Recurse -Force "$projectRoot\vcpkg\buildtrees"
    }
}

# Navigate to vcpkg directory
Push-Location "$projectRoot\vcpkg"

# Bootstrap vcpkg (rebuilds vcpkg executable)
Write-Host "Bootstrapping vcpkg..."
.\bootstrap-vcpkg.bat
if ($LASTEXITCODE -ne 0) {
    Write-Host "Failed to bootstrap vcpkg."
    Pop-Location
    exit 1
}

# Integrate vcpkg with Visual Studio
Write-Host "Integrating vcpkg with Visual Studio..."
.\vcpkg integrate install
if ($LASTEXITCODE -ne 0) {
    Write-Host "Warning: Failed to integrate vcpkg with Visual Studio. Visual Studio may not be installed."
}

# Install required dependencies
Write-Host "Installing GLM..."
.\vcpkg install glm:x64-windows

Write-Host "Installing OpenCV..."
.\vcpkg install opencv:x64-windows

# Return to original directory
Pop-Location

Write-Host "vcpkg dependencies reset and installed successfully."
Write-Host "You can now build your project using Visual Studio or the 'build-with-msvc.ps1' script." 