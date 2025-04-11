#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <chrono>

namespace TurtleEngine {
namespace Input {

// Define Gesture patterns and types
enum class GestureType {
    FLAMMIL,   // Quick forward thrust - creates a small burst of fire
    AERETH,    // Circular motion - creates a protective air shield
    TURANIS,   // Lightning bolt pattern - electrical attack
    AQUOS,     // Wave pattern - water/ice effect
    VITALIS,   // Spiral pattern - healing/regeneration
    UMBRIS,    // X pattern - shadow/darkness effect
    TERRAE     // Triangle pattern - earth/stone effect
};

struct GesturePoint {
    glm::vec3 position;
    float timestamp;
    
    GesturePoint(const glm::vec3& pos = glm::vec3(0.0f), float time = 0.0f)
        : position(pos), timestamp(time) {}
};

using GesturePattern = std::vector<GesturePoint>;
using GestureCallback = std::function<void(GestureType)>;

class GestureRecognizer {
public:
    GestureRecognizer() {
        // Initialize the gesture templates
        initializeGestureTemplates();
        std::cout << "[GestureRecognizer] Initialized with " << m_gestureTemplates.size() << " gesture patterns" << std::endl;
    }
    
    void recordGesturePoint(const glm::vec3& position) {
        float now = static_cast<float>(getCurrentTimeMillis()) / 1000.0f;
        
        // Only add points that are different enough from the last one
        if (m_currentGesture.empty() || 
            glm::distance(position, m_currentGesture.back().position) > m_minPointDistance) {
            m_currentGesture.push_back(GesturePoint(position, now));
            std::cout << "[GestureRecognizer] Recorded point at (" 
                     << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
        }
    }
    
    void beginGesture() {
        m_currentGesture.clear();
        m_isRecording = true;
        std::cout << "[GestureRecognizer] Beginning gesture recording" << std::endl;
    }
    
    GestureType endGesture() {
        m_isRecording = false;
        
        if (m_currentGesture.size() < 3) {
            std::cout << "[GestureRecognizer] Gesture too short, ignoring" << std::endl;
            return GestureType::FLAMMIL; // Default, not actually recognized
        }
        
        // Normalize the gesture to make it scale-invariant
        auto normalizedGesture = normalizeGesture(m_currentGesture);
        
        // Find the best matching gesture
        GestureType recognizedType;
        float bestScore = 0.0f;
        bool gestureRecognized = false;
        
        for (const auto& [type, templ] : m_gestureTemplates) {
            float score = compareGestures(normalizedGesture, templ);
            std::cout << "[GestureRecognizer] Match score for " << static_cast<int>(type) 
                     << ": " << score << std::endl;
            
            if (score > bestScore && score > m_recognitionThreshold) {
                bestScore = score;
                recognizedType = type;
                gestureRecognized = true;
            }
        }
        
        if (gestureRecognized) {
            std::cout << "[GestureRecognizer] Recognized gesture: " 
                     << static_cast<int>(recognizedType) << " with score " << bestScore << std::endl;
            
            // Trigger the callback if one is registered
            if (m_gestureCallbacks.count(recognizedType) > 0) {
                m_gestureCallbacks[recognizedType](recognizedType);
            }
            
            return recognizedType;
        } else {
            std::cout << "[GestureRecognizer] No gesture recognized" << std::endl;
            return GestureType::FLAMMIL; // Default, not actually recognized
        }
    }
    
    void registerGestureCallback(GestureType type, GestureCallback callback) {
        m_gestureCallbacks[type] = callback;
        std::cout << "[GestureRecognizer] Registered callback for gesture " 
                 << static_cast<int>(type) << std::endl;
    }
    
    void setRecognitionThreshold(float threshold) {
        m_recognitionThreshold = std::max(0.0f, std::min(1.0f, threshold));
    }
    
    float getRecognitionThreshold() const {
        return m_recognitionThreshold;
    }
    
    bool isRecording() const {
        return m_isRecording;
    }
    
    const GesturePattern& getCurrentGesture() const {
        return m_currentGesture;
    }
    
private:
    void initializeGestureTemplates() {
        // Define template gestures for each type
        
        // FLAMMIL - Forward thrust (simple straight line)
        GesturePattern flammil;
        for (int i = 0; i < 5; i++) {
            float t = i / 4.0f;
            flammil.push_back(GesturePoint(glm::vec3(0.0f, 0.0f, t), t));
        }
        m_gestureTemplates[GestureType::FLAMMIL] = normalizeGesture(flammil);
        
        // AERETH - Circular motion
        GesturePattern aereth;
        for (int i = 0; i < 12; i++) {
            float angle = i * 2.0f * 3.14159f / 12.0f;
            float t = i / 11.0f;
            aereth.push_back(GesturePoint(
                glm::vec3(std::cos(angle), std::sin(angle), 0.0f), t));
        }
        m_gestureTemplates[GestureType::AERETH] = normalizeGesture(aereth);
        
        // TURANIS - Lightning bolt (zig-zag)
        GesturePattern turanis;
        turanis.push_back(GesturePoint(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f));
        turanis.push_back(GesturePoint(glm::vec3(0.5f, 0.5f, 0.0f), 0.2f));
        turanis.push_back(GesturePoint(glm::vec3(0.0f, 0.0f, 0.0f), 0.4f));
        turanis.push_back(GesturePoint(glm::vec3(0.5f, -0.5f, 0.0f), 0.6f));
        turanis.push_back(GesturePoint(glm::vec3(0.0f, -1.0f, 0.0f), 0.8f));
        m_gestureTemplates[GestureType::TURANIS] = normalizeGesture(turanis);
        
        // AQUOS - Wave pattern
        GesturePattern aquos;
        for (int i = 0; i < 10; i++) {
            float t = i / 9.0f;
            aquos.push_back(GesturePoint(
                glm::vec3(t, std::sin(t * 4.0f * 3.14159f), 0.0f), t));
        }
        m_gestureTemplates[GestureType::AQUOS] = normalizeGesture(aquos);
        
        // VITALIS - Spiral pattern
        GesturePattern vitalis;
        for (int i = 0; i < 16; i++) {
            float angle = i * 2.0f * 3.14159f / 8.0f;
            float radius = 0.1f + (i / 15.0f) * 0.9f;
            float t = i / 15.0f;
            vitalis.push_back(GesturePoint(
                glm::vec3(radius * std::cos(angle), radius * std::sin(angle), 0.0f), t));
        }
        m_gestureTemplates[GestureType::VITALIS] = normalizeGesture(vitalis);
        
        // UMBRIS - X pattern
        GesturePattern umbris;
        umbris.push_back(GesturePoint(glm::vec3(-1.0f, 1.0f, 0.0f), 0.0f));
        umbris.push_back(GesturePoint(glm::vec3(0.0f, 0.0f, 0.0f), 0.25f));
        umbris.push_back(GesturePoint(glm::vec3(1.0f, -1.0f, 0.0f), 0.5f));
        umbris.push_back(GesturePoint(glm::vec3(0.0f, 0.0f, 0.0f), 0.75f));
        umbris.push_back(GesturePoint(glm::vec3(-1.0f, -1.0f, 0.0f), 0.8f));
        umbris.push_back(GesturePoint(glm::vec3(0.0f, 0.0f, 0.0f), 0.9f));
        umbris.push_back(GesturePoint(glm::vec3(1.0f, 1.0f, 0.0f), 1.0f));
        m_gestureTemplates[GestureType::UMBRIS] = normalizeGesture(umbris);
        
        // TERRAE - Triangle pattern
        GesturePattern terrae;
        terrae.push_back(GesturePoint(glm::vec3(-0.866f, -0.5f, 0.0f), 0.0f));
        terrae.push_back(GesturePoint(glm::vec3(0.0f, 1.0f, 0.0f), 0.33f));
        terrae.push_back(GesturePoint(glm::vec3(0.866f, -0.5f, 0.0f), 0.67f));
        terrae.push_back(GesturePoint(glm::vec3(-0.866f, -0.5f, 0.0f), 1.0f));
        m_gestureTemplates[GestureType::TERRAE] = normalizeGesture(terrae);
    }
    
    GesturePattern normalizeGesture(const GesturePattern& gesture) {
        if (gesture.empty()) return {};
        
        // Calculate centroid
        glm::vec3 centroid(0.0f);
        for (const auto& point : gesture) {
            centroid += point.position;
        }
        centroid /= static_cast<float>(gesture.size());
        
        // Find the maximum distance from centroid for scaling
        float maxDist = 0.0f;
        for (const auto& point : gesture) {
            float dist = glm::distance(point.position, centroid);
            maxDist = std::max(maxDist, dist);
        }
        
        // Create normalized pattern (centered at origin, scaled to unit size)
        GesturePattern normalized;
        for (const auto& point : gesture) {
            GesturePoint normPoint = point;
            if (maxDist > 0.0001f) {
                normPoint.position = (point.position - centroid) / maxDist;
            } else {
                normPoint.position = point.position - centroid;
            }
            normalized.push_back(normPoint);
        }
        
        return normalized;
    }
    
    float compareGestures(const GesturePattern& gesture1, const GesturePattern& gesture2) {
        if (gesture1.empty() || gesture2.empty()) return 0.0f;
        
        // Simple distance-based comparison algorithm
        // We'll interpolate the smaller pattern to match the larger one's point count
        const GesturePattern& smallerGesture = (gesture1.size() <= gesture2.size()) ? gesture1 : gesture2;
        const GesturePattern& largerGesture = (gesture1.size() > gesture2.size()) ? gesture1 : gesture2;
        
        float totalDistance = 0.0f;
        float maxPossibleDistance = static_cast<float>(largerGesture.size()) * 2.0f; // Max distance is 2 for normalized patterns
        
        for (size_t i = 0; i < largerGesture.size(); i++) {
            float t = static_cast<float>(i) / static_cast<float>(largerGesture.size() - 1);
            
            // Find interpolated point in smaller gesture
            glm::vec3 interpolatedPos;
            if (smallerGesture.size() == 1) {
                interpolatedPos = smallerGesture[0].position;
            } else {
                float idx = t * static_cast<float>(smallerGesture.size() - 1);
                size_t idx1 = static_cast<size_t>(idx);
                size_t idx2 = std::min(idx1 + 1, smallerGesture.size() - 1);
                float frac = idx - static_cast<float>(idx1);
                
                interpolatedPos = glm::mix(
                    smallerGesture[idx1].position,
                    smallerGesture[idx2].position,
                    frac);
            }
            
            float distance = glm::distance(largerGesture[i].position, interpolatedPos);
            totalDistance += distance;
        }
        
        // Convert distance to similarity score (0 to 1)
        float averageDistance = totalDistance / static_cast<float>(largerGesture.size());
        float similarity = 1.0f - (averageDistance / 2.0f); // Divide by 2 since max distance is 2
        
        return similarity;
    }
    
    int64_t getCurrentTimeMillis() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    
    std::unordered_map<GestureType, GesturePattern> m_gestureTemplates;
    std::unordered_map<GestureType, GestureCallback> m_gestureCallbacks;
    GesturePattern m_currentGesture;
    bool m_isRecording = false;
    float m_recognitionThreshold = 0.7f;
    float m_minPointDistance = 0.05f;
};

class HandController {
public:
    HandController(std::shared_ptr<GestureRecognizer> recognizer = nullptr)
        : m_position(0.0f), m_velocity(0.0f), m_isGripping(false),
          m_gestureRecognizer(recognizer) {
        
        if (!m_gestureRecognizer) {
            m_gestureRecognizer = std::make_shared<GestureRecognizer>();
        }
        
        std::cout << "[HandController] Initialized" << std::endl;
    }
    
    void update(float deltaTime) {
        // Apply simple physics to hand position
        m_position += m_velocity * deltaTime;
        
        // Dampen velocity
        m_velocity *= 0.95f;
        
        // Record gesture point if currently drawing a gesture
        if (m_gestureRecognizer && m_gestureRecognizer->isRecording()) {
            m_gestureRecognizer->recordGesturePoint(m_position);
        }
    }
    
    void setPosition(const glm::vec3& position) {
        m_position = position;
    }
    
    void setVelocity(const glm::vec3& velocity) {
        m_velocity = velocity;
    }
    
    glm::vec3 getPosition() const {
        return m_position;
    }
    
    glm::vec3 getVelocity() const {
        return m_velocity;
    }
    
    void gripBegin() {
        m_isGripping = true;
        
        // Start gesture recording when grip begins
        if (m_gestureRecognizer) {
            m_gestureRecognizer->beginGesture();
        }
        
        std::cout << "[HandController] Grip began at " 
                 << m_position.x << ", " << m_position.y << ", " << m_position.z << std::endl;
    }
    
    void gripEnd() {
        m_isGripping = false;
        
        // End gesture recording when grip ends
        if (m_gestureRecognizer) {
            GestureType recognizedGesture = m_gestureRecognizer->endGesture();
            onGestureRecognized(recognizedGesture);
        }
        
        std::cout << "[HandController] Grip ended" << std::endl;
    }
    
    bool isGripping() const {
        return m_isGripping;
    }
    
    std::shared_ptr<GestureRecognizer> getGestureRecognizer() const {
        return m_gestureRecognizer;
    }
    
private:
    void onGestureRecognized(GestureType type) {
        std::cout << "[HandController] Gesture recognized: " << static_cast<int>(type) << std::endl;
        
        // Add specific behavior for each gesture type
        switch (type) {
            case GestureType::FLAMMIL:
                std::cout << "[HandController] Executing Flammil gesture - Quick forward thrust" << std::endl;
                break;
                
            case GestureType::AERETH:
                std::cout << "[HandController] Executing Aereth gesture - Protective air shield" << std::endl;
                break;
                
            case GestureType::TURANIS:
                std::cout << "[HandController] Executing Turanis gesture - Lightning bolt" << std::endl;
                break;
                
            case GestureType::AQUOS:
                std::cout << "[HandController] Executing Aquos gesture - Water wave" << std::endl;
                break;
                
            case GestureType::VITALIS:
                std::cout << "[HandController] Executing Vitalis gesture - Healing spiral" << std::endl;
                break;
                
            case GestureType::UMBRIS:
                std::cout << "[HandController] Executing Umbris gesture - Shadow X" << std::endl;
                break;
                
            case GestureType::TERRAE:
                std::cout << "[HandController] Executing Terrae gesture - Earth triangle" << std::endl;
                break;
        }
    }
    
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    bool m_isGripping;
    std::shared_ptr<GestureRecognizer> m_gestureRecognizer;
};

} // namespace Input
} // namespace TurtleEngine

// Helper function to create a test gesture
std::vector<TurtleEngine::Input::GesturePoint> createTestGesture(
    const std::vector<glm::vec3>& points, float startTime = 0.0f, float duration = 1.0f) {
    
    std::vector<TurtleEngine::Input::GesturePoint> gesture;
    
    for (size_t i = 0; i < points.size(); i++) {
        float t = (points.size() > 1) ? 
            (startTime + (duration * static_cast<float>(i) / static_cast<float>(points.size() - 1))) : 
            startTime;
        
        gesture.push_back(TurtleEngine::Input::GesturePoint(points[i], t));
    }
    
    return gesture;
}

// Test functions to simulate different gestures
void testFlammilGesture(TurtleEngine::Input::HandController& hand) {
    std::cout << "\nTesting FLAMMIL gesture (forward thrust)..." << std::endl;
    
    hand.setPosition(glm::vec3(0.0f, 0.0f, -0.5f));
    hand.gripBegin();
    
    // Simulate movement over time
    for (int i = 0; i < 10; i++) {
        hand.setPosition(glm::vec3(0.0f, 0.0f, -0.5f + i * 0.1f));
        hand.update(0.05f);
    }
    
    hand.gripEnd();
}

void testAerethGesture(TurtleEngine::Input::HandController& hand) {
    std::cout << "\nTesting AERETH gesture (circular motion)..." << std::endl;
    
    hand.setPosition(glm::vec3(0.5f, 0.0f, 0.0f));
    hand.gripBegin();
    
    // Simulate circular movement
    for (int i = 0; i < 20; i++) {
        float angle = i * 2.0f * 3.14159f / 20.0f;
        hand.setPosition(glm::vec3(0.5f * std::cos(angle), 0.5f * std::sin(angle), 0.0f));
        hand.update(0.05f);
    }
    
    hand.gripEnd();
}

void testTuranisGesture(TurtleEngine::Input::HandController& hand) {
    std::cout << "\nTesting TURANIS gesture (zig-zag lightning)..." << std::endl;
    
    hand.setPosition(glm::vec3(0.0f, 0.5f, 0.0f));
    hand.gripBegin();
    
    // Zigzag pattern
    std::vector<glm::vec3> zigzag = {
        glm::vec3(0.0f, 0.5f, 0.0f),
        glm::vec3(0.25f, 0.25f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.25f, -0.25f, 0.0f),
        glm::vec3(0.0f, -0.5f, 0.0f)
    };
    
    for (const auto& pos : zigzag) {
        hand.setPosition(pos);
        hand.update(0.1f);
    }
    
    hand.gripEnd();
}

void testNonMatchingGesture(TurtleEngine::Input::HandController& hand) {
    std::cout << "\nTesting non-matching gesture (random movement)..." << std::endl;
    
    hand.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    hand.gripBegin();
    
    // Random movement that doesn't match any template
    for (int i = 0; i < 10; i++) {
        hand.setPosition(glm::vec3(
            (rand() % 200 - 100) / 100.0f,
            (rand() % 200 - 100) / 100.0f,
            (rand() % 200 - 100) / 100.0f
        ));
        hand.update(0.05f);
    }
    
    hand.gripEnd();
}

int main() {
    std::cout << "=== GestureRecognizer Test ===\n";

    // Create gesture recognizer
    auto recognizer = std::make_shared<TurtleEngine::Input::GestureRecognizer>();
    
    // Create hand controller with the gesture recognizer
    TurtleEngine::Input::HandController handController(recognizer);
    
    // Test 1: Flammil gesture recognition
    std::cout << "\nTest 1: Flammil gesture recognition\n";
    testFlammilGesture(handController);
    
    // Test 2: Aereth gesture recognition
    std::cout << "\nTest 2: Aereth gesture recognition\n";
    testAerethGesture(handController);
    
    // Test 3: Turanis gesture recognition
    std::cout << "\nTest 3: Turanis gesture recognition\n";
    testTuranisGesture(handController);
    
    // Test 4: Test a gesture that shouldn't match any template
    std::cout << "\nTest 4: Non-matching gesture\n";
    testNonMatchingGesture(handController);
    
    // Test 5: Event-based gesture response
    std::cout << "\nTest 5: Event-based gesture response\n";
    
    // Register callbacks for specific gestures
    bool flammilTriggered = false;
    recognizer->registerGestureCallback(TurtleEngine::Input::GestureType::FLAMMIL, 
        [&flammilTriggered](TurtleEngine::Input::GestureType type) {
            std::cout << "[Test] Flammil gesture callback triggered" << std::endl;
            flammilTriggered = true;
        });
    
    // Test callback with Flammil gesture
    testFlammilGesture(handController);
    
    bool callbackTest = flammilTriggered;
    std::cout << "Callback Test: " << (callbackTest ? "PASSED" : "FAILED") << std::endl;
    
    // Test 6: Threshold adjustment
    std::cout << "\nTest 6: Threshold adjustment\n";
    
    // Set a very high threshold that should prevent recognition
    recognizer->setRecognitionThreshold(0.95f);
    std::cout << "Set recognition threshold to " << recognizer->getRecognitionThreshold() << std::endl;
    
    // Test Flammil again - should fail with high threshold
    testFlammilGesture(handController);
    
    // Set back to a more reasonable threshold
    recognizer->setRecognitionThreshold(0.6f);
    std::cout << "Set recognition threshold to " << recognizer->getRecognitionThreshold() << std::endl;
    
    // Test Flammil again - should succeed with lower threshold
    testFlammilGesture(handController);
    
    std::cout << "\n=== GestureRecognizer Test Complete ===\n";
    return 0;
} 