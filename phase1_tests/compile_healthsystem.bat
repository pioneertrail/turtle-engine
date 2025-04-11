@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd /d "%CD%"
echo Compiling HealthSystem test...
set "INCLUDE_DIRS=/I. /I.\vcpkg\installed\x64-windows\include /I..\vcpkg\installed\x64-windows\include"
cl /nologo /EHsc /std:c++17 %INCLUDE_DIRS% HealthSystemTest.cpp /Fe:HealthSystemTest.exe /D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING /DGLM_ENABLE_EXPERIMENTAL
if %ERRORLEVEL% EQU 0 (
  echo Compilation successful. Running test...
  HealthSystemTest.exe
) else (
  echo Compilation failed.
)
