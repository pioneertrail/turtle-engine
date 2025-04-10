#ifndef TURTLE_ENGINE_COMBO_HPP
#define TURTLE_ENGINE_COMBO_HPP

#include <vector>
#include <string> // Or use an enum/ID system for moves
#include <chrono> // For timing windows

namespace TurtleEngine
{
    // Represents a single step (move/gesture) within a combo sequence
    struct ComboStep
    {
        std::string moveIdentifier; // Identifier for the move/gesture (e.g., "Flammil_Swipe", "Light_Punch")
        std::chrono::milliseconds maxTimeSincePrevious{500}; // Max time allowed since the previous step
        // Add other step-specific properties: damage, effects, animation, etc.
        // float damage = 10.0f;
        // std::string animationName = "default_attack";

        // Potential next steps in the combo
        std::vector<ComboStep> nextSteps;
    };

    // Represents a complete combo sequence, potentially starting from an initial move
    struct ComboSequence
    {
        std::string sequenceName; // Name for the combo (e.g., "Flammil_Basic_Combo")
        ComboStep startingStep;  // The first move that initiates this combo sequence
    };

    // Manages the combo state for a single entity (e.g., player, AI)
    class ComboManager
    {
    public:
        // Constructor: Takes a reference to the available combo definitions
        ComboManager(const std::vector<ComboSequence>& availableCombos);

        // Processes a new move input and updates the combo state
        void ProcessMove(const std::string& moveIdentifier);

        // TODO: Add methods to query the current combo state, completed combos, etc.
        // bool IsComboActive() const;
        // std::string GetCurrentComboName() const;

        // Debug methods (optional, could be removed later)
        std::string getCurrentStateDebug() const;
        std::chrono::steady_clock::time_point getLastMoveTimeDebug() const;

    private:
        const std::vector<ComboSequence>& availableCombos_; // Reference to all defined combos
        const ComboStep* currentStatePtr_; // Pointer to the current step within a sequence in availableCombos_
        std::string currentState_; // TEMPORARY simple state, replace with currentStatePtr_ logic
        std::chrono::steady_clock::time_point lastMoveTime_; // Timestamp of the last processed move
    };

    // TODO: Define how moveIdentifiers link to actual game actions/animations/CSL gestures.
    // TODO: Implement logic to check if a sequence of inputs matches a defined ComboSequence within timing windows.

} // namespace TurtleEngine

#endif // TURTLE_ENGINE_COMBO_HPP 