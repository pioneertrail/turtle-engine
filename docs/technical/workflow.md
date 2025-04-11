# TurtleEngine Git Workflow Guide

## Overview

This document outlines the Git workflow for the TurtleEngine project, ensuring smooth collaboration and maintaining code quality across all tribes.

## Branch Structure

- `main`: Production-ready code, always stable
- `develop`: Integration branch for feature development
- `feature/*`: Individual feature branches
- `bugfix/*`: Bug fix branches
- `release/*`: Release preparation branches
- `hotfix/*`: Emergency production fixes

## Feature Development Workflow

1. **Branch Creation**
   ```bash
   git checkout -b feature/your-feature-name develop
   ```

2. **Regular Updates**
   - Rebase frequently to stay current with develop:
   ```bash
   git fetch origin
   git rebase origin/develop
   ```

3. **Commit Guidelines**
   - Use semantic commit messages:
     ```
     feat(component): Add new feature
     fix(component): Fix specific issue
     docs(component): Update documentation
     perf(component): Improve performance
     refactor(component): Code restructuring
     test(component): Add/update tests
     ```
   - Keep commits small and focused
   - Include relevant test updates
   - Reference issue numbers when applicable

4. **Pre-Push Checklist**
   - Run all tests
   - Check performance metrics
   - Verify on target hardware (Intel HD 4000, GTX 1660)
   - Update documentation
   - Review debug visualizations

5. **Code Review Process**
   - Create detailed pull request
   - Include performance metrics
   - Add visual comparisons for graphics changes
   - Request reviews from relevant specialists
   - Address feedback promptly

## Conflict Resolution

1. **Prevention**
   - Regular rebasing (at least daily)
   - Small, focused commits
   - Clear communication on shared component changes
   - Use feature flags for major changes

2. **Resolution Steps**
   ```bash
   git fetch origin
   git rebase origin/develop
   # If conflicts occur:
   # 1. Resolve conflicts
   # 2. git add <resolved-files>
   # 3. git rebase --continue
   # 4. Run tests to verify
   ```

## Performance Considerations

- Run performance tests before and after changes
- Document performance impacts in commit messages
- Include profiling data for significant changes
- Test on all target hardware configurations

## Debug Visualization

- Enable debug views when testing visual changes
- Document visual debug flags
- Include before/after screenshots in PRs
- Test high-contrast mode for accessibility

## Communication

1. **Branch Status**
   - Update branch status in project board
   - Tag relevant team members
   - Use standardized labels

2. **Documentation Updates**
   - Update technical docs with changes
   - Include visual guides for UI changes
   - Document performance implications

3. **Cross-Tribe Coordination**
   - Regular sync meetings
   - Shared component change notifications
   - Performance impact discussions

## Best Practices

1. **Code Organization**
   - Follow established module structure
   - Keep related changes together
   - Update tests with code changes
   - Maintain documentation

2. **Performance**
   - Profile before optimization
   - Document performance requirements
   - Test on all target platforms
   - Use debug visualization tools

3. **Graphics**
   - Test all rendering paths
   - Verify high-contrast mode
   - Check multiple resolutions
   - Validate debug views

4. **Testing**
   - Unit tests for new features
   - Performance tests for changes
   - Visual regression tests
   - Cross-platform validation

## Release Process

1. **Preparation**
   ```bash
   git checkout -b release/v1.x.0 develop
   # Update version numbers
   # Run full test suite
   # Update documentation
   ```

2. **Testing**
   - Full regression testing
   - Performance validation
   - Platform-specific checks
   - Debug view verification

3. **Finalization**
   ```bash
   git checkout main
   git merge release/v1.x.0
   git tag -a v1.x.0 -m "Release v1.x.0"
   git checkout develop
   git merge release/v1.x.0
   ```

## Emergency Fixes

1. **Hotfix Process**
   ```bash
   git checkout -b hotfix/issue-description main
   # Fix the issue
   # Update version number
   git checkout main
   git merge hotfix/issue-description
   git checkout develop
   git merge hotfix/issue-description
   ```

## Additional Resources

- [Git Documentation](https://git-scm.com/doc)
- [OpenGL Best Practices](https://www.khronos.org/opengl/wiki/Performance)
- [Performance Optimization Guide](docs/technical/performance.md)
- [Visual Debug Guide](docs/technical/debug-visualization.md) 