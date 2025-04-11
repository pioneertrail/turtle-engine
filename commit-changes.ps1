# PowerShell script to commit workflow and demo enhancements

# Create a new branch
git checkout -b docs/workflow-enhancements

# Stage all changed files
git add .github/CODEOWNERS
git add .github/workflows/pr-validation.yml
git add .github/workflows/release.yml
git add docs/technical/workflow.md
git add docs/notifications/version-control-update.md
git add docs/pr-description.md
git add run_demo.ps1
git add docs/demo.md
git add setup_dependencies.ps1
git add CMakeLists.txt

# Create commit
git commit -m "docs(ci): add CODEOWNERS, enhance workflow with CI/CD, and complete Phase 1 demo"

# Instructions for manual steps
Write-Host "Changes committed to branch 'docs/workflow-enhancements'"
Write-Host ""
Write-Host "To push and create PR, run the following commands:"
Write-Host "git push origin docs/workflow-enhancements"
Write-Host "gh pr create --base development --head docs/workflow-enhancements --title 'Add CODEOWNERS and Enhance Workflow Documentation' --body-file docs/pr-description.md" 