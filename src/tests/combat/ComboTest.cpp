#include "combat/Combo.hpp"
// Need to include the definition of ComboManager, likely by including Combo.cpp
// or preferably moving ComboManager class definition to Combo.hpp if it's intended for external use.
// For now, assuming Combo.cpp includes necessary definitions or they are moved.
// #include "combat/Combo.cpp" // Avoid including cpp files directly in general

// Temporary: Define ComboManager here for the test if not in a header
// Ideally, ComboManager class definition should be in Combo.hpp
#include <vector>
#include <chrono>
#include <string>
#include <iostream>
#include <thread> // For sleep
#include <cassert> 

// Removed the entire redundant ComboManager definition that was here.
// The correct definition will be linked from TurtleEngineCore library.

void RunComboTests()
{
    std::cout << "Running Combo Tests..." << std::endl;

    // Define some test combos
    TurtleEngine::ComboStep step2 {"Punch2", std::chrono::milliseconds(300)};
    TurtleEngine::ComboStep step1 {"Punch1", std::chrono::milliseconds(500), {}, {step2}};
    TurtleEngine::ComboSequence combo1 {"Basic_Punch", step1};

    TurtleEngine::ComboStep kickStep {"Kick1", std::chrono::milliseconds(600)};
    TurtleEngine::ComboSequence combo2 {"Basic_Kick", kickStep};

    std::vector<TurtleEngine::ComboSequence> availableCombos = {combo1, combo2};

    TurtleEngine::ComboManager manager(availableCombos);

    // Test Case 1: Start a combo
    std::cout << "  Test Case 1: Start Combo" << std::endl;
    manager.ProcessMove("Punch1");
    assert(manager.getCurrentStateDebug() == "Punch1" && "Test Case 1 Failed: State not Punch1");
    std::cout << "    Passed." << std::endl;

    // Test Case 2: Continue a combo (within time)
    std::cout << "  Test Case 2: Continue Combo (Valid Time)" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Wait less than 300ms for Punch2
    auto timeBefore = manager.getLastMoveTimeDebug();
    manager.ProcessMove("Punch2");
    auto timeAfter = manager.getLastMoveTimeDebug();
    assert(manager.getCurrentStateDebug() == "Punch2" && "Test Case 2 Failed: State not Punch2 after valid transition"); // Should now transition correctly
    assert(timeAfter > timeBefore && "Test Case 2 Failed: Time not updated");
    std::cout << "    Passed." << std::endl;

    // Test Case 3: Fail to continue (too slow)
    std::cout << "  Test Case 3: Fail Combo (Invalid Time)" << std::endl;
    manager.ProcessMove("Punch1"); // Restart combo
    assert(manager.getCurrentStateDebug() == "Punch1" && "Test Case 3 Setup Failed: State not Punch1");
    std::this_thread::sleep_for(std::chrono::milliseconds(400)); // Wait more than 300ms (Punch2 window)
    manager.ProcessMove("Punch2");
    assert(manager.getCurrentStateDebug().empty() && "Test Case 3 Failed: State not reset after failed Punch2 (timeout)");
    std::cout << "    Passed." << std::endl;

    // Test Case 4: Start a different combo after breaking one
    std::cout << "  Test Case 4: Start Different Combo" << std::endl;
    manager.ProcessMove("Kick1");
    assert(manager.getCurrentStateDebug() == "Kick1" && "Test Case 4 Failed: State not Kick1");
    std::cout << "    Passed." << std::endl;

    // Test Case 5: Invalid move during combo
    std::cout << "  Test Case 5: Invalid Move During Combo" << std::endl;
    manager.ProcessMove("Punch1"); // Start Punch combo
    assert(manager.getCurrentStateDebug() == "Punch1" && "Test Case 5 Setup Failed: State not Punch1");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    manager.ProcessMove("InvalidMove");
    assert(manager.getCurrentStateDebug().empty() && "Test Case 5 Failed: State not reset after invalid move during combo");
    std::cout << "    Passed." << std::endl;

    std::cout << "Combo Tests Completed!" << std::endl; // Remove placeholder note
}

int main() {
    RunComboTests();
    return 0;
} 