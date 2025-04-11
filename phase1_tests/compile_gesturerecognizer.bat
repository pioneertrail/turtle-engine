@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd /d "%CD%"
echo Compiling GestureRecognizer test...
set "INCLUDE_DIRS=/I. /I.\vcpkg\installed\x64-windows\include /I..\vcpkg\installed\x64-windows\include"
cl /nologo /EHsc /std:c++17 %INCLUDE_DIRS% GestureRecognizerTest.cpp /Fe:GestureRecognizerTest.exe /D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING /DGLM_ENABLE_EXPERIMENTAL
if %ERRORLEVEL% EQU 0 (
  echo Compilation successful. Running test...
  GestureRecognizerTest.exe
) else (
  echo Compilation failed.
) 