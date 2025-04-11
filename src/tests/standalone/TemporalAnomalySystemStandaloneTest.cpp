#include <iostream>
#include <glm/glm.hpp>
#include "../../engine/temporal/include/TemporalAnomalySystem.hpp"

class TestEntity : public TurtleEngine::Temporal::AffectedEntity {
public:
    TestEntity(const std::string& id, const glm::vec3& position)
        : m_id(id), m_position(position), m_timeScale(1.0f) {}

    void applyTemporalEffect(const TurtleEngine::Temporal::AnomalyEffect& effect, float deltaTime) override {
        m_timeScale = effect.timeDistortion;
        std::cout << "Entity " << m_id << " affected by temporal anomaly. Time scale: " << m_timeScale << "\n";
    }

    glm::vec3 getPosition() const override { return m_position; }
    std::string getIdentifier() const override { return m_id; }

    float getTimeScale() const { return m_timeScale; }

private:
    std::string m_id;
    glm::vec3 m_position;
    float m_timeScale;
};

int main() {
    std::cout << "=== TemporalAnomalySystem Standalone Test ===\n";

    // Create a TemporalAnomalySystem instance
    auto anomalySystem = std::make_shared<TurtleEngine::Temporal::TemporalAnomalySystem>();

    // Create a test entity
    auto entity = std::make_shared<TestEntity>("TestEntity", glm::vec3(1.0f, 0.0f, 0.0f));
    anomalySystem->registerEntity(entity);

    // Test 1: Create and apply a Dilation anomaly
    std::cout << "\nTest 1: Create and apply Dilation anomaly\n";
    TurtleEngine::Temporal::AnomalyEffect dilationEffect(
        TurtleEngine::Temporal::AnomalyType::DILATION,
        0.5f, // Slowdown to 50%
        glm::vec3(0.0f),
        2.0f, // 2m radius
        1.0f, // 1s duration
        "DilationTest"
    );
    anomalySystem->createAnomaly(dilationEffect);
    anomalySystem->update(0.1f);
    bool dilationSuccess = (entity->getTimeScale() == 0.5f);
    std::cout << "Dilation Test: " << (dilationSuccess ? "PASSED" : "FAILED") 
              << " (Time Scale: " << entity->getTimeScale() << ")\n";

    // Test 2: Create and apply a Rift anomaly
    std::cout << "\nTest 2: Create and apply Rift anomaly\n";
    TurtleEngine::Temporal::AnomalyEffect riftEffect(
        TurtleEngine::Temporal::AnomalyType::RIFT,
        1.5f, // Chaotic distortion
        glm::vec3(0.0f),
        2.0f,
        1.0f,
        "RiftTest"
    );
    anomalySystem->createAnomaly(riftEffect);
    anomalySystem->update(0.1f);
    bool riftSuccess = (entity->getTimeScale() != 1.0f); // Rift should apply some distortion
    std::cout << "Rift Test: " << (riftSuccess ? "PASSED" : "FAILED") 
              << " (Time Scale: " << entity->getTimeScale() << ")\n";

    std::cout << "\n=== TemporalAnomalySystem Test Complete ===\n";
    return (dilationSuccess && riftSuccess) ? 0 : 1;
} 