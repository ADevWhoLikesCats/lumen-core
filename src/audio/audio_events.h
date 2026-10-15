#pragma once
#include <glm/glm.hpp>
#include <vector>

struct ImpactEvent {
    glm::vec3 position;
    float     strength = 0.f;   // 0..1, from relative velocity at contact
};

// Single-threaded, main-thread-only queue. Physics callbacks push,
// the main loop drains. No locking needed in the current design.
class ImpactQueue {
public:
    void push(const ImpactEvent& e) { m_pending.push_back(e); }

    void drain(std::vector<ImpactEvent>& out) {
        out.swap(m_pending);
        m_pending.clear();
    }

private:
    std::vector<ImpactEvent> m_pending;
};

// Global pointer so Bullet's C-style contact callback can reach the queue.
// One queue per process; we only have one physics world.
extern ImpactQueue* gImpactQueue;

// Register Bullet's gContactAddedCallback. Call once after PhysicsWorld::init().
void audio_events_install();
