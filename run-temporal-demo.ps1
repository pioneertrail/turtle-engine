# Run Temporal Anomaly Demo
# This script finds and runs one of the temporal anomaly demos (simple or improved)

$possiblePaths = @(
    ".\build\bin\Debug\SimpleTemporalDemo.exe",
    ".\build\bin\Release\SimpleTemporalDemo.exe",
    ".\bin\SimpleTemporalDemo.exe",
    ".\SimpleTemporalDemo.exe",
    ".\build\bin\Debug\ImprovedTemporalDemo.exe",
    ".\build\bin\Release\ImprovedTemporalDemo.exe",
    ".\bin\ImprovedTemporalDemo.exe",
    ".\ImprovedTemporalDemo.exe"
)

# Check if a specific demo was requested
param (
    [string]$demo = "simple" # Default to simple demo
)

# Filter the paths based on the requested demo
if ($demo -eq "simple") {
    $possiblePaths = $possiblePaths | Where-Object { $_ -like "*SimpleTemporalDemo*" }
    $demoName = "Simple Temporal Demo"
} elseif ($demo -eq "improved") {
    $possiblePaths = $possiblePaths | Where-Object { $_ -like "*ImprovedTemporalDemo*" }
    $demoName = "Improved Temporal Demo"
} else {
    Write-Host "Invalid demo specified: $demo. Using 'simple' as default."
    $possiblePaths = $possiblePaths | Where-Object { $_ -like "*SimpleTemporalDemo*" }
    $demoName = "Simple Temporal Demo"
}

# Find the first executable that exists
$exePath = $null
foreach ($path in $possiblePaths) {
    if (Test-Path $path) {
        $exePath = $path
        break
    }
}

# Run the demo if found
if ($null -eq $exePath) {
    Write-Host "No $demoName executable found. Make sure to build the project with -DSF_BUILD_TESTS=ON first." -ForegroundColor Red
    Write-Host "Expected paths:"
    foreach ($path in $possiblePaths) {
        Write-Host "  $path"
    }
    Write-Host ""
    Write-Host "You can build with: cmake --build build --config Debug" -ForegroundColor Yellow
} else {
    Write-Host "Running $demoName from: $exePath" -ForegroundColor Green
    & $exePath
}

# Enable ANSI color support for Windows terminal
if ($PSVersionTable.PSVersion.Major -ge 7) {
    # PowerShell 7+ has native ANSI color support
    # No need to do anything
} else {
    # For Windows PowerShell, try to enable VT processing
    try {
        # This only works if running in regular PowerShell console, not in VSCode integrated terminal
        $Host.UI.RawUI.WindowTitle = "Temporal Anomaly Demo"
        
        # Set console mode to enable ANSI processing
        Add-Type -AssemblyName System.Runtime.InteropServices
        $consoleHandle = [System.Runtime.InteropServices.Marshal]::GetStdHandle(-11) # STD_OUTPUT_HANDLE
        $mode = 0
        [void][System.Runtime.InteropServices.Marshal]::GetConsoleMode($consoleHandle, [ref]$mode)
        [void][System.Runtime.InteropServices.Marshal]::SetConsoleMode($consoleHandle, $mode -bor 4) # ENABLE_VIRTUAL_TERMINAL_PROCESSING
    } catch {
        Write-Host "Warning: Could not enable ANSI color support. Demo may not display colors correctly." -ForegroundColor Yellow
    }
}

Write-Host "`nDemo execution complete!" -ForegroundColor Green 