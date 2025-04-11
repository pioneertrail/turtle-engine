#!/usr/bin/env pwsh
# Launcher script for dependency setup
$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Definition
$setupScript = Join-Path $scriptPath "scripts\build\setup_dependencies.ps1"
& $setupScript @args
