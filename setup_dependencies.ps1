# PowerShell script to set up dependencies for Silent Forge Phase 1 Demo
Write-Host "==== Silent Forge: Dependencies Setup ===="

# Function to check if vcpkg is installed
function Check-VcpkgInstallation {
    $vcpkgPath = Join-Path $PSScriptRoot "vcpkg"
    
    if (Test-Path $vcpkgPath) {
        Write-Host "vcpkg found at: $vcpkgPath"
        return $vcpkgPath
    } else {
        Write-Host "vcpkg not found in the project directory."
        return $null
    }
}

# Function to install vcpkg if needed
function Install-Vcpkg {
    $vcpkgPath = Join-Path $PSScriptRoot "vcpkg"
    
    Write-Host "Installing vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git
    
    # Run the bootstrap script
    Set-Location vcpkg
    .\bootstrap-vcpkg.bat
    Set-Location ..
    
    Write-Host "vcpkg installed successfully at: $vcpkgPath"
    return $vcpkgPath
}

# Function to install dependencies using vcpkg
function Install-Dependencies {
    param (
        [string]$VcpkgPath
    )
    
    Write-Host "Installing required dependencies..."
    
    # Install GLM
    Write-Host "Installing GLM..."
    & "$VcpkgPath\vcpkg" install glm:x64-windows
    
    # Install OpenCV
    Write-Host "Installing OpenCV..."
    & "$VcpkgPath\vcpkg" install opencv:x64-windows
    
    # Integrate with Visual Studio
    Write-Host "Integrating vcpkg with Visual Studio..."
    & "$VcpkgPath\vcpkg" integrate install
    
    Write-Host "Dependencies installed successfully."
}

# Function to update CMakeLists.txt
function Update-CMakeLists {
    $cmakeListsPath = Join-Path $PSScriptRoot "CMakeLists.txt"
    
    if (Test-Path $cmakeListsPath) {
        Write-Host "Updating CMakeLists.txt to include dependencies..."
        
        $cmakeContent = Get-Content $cmakeListsPath
        
        # Check if the toolchain file is already set
        $toolchainLine = 'set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")'
        if ($cmakeContent -notcontains $toolchainLine) {
            # Add the toolchain file at the beginning
            $newContent = @($toolchainLine)
            $newContent += $cmakeContent
            Set-Content -Path $cmakeListsPath -Value $newContent
        }
        
        # Check if the find_package commands are already there
        $findGlm = 'find_package(glm CONFIG REQUIRED)'
        $findOpenCV = 'find_package(OpenCV REQUIRED)'
        
        $updatedContent = @()
        $glmAdded = $false
        $openCVAdded = $false
        
        foreach ($line in $cmakeContent) {
            $updatedContent += $line
            
            # Add find_package commands after the project() line
            if ($line -match "^project\(") {
                if ($cmakeContent -notcontains $findGlm) {
                    $updatedContent += ""
                    $updatedContent += "# Required dependencies"
                    $updatedContent += $findGlm
                    $glmAdded = $true
                }
                
                if ($cmakeContent -notcontains $findOpenCV) {
                    if (-not $glmAdded) {
                        $updatedContent += ""
                        $updatedContent += "# Required dependencies"
                    }
                    $updatedContent += $findOpenCV
                    $openCVAdded = $true
                }
                
                if ($glmAdded -or $openCVAdded) {
                    $updatedContent += ""
                }
            }
        }
        
        if ($glmAdded -or $openCVAdded) {
            Set-Content -Path $cmakeListsPath -Value $updatedContent
            Write-Host "CMakeLists.txt updated successfully."
        } else {
            Write-Host "CMakeLists.txt already contains dependency includes."
        }
    } else {
        Write-Host "CMakeLists.txt not found in the project directory."
    }
}

# Main script execution
Write-Host "Checking for vcpkg installation..."
$vcpkgPath = Check-VcpkgInstallation

if (-not $vcpkgPath) {
    $installVcpkg = Read-Host "vcpkg not found. Would you like to install it? (Y/N)"
    if ($installVcpkg -eq "Y" -or $installVcpkg -eq "y") {
        $vcpkgPath = Install-Vcpkg
    } else {
        Write-Host "vcpkg is required to install the dependencies. Exiting..."
        exit
    }
}

$installDeps = Read-Host "Would you like to install GLM and OpenCV dependencies? (Y/N)"
if ($installDeps -eq "Y" -or $installDeps -eq "y") {
    Install-Dependencies -VcpkgPath $vcpkgPath
} else {
    Write-Host "Skipping dependency installation."
}

$updateCMake = Read-Host "Would you like to update CMakeLists.txt to include the dependencies? (Y/N)"
if ($updateCMake -eq "Y" -or $updateCMake -eq "y") {
    Update-CMakeLists
} else {
    Write-Host "Skipping CMakeLists.txt update."
}

Write-Host "`n==== Dependency Setup Complete ===="
Write-Host "Next steps:"
Write-Host "1. Run 'cmake -B build -S . -G \"Visual Studio 17 2022\" -A x64' to generate project files"
Write-Host "2. Run 'cmake --build build --config Release' to build the project"
Write-Host "3. Once built, run the Phase 1 Demo with 'build\Release\SilentForge.exe'" 