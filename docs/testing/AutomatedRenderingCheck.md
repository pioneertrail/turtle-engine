# Automated Rendering Pipeline Check Strategy

## Goal

To provide a basic, automated check that verifies the core rendering pipeline (shader loading/linking, reaching render calls) without requiring manual visual inspection. This is useful for CI/CD environments or quick regression checks.

## Methodology

1.  **Dedicated Test Executable:**
    *   A separate executable (e.g., `TurtleEngine_Tests`) is created using CMake (`add_executable`).
    *   This executable links against the core engine library (`TurtleEngineCore`).
    *   This keeps test-specific logic separate from the main application.

2.  **Engine Initialization & Shutdown:**
    *   The test executable initializes a `TurtleEngine::Engine` instance, including necessary subsystems (window context, GLEW, etc.).
    *   It calls `engine.shutdown()` at the end for clean resource release.

3.  **Simulation Hooks:**
    *   Core engine components are modified to allow external triggering of logic relevant to rendering:
        *   `Engine::simulateKeyPress(char key)`: Allows the test to simulate user input needed to activate certain rendering paths (e.g., spawning particles via a gesture).
        *   `Engine::update(float deltaTime)`: Allows the test to step the engine's logic (particle updates, CSL updates) for one or more frames.

4.  **State Accessors:**
    *   Core engine components provide minimal `const` accessor methods to query state relevant to the test:
        *   `Engine::getParticleSystem()`: Returns a reference to the particle system.
        *   `ParticleSystem::getActiveParticleCount()`: Returns the number of particles expected to be rendered.

5.  **Test Execution Flow:**
    *   Initialize the engine.
    *   Run an initial `engine.update()` if necessary to settle systems.
    *   Call `engine.simulateKeyPress()` to trigger the desired rendering condition (e.g., Flammil gesture).
    *   Call `engine.update()` again to process the simulated input and update internal states (e.g., particle buffer).
    *   Query the relevant state using accessors (e.g., `engine.getParticleSystem().getActiveParticleCount()`).
    *   Based on the queried state, print a clear, machine-readable message to standard output/error:
        *   `std::cout << "TEST_PASSED: FeatureCheck"` (e.g., `TEST_PASSED: ParticleSpawn`)
        *   `std::cerr << "TEST_FAILED: FeatureCheck - Reason"` (e.g., `TEST_FAILED: ParticleSpawn - Count: 0`)
    *   Return `0` on pass, non-zero on failure.

6.  **Build System Support:**
    *   The `CMakeLists.txt` file must ensure all necessary runtime assets (e.g., the `shaders` directory) are copied to the test executable's output directory using `add_custom_command(TARGET ... POST_BUILD ...)` for the test target.

## Usage

Run the test executable (e.g., `TurtleEngine_Tests.exe`) from the command line, a script, or a CI pipeline. Check the exit code (0 for success) and parse the stdout/stderr for the `TEST_PASSED` or `TEST_FAILED` messages.

## Limitations

*   **Does not verify visual correctness:** This test only confirms that the *logic* for rendering (shader loading, linking, updates, reaching draw calls with expected counts) executed without specific errors. It doesn't guarantee the output *looks* correct.
*   **No GPU-specific validation:** Doesn't catch rendering artifacts or errors specific to certain GPUs or drivers.
*   **Requires engine modification:** Needs specific simulation hooks and state accessors added to the core engine.

## Example (Particle Spawning Check)

See `src/tests/TurtleEngine_Tests.cpp` for an implementation checking if particles are correctly spawned and updated in the buffer after simulating the 'F' key press (Flammil gesture). 