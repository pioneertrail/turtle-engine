@echo off
echo Compiling HealthSystemTest...

set "INCLUDE_DIRS=/I.\vcpkg\installed\x64-windows\include"
set "SOURCE_FILES=src\tests\standalone\HealthSystemTest.cpp src\engine\combat\src\HealthSystem.cpp"

cl /nologo /EHsc /std:c++17 %INCLUDE_DIRS% %SOURCE_FILES% /Fe:HealthSystemTest.exe

if %ERRORLEVEL% EQU 0 (
  echo Compilation successful. Running test...
  HealthSystemTest.exe
) else (
  echo Compilation failed.
) 