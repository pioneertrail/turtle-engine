#include "combat/Hitbox.hpp"
#include <cmath> // For std::abs

namespace TurtleEngine
{
    bool CheckCollision(const HitboxAABB& a, const HitboxAABB& b)
    {
        // Check for overlap on each axis
        bool overlapX = std::abs(a.center.x - b.center.x) <= (a.halfExtents.x + b.halfExtents.x);
        bool overlapY = std::abs(a.center.y - b.center.y) <= (a.halfExtents.y + b.halfExtents.y);
        bool overlapZ = std::abs(a.center.z - b.center.z) <= (a.halfExtents.z + b.halfExtents.z);

        // Collision occurs if there is overlap on all axes
        return overlapX && overlapY && overlapZ;
    }

} // namespace TurtleEngine 