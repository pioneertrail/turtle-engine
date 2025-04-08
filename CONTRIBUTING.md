# Contributing to Turtle Engine

Thank you for your interest in contributing to Turtle Engine! This document provides guidelines and instructions for contributing.

## Code of Conduct

By participating in this project, you agree to maintain a respectful and inclusive environment for everyone.

## How to Contribute

### Reporting Bugs

1. Check if the bug has already been reported in the [Issues](https://github.com/pioneertrail/turtle-engine/issues)
2. If not, create a new issue with:
   - Clear title and description
   - Steps to reproduce
   - Expected vs actual behavior
   - System information (OS, GPU, drivers)
   - Code samples if applicable

### Suggesting Enhancements

1. Open a new issue labeled as 'enhancement'
2. Provide:
   - Clear description of the feature
   - Use cases and benefits
   - Potential implementation approach
   - Any relevant examples or references

### Pull Requests

1. Fork the repository
2. Create a new branch:
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. Make your changes following our coding standards
4. Write or update tests as needed
5. Update documentation if required
6. Commit your changes:
   ```bash
   git commit -m "feat: add your feature description"
   ```
7. Push to your fork:
   ```bash
   git push origin feature/your-feature-name
   ```
8. Open a Pull Request

## Development Setup

1. Install prerequisites:
   - CMake 3.10+
   - C++17 compiler
   - OpenGL 3.3+ capable GPU
   - GLEW, GLFW3, GLM

2. Clone and build:
   ```bash
   git clone https://github.com/pioneertrail/turtle-engine.git
   cd turtle-engine
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

3. Run tests:
   ```bash
   ctest --output-on-failure
   ```

## Coding Standards

- Use C++17 features appropriately
- Follow existing code style
- Document all public APIs using Doxygen format
- Write unit tests for new features
- Keep commits focused and atomic
- Use descriptive commit messages following [Conventional Commits](https://www.conventionalcommits.org/)

## Commit Message Format

```
type(scope): description

[optional body]

[optional footer]
```

Types:
- feat: New feature
- fix: Bug fix
- docs: Documentation changes
- style: Code style changes
- refactor: Code refactoring
- test: Adding/modifying tests
- chore: Maintenance tasks

Example:
```
feat(renderer): add support for texture mapping

Implement texture mapping functionality in the renderer:
- Add texture loading using stb_image
- Add texture coordinate support in vertex data
- Update shaders to support texturing

Closes #42
```

## Documentation

- Update README.md for significant changes
- Document new features in code using Doxygen
- Update API documentation when changing interfaces
- Include examples for new functionality

## Questions?

Feel free to:
- Open an issue for questions
- Join our discussions
- Contact the maintainers

Thank you for contributing to Turtle Engine! 