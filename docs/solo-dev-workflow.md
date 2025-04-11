# Solo Developer Git Workflow

## Overview
This document outlines a simplified Git workflow for solo development on the Silent Forge: Rebellion of Aethelgard project. It establishes streamlined procedures that maintain code quality while eliminating unnecessary overhead.

## Branch Strategy

### Main Branch
- **new-main**: Primary development branch, should remain stable enough for builds

### Optional Working Branches
- **feature/XXX**: For experimental features or major changes
- **bugfix/XXX**: For complex bug fixes that require isolation
- **refactor/XXX**: For significant code reorganization

### Branch Flow Diagram
```
    feature/XXX
    bugfix/XXX     
    refactor/XXX
        │
        ▼
new-main ◄────────
```

## Commit Message Guidelines

### Format
```
<type>(<scope>): <subject>
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
- **chore**: Routine tasks, maintenance, etc.

### Scope
The system or component affected (e.g., `combat`, `ai`, `health`, `temporal`)

### Examples
```
feat(combat): add scatter firing mode to plasma weapons
```

```
fix(health): correct damage calculation for shield penetration
```

## Working Directly on main

For most changes, you can work directly on the `new-main` branch:

1. Make your changes with descriptive commits
2. Push directly to `new-main`

## Using Feature Branches (when necessary)

For experimental changes or major features:

1. Create a branch from `new-main`: `git checkout -b feature/new-feature`
2. Implement changes with descriptive commits
3. When ready, merge back to `new-main`:
   ```
   git checkout new-main
   git merge feature/new-feature
   ```
4. Delete the feature branch when no longer needed: `git branch -d feature/new-feature`

## Versioning

### Version Format
Follow [Semantic Versioning](https://semver.org/): `MAJOR.MINOR.PATCH`

- **MAJOR**: Incompatible API changes
- **MINOR**: Backward-compatible functionality
- **PATCH**: Backward-compatible bug fixes

### Creating Releases
1. Update version numbers in relevant files
2. Commit with message `chore(release): vX.Y.Z`
3. Create tag: `git tag -a vX.Y.Z -m "Release vX.Y.Z: Brief description"`
4. Push changes and tag: `git push && git push --tags`

## Backup Practices

Since you're the only developer, it's crucial to:

1. Push changes frequently to the remote repository
2. Consider creating periodic backup branches: `git branch backup/YYYY-MM-DD`
3. Keep local backups of important development milestones

## Documentation Updates

### When to Update Documentation
- Document new features as you implement them
- Keep API documentation current
- Update README.md when making significant changes

## Simplified Release Process

1. Ensure all tests pass on `new-main`
2. Update version numbers and CHANGELOG.md
3. Create and push a version tag
4. Build release artifacts

This simplified workflow eliminates the overhead of pull requests and code reviews while maintaining good development practices through consistent commit messages and smart branching when needed. 