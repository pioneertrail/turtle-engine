#include "combat/Hitbox.hpp"
#include <cassert> // For basic assertions
#include <iostream> // For output

// Declare the collision check function if not already in a shared header accessible here
// (Assuming Hitbox.hpp includes the declaration or it's implicitly available)
namespace TurtleEngine {
    bool CheckCollision(const HitboxAABB& a, const HitboxAABB& b);
}

void RunHitboxTests()
{
    std::cout << "Running Hitbox Tests..." << std::endl;

    // Test Case 1: Perfect Overlap
    TurtleEngine::HitboxAABB box1{{0, 0, 0}, {1, 1, 1}};
    TurtleEngine::HitboxAABB box2{{0, 0, 0}, {1, 1, 1}};
    assert(TurtleEngine::CheckCollision(box1, box2) && "Test Case 1 Failed: Perfect Overlap");
    std::cout << "  Test Case 1 Passed: Perfect Overlap" << std::endl;

    // Test Case 2: Partial Overlap
    TurtleEngine::HitboxAABB box3{{1, 1, 1}, {1, 1, 1}};
    TurtleEngine::HitboxAABB box4{{2, 2, 2}, {1, 1, 1}};
    assert(TurtleEngine::CheckCollision(box3, box4) && "Test Case 2 Failed: Partial Overlap");
    std::cout << "  Test Case 2 Passed: Partial Overlap" << std::endl;

    // Test Case 3: Touching Edges
    TurtleEngine::HitboxAABB box5{{0, 0, 0}, {1, 1, 1}};
    TurtleEngine::HitboxAABB box6{{2, 0, 0}, {1, 1, 1}};
    assert(TurtleEngine::CheckCollision(box5, box6) && "Test Case 3 Failed: Touching Edges X");
    std::cout << "  Test Case 3 Passed: Touching Edges X" << std::endl;

    // Test Case 4: No Overlap (Separated)
    TurtleEngine::HitboxAABB box7{{0, 0, 0}, {1, 1, 1}};
    TurtleEngine::HitboxAABB box8{{3, 0, 0}, {1, 1, 1}};
    assert(!TurtleEngine::CheckCollision(box7, box8) && "Test Case 4 Failed: No Overlap X");
    std::cout << "  Test Case 4 Passed: No Overlap X" << std::endl;

    // Test Case 5: No Overlap (Separated Y)
    TurtleEngine::HitboxAABB box9{{0, 0, 0}, {1, 1, 1}};
    TurtleEngine::HitboxAABB box10{{0, 3, 0}, {1, 1, 1}};
    assert(!TurtleEngine::CheckCollision(box9, box10) && "Test Case 5 Failed: No Overlap Y");
    std::cout << "  Test Case 5 Passed: No Overlap Y" << std::endl;

    // Test Case 6: No Overlap (Separated Z)
    TurtleEngine::HitboxAABB box11{{0, 0, 0}, {1, 1, 1}};
    TurtleEngine::HitboxAABB box12{{0, 0, 3}, {1, 1, 1}};
    assert(!TurtleEngine::CheckCollision(box11, box12) && "Test Case 6 Failed: No Overlap Z");
    std::cout << "  Test Case 6 Passed: No Overlap Z" << std::endl;

    // Test Case 7: One box inside another
    TurtleEngine::HitboxAABB box13{{0, 0, 0}, {2, 2, 2}};
    TurtleEngine::HitboxAABB box14{{0, 0, 0}, {0.5, 0.5, 0.5}};
    assert(TurtleEngine::CheckCollision(box13, box14) && "Test Case 7 Failed: Contained");
    std::cout << "  Test Case 7 Passed: Contained" << std::endl;

    std::cout << "Hitbox Tests Completed Successfully!" << std::endl;
}

// Basic main for running tests if this becomes an executable
// If integrated into a larger test suite, this main might not be needed.
int main() {
    RunHitboxTests();
    return 0;
} 