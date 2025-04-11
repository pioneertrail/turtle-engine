# Testing Documentation

## Overview
This document outlines the testing strategy for Silent Forge, including test frameworks, procedures, and expectations for different system components. Comprehensive testing is essential to ensure stability, performance, and correctness of all game systems.

## Testing Strategy

### Test Types
1. **Unit Tests**: Test individual functions and methods in isolation
2. **Component Tests**: Test full components with mock dependencies
3. **Integration Tests**: Test interactions between multiple components
4. **System Tests**: Test complete systems with real dependencies
5. **Performance Tests**: Measure performance against established baselines

### Test Environment
- Test builds run on dedicated test machines with controlled environments
- Automated tests run on every PR and nightly on the main branch
- Performance tests run on reference hardware (GTX 1660, Intel i5-8400)

## Test Framework

### Building Tests
Tests are built using our custom test framework based on Catch2:

```bash
# Build all tests
./compile_test.ps1

# Build specific test suite
./compile_[component]_test.ps1
```

### Running Tests
```bash
# Run all tests
./run_tests.ps1

# Run specific test suite
./run_tests.ps1 --suite=[component]

# Run with verbose output
./run_tests.ps1 --verbose

# Run performance tests only
./run_tests.ps1 --perf
```

## Combat System Tests

### PlasmaWeapon Tests
Located in `src/tests/combat/PlasmaWeaponTest.cpp`

**Test Cases:**
1. Initialization with different parameters
2. Charging mechanics and timing
3. Different firing modes (Burst, Beam, Charged, Scatter)
4. Cooldown behavior
5. Particle system integration
6. Edge cases (zero charge, rapid firing)

**Example Test:**
```cpp
TEST_CASE("PlasmaWeapon charging behavior", "[plasma]") {
    auto mockParticleSystem = std::make_shared<MockParticleSystem>();
    PlasmaWeapon weapon(mockParticleSystem);
    
    // Test that charging increases over time
    weapon.beginCharging();
    REQUIRE(weapon.getChargePercentage() == Approx(0.0f));
    
    // Simulate time passing
    for (int i = 0; i < 10; i++) {
        weapon.update(0.1f);  // Update with 100ms
    }
    
    // Should have charged to approximately 50% (depends on charge rate)
    REQUIRE(weapon.getChargePercentage() > 0.4f);
    REQUIRE(weapon.getChargePercentage() < 0.6f);
}
```

### AIConstruct Tests
Located in `src/tests/combat/AIConstructTest.cpp`

**Test Cases:**
1. State transitions based on player proximity
2. Attack behavior and damage calculation
3. Type-specific properties (Sentry, Hunter, Guardian, Swarm)
4. Path following in patrol state
5. Retreat behavior when health is low
6. Reaction to damage

**Example Test:**
```cpp
TEST_CASE("AIConstruct transitions to attack when player in range", "[ai]") {
    auto mockParticleSystem = std::make_shared<MockParticleSystem>();
    AIConstruct ai(mockParticleSystem, AIConstruct::Type::HUNTER);
    
    // Initialize in IDLE state
    ai.forceState(AIConstruct::State::IDLE);
    REQUIRE(ai.getState() == AIConstruct::State::IDLE);
    
    // Player position far from AI
    glm::vec3 farPosition(100.0f, 0.0f, 0.0f);
    ai.update(0.1f, farPosition);
    REQUIRE(ai.getState() == AIConstruct::State::IDLE);
    
    // Player position within detection range
    glm::vec3 nearPosition(5.0f, 0.0f, 0.0f);
    ai.update(0.1f, nearPosition);
    REQUIRE(ai.getState() == AIConstruct::State::ATTACK);
}
```

### HealthSystem Tests
Located in `src/tests/combat/HealthSystemTest.cpp`

**Test Cases:**
1. Basic damage application
2. Different damage types and resistances
3. Resilience component functionality
4. Shield absorption
5. Flat damage reduction
6. Death callback
7. Healing mechanics

**Example Test:**
```cpp
TEST_CASE("HealthComponent applies resistances correctly", "[health]") {
    HealthComponent health(100.0f);
    
    // Setup resistances
    health.getResilience().setResistance(DamageType::PLASMA, 0.5f);  // 50% resistance
    
    // Create damage info
    DamageInfo damage(20.0f, DamageType::PLASMA);
    
    // Apply damage and check results
    float actualDamage = health.applyDamage(damage);
    REQUIRE(actualDamage == Approx(10.0f));  // 50% of 20
    REQUIRE(health.getCurrentHealth() == Approx(90.0f));
}
```

## Test Result Analysis

### Output Files
Test results are stored in several formats:
- `test_results_[component].txt` - Text-based test results
- `test_summary.txt` - Overall summary of all tests
- `logs/test_[component]_[timestamp].log` - Detailed logs
- `performance_data.csv` - Performance metrics for analysis

### Result Format
```
[TEST][PlasmaWeapon] Firing mode BEAM particles: Expected >40, Actual: 42 ✓
[TEST][PlasmaWeapon] Cooldown time after firing: Expected ~0.5s, Actual: 0.51s ✓
```

### Performance Testing
Performance tests track key metrics:
- Frame time (ms)
- Memory usage (MB)
- Draw calls per frame
- CPU usage per system (%)
- Load times (seconds)

Results are compared against baseline performance requirements from `docs/project/technical-specs.md`.

## Continuous Integration

### Automated Test Pipeline
1. Code changes pushed to branch
2. CI system builds test binaries
3. Unit and component tests run automatically
4. Performance tests run on reference hardware
5. Results compared against baselines
6. Test report generated and attached to PR

### Test Status Badges
- ✅ All tests passed
- ⚠️ Performance tests below target
- ❌ Test failures

## Test-Driven Development Guidelines

1. Write tests before implementing features
2. Ensure all code paths are covered
3. Include edge cases and error scenarios
4. Test performance implications
5. Update tests when changing functionality

## Debug Visualization for Testing

When running tests with the `--visual` flag, debug visualization is enabled to help diagnose issues:

```bash
./run_tests.ps1 --suite=ai --visual
```

This produces screenshot evidence for each test case in the `test_output/screenshots` directory. 