#ifndef TURTLE_ENGINE_VISUAL_FEEDBACK_HPP
#define TURTLE_ENGINE_VISUAL_FEEDBACK_HPP

#include <glm/vec3.hpp>
#include <string>
#include <chrono>

namespace TurtleEngine
{
    // Represents a transient visual effect at a specific location (e.g., hit spark, block effect)
    struct TransientEffectFeedback
    {
        enum class Type { HIT_SPARK, BLOCK_SHIELD, PARRY_FLASH };
        
        Type type = Type::HIT_SPARK;
        glm::vec3 position{0.0f}; // World position for the effect
        std::chrono::steady_clock::time_point creationTime;
        std::chrono::milliseconds duration{150}; // How long the effect lasts
        // Add other properties: intensity, scale, specific particle system ID, color, etc.
        // float scale = 1.0f;
        // std::string particleSystemId = "default_hit_spark";
    };

    // Represents persistent UI feedback (e.g., combo counter)
    struct UIFeedback
    {
        enum class Type { COMBO_COUNTER, DAMAGE_NUMBER, STATUS_EFFECT_ICON };
        
        Type type = Type::COMBO_COUNTER;
        std::string text; // Text to display (e.g., "3 Hits!")
        glm::vec3 screenPosition{0.0f}; // Position on screen (could be relative or absolute)
        std::chrono::steady_clock::time_point creationTime;
        std::chrono::milliseconds duration{1000}; // How long the text stays visible
        // Add other properties: color, font size, animation (e.g., fade out, scale up), target entity ID
        // glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        // int fontSize = 24;
    };

    // TODO: Implement a VisualFeedbackManager to manage active feedback instances.
    // TODO: Integrate with rendering system to draw effects and UI elements.
    // TODO: Connect Hitbox collisions and ComboManager events to trigger feedback creation.

} // namespace TurtleEngine

#endif // TURTLE_ENGINE_VISUAL_FEEDBACK_HPP 