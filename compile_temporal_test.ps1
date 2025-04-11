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

$tempBatch = "temp_compile_temporal.bat"
@"
@echo off
call "$vcvarsPath"
cd /d "%CD%"
echo Compiling TemporalAnomalySystemStandaloneTest...
set "INCLUDE_DIRS=/I. /I.\vcpkg\installed\x64-windows\include /I.\src /I.\src\engine /I.\src\engine\temporal\include"
set "SOURCE_FILES=src\tests\standalone\TemporalAnomalySystemStandaloneTest.cpp src\engine\temporal\src\TemporalAnomalySystem.cpp"
cl /nologo /EHsc /std:c++17 %INCLUDE_DIRS% %SOURCE_FILES% /Fe:TemporalTestStandalone.exe /DGLM_ENABLE_EXPERIMENTAL /D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
if %ERRORLEVEL% EQU 0 (
  echo Compilation successful. Running test...
  TemporalTestStandalone.exe
) else (
  echo Compilation failed.
)
"@ | Out-File -FilePath $tempBatch -Encoding ASCII

Write-Host "Running compilation..."
cmd /c $tempBatch > test_results_temporal.txt 2>&1

Remove-Item -Path $tempBatch -Force 