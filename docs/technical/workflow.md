# GitHub Workflow & Version Control

## Overview
This document outlines the version control workflow and best practices for the Silent Forge: Rebellion of Aethelgard project. It establishes standardized procedures for code contributions, reviews, and releases to maintain code quality and project stability.

## Branch Strategy

### Main Branches
- **main**: Production-ready code, always stable
- **development**: Integration branch for completed features
- **release/vX.Y.Z**: Release preparation branches

### Working Branches
- **feature/XXX**: New features (e.g., `feature/plasma-weapon-scatter`)
- **bugfix/XXX**: Bug fixes (e.g., `bugfix/health-calculation`)
- **hotfix/XXX**: Critical fixes for production (e.g., `hotfix/crash-on-startup`)
- **docs/XXX**: Documentation updates (e.g., `docs/combat-system`)
- **refactor/XXX**: Code refactoring (e.g., `refactor/ai-state-machine`)
- **test/XXX**: Test additions or modifications (e.g., `test/plasma-weapon-tests`)

### Branch Retention Policy
- Merged feature, bugfix, docs, and refactor branches are deleted 7 days after merging
- Release branches are retained for historical reference
- Stale branches (no commits for 30+ days) should be reviewed for deletion

### Branch Flow
```
              hotfix/XXX
                  │
                  ▼
main ────────────►──────────────────────────►───────
 │                                             ▲
 │                                             │
 └───► development ─────────────────────────►──┘
         │  ▲
         │  │
    feature/XXX
    bugfix/XXX
    docs/XXX
    refactor/XXX
    test/XXX
```

## Commit Message Guidelines

### Format
```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types
- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation changes
- **style**: Formatting, missing semicolons, etc; no code change
- **refactor**: Code refactoring
- **test**: Adding tests, refactoring tests
- **perf**: Performance improvements
- **build**: Build system changes
- **ci**: CI configuration changes

### Scope
The system or component affected (e.g., `combat`, `ai`, `health`, `temporal`)

### Message Length Guidelines
- Subject line: Maximum 50 characters
- Body lines: Maximum 72 characters per line
- Keep messages concise but descriptive

### Examples
```
feat(combat): add scatter firing mode to plasma weapons

Implement scatter mode that fires multiple projectiles in a cone pattern.
- Add particle spread calculation
- Implement damage distribution across particles
- Add visual feedback for scatter mode

Resolves: #142
```

```
fix(health): correct damage calculation for shield penetration

The shield penetration calculation was applying resistance twice, resulting in
lower damage than intended.

Fixes: #157
```

## Pull Request Process

### PR Creation
1. Create branch from appropriate base (usually `development`)
2. Implement changes with descriptive commits
3. Run local tests to verify changes
4. Push branch to remote
5. Create PR with detailed description

### PR Template
```markdown
## Description
Briefly describe the changes.

## Related Issue
Link to the related issue.

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Performance improvement
- [ ] Code refactoring
- [ ] Test update
- [ ] Build/CI change
- [ ] Other (please describe)

## Testing Performed
Describe testing done for this change.

## Screenshots (if appropriate)
Add screenshots to help explain the change.

## Checklist
- [ ] My code follows the project style guidelines
- [ ] I have updated documentation accordingly
- [ ] I have added tests for new functionality
- [ ] All tests pass locally
- [ ] I have updated CHANGELOG.md
```

### Review Requirements
- At least one approval from a code owner
- All CI checks passing
- No merge conflicts with target branch
- All discussion items resolved
- Reviews should be completed within 48 hours (24 hours for hotfixes)

### Merge Strategy
- Feature branches: Squash and merge
- Development → Main: Create merge commit

## Versioning

### Version Format
We follow [Semantic Versioning](https://semver.org/): `MAJOR.MINOR.PATCH`

- **MAJOR**: Incompatible API changes
- **MINOR**: Backward-compatible functionality
- **PATCH**: Backward-compatible bug fixes

### Version Control
- Create tags for all releases: `v1.0.0`, `v1.0.1`, etc.
- Create annotated tags with release notes:
  ```
  git tag -a v1.0.0 -m "Release v1.0.0: First stable release with combat systems"
  ```

### Release Process
1. Create `release/vX.Y.Z` branch from `development`
2. Finalize release (version bumps, CHANGELOG updates)
3. Update documentation versioning (e.g., `@since vX.Y.Z` in API docs)
4. Create PR to `main`
5. After approval and merge, tag the release
6. Merge `main` back to `development`

## CI/CD Integration

### GitHub Actions
- **PR Validation**: Run on all PRs to verify changes
  - Build check
  - Unit tests
  - Integration tests
  - Code style check
  - Documentation build

- **Main Branch Pipeline**: Run after merges to `main`
  - All PR validation steps
  - Release artifact creation
  - Deployment to staging environment

- **Release Pipeline**: Run after release tag creation
  - Create GitHub release
  - Generate binaries
  - Deploy to production environment

### Accessing CI Logs and Artifacts
- Access logs via GitHub Actions tab in the repository
- Click on the workflow run to view detailed steps and logs
- Download artifacts from the "Artifacts" section of a completed run
- Use the "Re-run" option for failed jobs after fixing issues

### Status Badges
Include status badges in README.md:
```markdown
![Build](https://github.com/username/silent-forge/workflows/Build/badge.svg)
![Tests](https://github.com/username/silent-forge/workflows/Tests/badge.svg)
```

## Documentation Updates

### When to Update Documentation
- New features: Add explanation and examples
- API changes: Update interface descriptions
- Bug fixes (if relevant): Update behavior descriptions
- Architectural changes: Update diagrams and explanations

### Documentation PR Process
1. Create branch: `docs/XXX`
2. Update relevant files in `/docs` directory
3. Preview changes locally if possible
4. Submit PR with the "Documentation update" type checked
5. Request review from technical writer or documentation owner

### Version Documentation
- Major versions should have version-specific documentation
- Add tags like `@since vX.Y.Z` in API documentation
- Maintain a "Deprecated" section for features scheduled for removal
- Update API references in `docs/technical/api.md` during release process

## Working with Large Features

### Feature Flags
- Use feature flags for gradual rollout of large features
- Keep feature flag logic isolated and easy to remove
- Document feature flags in `docs/technical/feature-flags.md`

### Feature Flag Naming Convention
- Use format: `FEATURE_<SYSTEM>_<FUNCTIONALITY>`
- Example: `FEATURE_COMBAT_TEMPORAL_EFFECTS`
- Document lifecycle plan for each flag (target release for permanent inclusion)

### Example Feature Flag
```cpp
// Feature flag defined in config
#ifdef FEATURE_TEMPORAL_COMBAT

// New feature implementation
void applyTemporalEffectsToCombat() {
    // Implementation
}

#else

// Fallback or empty implementation
void applyTemporalEffectsToCombat() {
    // Do nothing or legacy behavior
}

#endif
```

## Handling Critical Issues

### Hotfix Process
1. Create `hotfix/XXX` branch from `main`
2. Implement fix with minimal changes
3. Create PR directly to `main`
4. After approval and merge, tag as patch version
5. Merge hotfix back to `development`

### Security Vulnerabilities
- Report privately to security@example.com
- Do not create public issues for security problems
- Follow responsible disclosure practices

## Code Ownership

### CODEOWNERS File
Maintain a `.github/CODEOWNERS` file:

```
# Combat systems
/src/engine/combat/ @gabrielhuerta

# Temporal systems
/src/engine/temporal/ @gabrielhuerta

# Health systems
/src/engine/health/ @gabrielhuerta

# Documentation
/docs/ @gabrielhuerta

# Tests
/tests/ @gabrielhuerta
```

### Responsibilities
- Code owners are individuals identified by their GitHub username (e.g., `@gabrielhuerta`)
- Code owners are responsible for reviewing changes in their areas
- Code owners help maintain code standards and consistency
- At least one code owner must approve changes to their areas
- Ownership changes should be requested via PR to the CODEOWNERS file

## Best Practices

### Keep Branches Short-Lived
- Aim to merge feature branches within 1-2 weeks
- Split large features into smaller, mergeable pieces
- Regularly rebase feature branches on `development`

### Maintain a Clean History
- Use meaningful commit messages
- Squash trivial commits before merging
- Rebase to resolve conflicts (prefer over merge)

### Automated Testing
- All features should have corresponding tests
- Run tests locally before pushing
- CI should enforce test coverage requirements

### Code Style
- Follow established style guide
- Use automated formatting tools
- Run linters as part of pre-commit hooks

## Resolving Conflicts

### Rebase Process (Preferred)
1. Rebase branch on latest target branch:
   ```
   git checkout feature/my-feature
   git fetch origin
   git rebase origin/development
   ```

2. Resolve conflicts in each file
3. Continue rebase:
   ```
   git add .
   git rebase --continue
   ```

4. Force-push updated branch:
   ```
   git push --force-with-lease origin feature/my-feature
   ```

### Merge Alternative
For shared branches with multiple contributors where rebasing is impractical:

```
git checkout feature/my-feature
git fetch origin
git merge origin/development
# Resolve conflicts if any
git add .
git commit -m "Merge development into feature/my-feature"
git push origin feature/my-feature
```

### Preventing Conflicts
- Pull/rebase frequently from the target branch
- Communicate with team members working on related areas
- Break large changes into smaller, focused PRs

## Appendix

### Git Command Reference
```bash
# Create a new feature branch
git checkout development
git pull
git checkout -b feature/my-feature

# Commit changes
git add .
git commit -m "feat(combat): add new feature"

# Push to remote
git push -u origin feature/my-feature

# Update from development
git fetch origin
git rebase origin/development

# Create a release tag
git checkout main
git pull
git tag -a v1.2.0 -m "Release v1.2.0"
git push origin v1.2.0
```

### GitHub CLI Reference

```bash
# Create a PR
gh pr create --base development --head feature/my-feature --title "Add new feature" --body "Description"

# Check PR status
gh pr view

# Merge a PR
gh pr merge --squash
```

### Character Encoding Note
To ensure proper display of special characters (such as arrows in the branch flow diagram) in documentation and console output:

- In PowerShell: Use `chcp 65001` to set UTF-8 encoding before viewing or editing files
- In GitHub: Ensure files are committed with UTF-8 encoding
- In code editors: Set the default encoding to UTF-8 without BOM

This prevents characters like `─►` from appearing as `â†'` on some systems.

### Additional Resources
- [GitHub Flow Guide](https://guides.github.com/introduction/flow/)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [GitHub Actions Documentation](https://docs.github.com/en/actions) 