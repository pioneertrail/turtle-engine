// Temporarily comment out Combo.hpp include
//#include "../../combat/include/Combo.hpp"
#include "../../../engine/combat/include/Combo.hpp"
#include <vector>
#include <chrono>
#include <string>
#include <iostream> // For placeholder logging

namespace TurtleEngine
{
    // --- Helper Function --- 
    namespace {
        // Helper function to find a step by identifier (Recursive)
        // NOTE: This assumes identifiers are unique enough within the search scope.
        // Returns nullptr if not found.
        const ComboStep* FindStepByIdentifier(const ComboStep& step, const std::string& id) {
            if (step.moveIdentifier == id) {
                return &step;
            }
            for (const auto& next : step.nextSteps) {
                const ComboStep* found = FindStepByIdentifier(next, id);
                if (found) {
                    return found;
                }
            }
            return nullptr;
        }    
    } // anonymous namespace

    // --- ComboManager Method Implementations --- 

    ComboManager::ComboManager(const std::vector<ComboSequence>& availableCombos)
        : availableCombos_(availableCombos), 
          lastMoveTime_(std::chrono::steady_clock::now()), // Initialize time
          currentStatePtr_(nullptr) // Start with no active combo step
    {}

    void ComboManager::ProcessMove(const std::string& moveIdentifier)
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::milliseconds timeSinceLast = 
            std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMoveTime_);

        std::cout << "Processing move: " << moveIdentifier 
                    << " (Time since last: " << timeSinceLast.count() << "ms)" << std::endl;

        const ComboStep* nextStatePtr = nullptr;
        bool comboContinued = false;
        const ComboStep* previousStatePtr = currentStatePtr_; // Store previous state for messages

        // 1. Check if currently in a combo and the input continues it
        if (currentStatePtr_) {
            // Find the actual current step definition within availableCombos_ to access its *real* nextSteps
            // This search is inefficient and assumes moveIdentifiers are unique within a combo's direct next steps.
            // A better approach might involve restructuring ComboSequence/ComboStep.
            for (const auto& sequence : availableCombos_) { 
                 // TODO: Need a way to efficiently find the sequence/step `currentStatePtr_` belongs to
                 // This naive loop won't work correctly if multiple sequences share starting steps.
                 // For now, we directly check the nextSteps of the pointed-to object, ASSUMING it's valid.
            }
            // Check the next possible steps from the current one
            for (const auto& possibleNextStep : currentStatePtr_->nextSteps) {
                if (possibleNextStep.moveIdentifier == moveIdentifier && timeSinceLast <= possibleNextStep.maxTimeSincePrevious) {
                    // Found a valid transition!
                    // Now, find the *actual* ComboStep object this corresponds to within availableCombos_
                    // This requires searching - again, inefficient and potentially ambiguous.
                    // Simple Placeholder: Assume the first match found by identifier is correct (RISKY!)
                    for (const auto& seq : availableCombos_) {
                         // Recursive search function would be better here
                         const ComboStep* found = FindStepByIdentifier(seq.startingStep, possibleNextStep.moveIdentifier);
                         if (found) {
                             nextStatePtr = found;
                             break;
                         }
                    }
                    
                    if (nextStatePtr) {
                         std::cout << "  Combo continued: " << currentStatePtr_->moveIdentifier << " -> " << nextStatePtr->moveIdentifier << std::endl;
                         comboContinued = true;
                         break; // Found transition
                    }
                }
            }
        }

        // 2. If combo didn't continue (or wasn't active), check if the input starts a new combo
        if (!comboContinued) {
            for (const auto& sequence : availableCombos_) {
                if (sequence.startingStep.moveIdentifier == moveIdentifier) {
                    // Found a starting move
                    nextStatePtr = &sequence.startingStep; // Point to the actual starting step object
                    std::cout << "  Started new combo: " << sequence.sequenceName << std::endl;
                    break;
                }
            }
        }

        // 3. Update state
        currentStatePtr_ = nextStatePtr; // Update (or nullify) the current state pointer

        if (!currentStatePtr_) {
             if(comboContinued) { // Mid-combo, but failed to find the next step object
                 std::cout << "  Combo broken (failed to find next step object)." << std::endl;
             } else if (previousStatePtr) { // Was in a combo previously, but it didn't continue and no new one started
                 std::cout << "  Combo broken (timeout or invalid move)." << std::endl;
             } // else: wasn't in a combo and didn't start one - normal state, no message needed
        } else {
            // Update the simple string state for debug method (TEMP)
            currentState_ = currentStatePtr_->moveIdentifier;
        }
        if (!currentStatePtr_) { // Ensure string state is cleared if combo ends
             currentState_.clear();
        }

        lastMoveTime_ = now; // Always update last move time
    }

    // Debug methods implementation
    std::string ComboManager::getCurrentStateDebug() const 
    {
        // Return the temporary string state for now
        // return currentState_; 
        // Correct version using the pointer:
        return currentStatePtr_ ? currentStatePtr_->moveIdentifier : ""; 
    }

    std::chrono::steady_clock::time_point ComboManager::getLastMoveTimeDebug() const 
    {
        return lastMoveTime_;
    }

    // TODO: Need to instantiate ComboManager for entities and provide the list of defined ComboSequences.

} // namespace TurtleEngine