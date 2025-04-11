# PowerShell script to run all standalone tests
Write-Host "==== Running All Component Tests ===="

$scriptDir = $PSScriptRoot
$components = @(
    @{
        Name = "HealthSystem";
        Script = "compile_test.ps1";
        ResultsFile = "test_results.txt";
    },
    @{
        Name = "GestureRecognizer";
        Script = "compile_gesture_test.ps1";
        ResultsFile = "test_results_gesture.txt";
    },
    @{
        Name = "PlasmaWeapon";
        Script = "compile_plasma_weapon_test.ps1";
        ResultsFile = "test_results_plasma_weapon.txt";
    },
    @{
        Name = "AIConstruct";
        Script = "compile_ai_construct_test.ps1";
        ResultsFile = "test_results_ai_construct.txt";
    },
    @{
        Name = "TemporalAnomalySystem";
        Script = "compile_temporal_test.ps1";
        ResultsFile = "test_results_temporal.txt";
    }
)

$results = @()

foreach ($component in $components) {
    Write-Host "Running test for $($component.Name)..."
    $testPath = Join-Path $scriptDir $component.Script
    
    # Run the test
    try {
        & $testPath
        
        # Read the result file
        $resultPath = Join-Path $scriptDir $component.ResultsFile
        $content = Get-Content $resultPath -Raw
        
        # Check if test was successful
        $success = $content -match "Compilation successful" -and $content -notmatch "Compilation failed"
        $passed = $content -match "PASSED" -and $content -notmatch "FAILED"
        
        $status = "FAILED"
        if ($success -and $passed) {
            $status = "PASSED"
        } elseif ($success) {
            $status = "COMPILED, TESTS FAILED"
        } else {
            $status = "COMPILATION FAILED"
        }
        
        $results += [PSCustomObject]@{
            Component = $component.Name
            Status = $status
        }
        
        Write-Host "$($component.Name) test completed with status: $status"
    } catch {
        Write-Host "Error running test for $($component.Name): $_"
        $results += [PSCustomObject]@{
            Component = $component.Name
            Status = "ERROR"
        }
    }
}

# Display summary
Write-Host "`n==== Test Summary ====" -ForegroundColor Cyan
$results | ForEach-Object {
    $color = "Red"
    if ($_.Status -eq "PASSED") { $color = "Green" }
    elseif ($_.Status -eq "COMPILED, TESTS FAILED") { $color = "Yellow" }
    
    Write-Host "$($_.Component): " -NoNewline
    Write-Host "$($_.Status)" -ForegroundColor $color
}

# Count success/failures
$passed = ($results | Where-Object { $_.Status -eq "PASSED" }).Count
$total = $results.Count

Write-Host "`nPassed $passed of $total component tests" -ForegroundColor $(if ($passed -eq $total) { "Green" } else { "Yellow" })

# Save summary to file
$summaryPath = Join-Path $scriptDir "test_summary.txt"
"==== Phase 1 Component Test Summary ====`n" | Out-File $summaryPath
$results | ForEach-Object {
    "$($_.Component): $($_.Status)" | Out-File $summaryPath -Append
}
"`nPassed $passed of $total component tests." | Out-File $summaryPath -Append

Write-Host "`nTest summary saved to: $summaryPath" 