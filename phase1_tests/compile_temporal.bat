@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd /d "%CD%"
echo Compiling TemporalAnomalySystem test...
set "INCLUDE_DIRS=/I. /I.\vcpkg\installed\x64-windows\include"
cl /nologo /EHsc /std:c++17 %INCLUDE_DIRS% phase1_tests\TemporalAnomalyTest.cpp /Fe:phase1_tests\TemporalAnomalyTest.exe /DGLM_ENABLE_EXPERIMENTAL /D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
if %ERRORLEVEL% EQU 0 (
  echo Compilation successful. Running test...
  phase1_tests\TemporalAnomalyTest.exe
) else (
  echo Compilation failed.
)
