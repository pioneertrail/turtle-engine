#ifndef TURTLE_ENGINE_HITBOX_HPP
#define TURTLE_ENGINE_HITBOX_HPP

#include <glm/vec3.hpp> // For 3D position and dimensions

namespace TurtleEngine
{
    // Represents an Axis-Aligned Bounding Box (AABB)
    struct HitboxAABB
    {
        glm::vec3 center{0.0f, 0.0f, 0.0f};       // Center position of the hitbox
        glm::vec3 halfExtents{0.5f, 0.5f, 0.5f}; // Half-dimensions along each axis
        // Add other properties as needed, e.g., owner ID, type (attack/hurtbox), active state, etc.
        // unsigned int ownerId = 0;
        // enum class Type { ATTACK, HURTBOX };
        // Type type = Type::HURTBOX;
        // bool isActive = true;
    };

    // TODO: Add functions for collision detection, e.g., CheckCollision(const HitboxAABB& a, const HitboxAABB& b)
    // TODO: Consider adding other hitbox shapes (Sphere, OBB) later if needed.

} // namespace TurtleEngine

#endif // TURTLE_ENGINE_HITBOX_HPP 