#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef LUMEN_HAVE_BULLET
#include <btBulletDynamicsCommon.h>

class PhysicsWorld;

// Arcade vehicle. The chassis is a kinematic rigid body: code owns its
// position and rotation, physics only handles collisions with other
// objects (ramps, walls). No suspension, no raycast vehicle, no solver
// fighting. This makes handling predictable and removes every class of
// "car stops when turning" bug.
class Vehicle {
public:
    struct Config {
        glm::vec3 halfExtents{1.0f, 0.45f, 2.4f};   // chassis box, metres
        glm::vec3 bodyCenterLocal{0.f};             // box offset in mesh space
        glm::vec3 wheelCenters[4]{};                 // mesh-space wheel pivots
        float     wheelRadii[4]{};

        // Handling tunables
        float maxSpeed       = 40.f;   // m/s forward
        float maxReverse     = 12.f;   // m/s backward
        float accel          = 12.f;   // m/s^2
        float brakeDecel     = 25.f;   // m/s^2
        float coastDrag      = 0.6f;   // 1/s
        float maxYawRate     = 2.0f;   // rad/s at full authority
        float steerLowSpeed  = 1.5f;   // below this, no steering
        float steerFullSpeed = 8.0f;   // above this, full steering
        // Drift: how quickly velocity snaps to the car's facing.
        // 1.0 = no drift (on rails). 0.05 = very drifty. 0.15 = sporty.
        float grip = 0.15f;
    };

    bool init(PhysicsWorld* phys, const glm::mat4& spawnXf, const Config& cfg);
    void shutdown();

    // Advance one fixed step. Reads inputs, integrates position/rotation,
    // writes the new transform to the physics body.
    void update(float dt);

    void setInput(float throttle, float steering, float brake);

    glm::mat4 chassisTransform() const;
    glm::mat4 wheelTransform(int i) const;   // 0..3 = FL FR RL RR

    btRigidBody* chassis() { return m_chassis; }

private:
    PhysicsWorld* m_phys = nullptr;
    btRigidBody*  m_chassis = nullptr;
    btCollisionShape* m_shape = nullptr;
    Config m_cfg;

    // Code-owned state. This is the authoritative position and rotation;
    // the physics body is kept in sync each update.
    glm::vec3 m_pos{0.f};
    float     m_yaw = 0.f;         // radians about world Y
    float     m_speed = 0.f;       // signed, m/s along m_velDir
    glm::vec3 m_velDir{0.f, 0.f, 1.f};  // unit direction of travel (may differ from facing)

    // Visual only
    float m_wheelSpin = 0.f;

    float m_throttle = 0.f;
    float m_steering = 0.f;
    float m_brake    = 0.f;
};

#endif
