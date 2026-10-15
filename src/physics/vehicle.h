#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef LUMEN_HAVE_BULLET
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Vehicle/btWheelInfo.h>
#include <BulletDynamics/Vehicle/btVehicleRaycaster.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>

class PhysicsWorld;

// Wraps btRaycastVehicle + four wheels. The chassis is a box; wheels are
// raycast against the ground and produce spring/drive/steer forces.
//
// Units: metres, seconds, kilograms. The glb is loaded in centimetres, so
// the caller must scale vertices by 0.01 before handing them to us.
class Vehicle {
public:
    // halfExtents: chassis box half-sizes, in metres, in chassis local space.
    // bodyCenterLocal: centre of the visible body relative to the model
    //   origin, in metres. The physics box is centered here so it sits
    //   inside the mesh.
    // wheelPositions: four positions in metres, chassis local space.
    //   Order: FL, FR, RL, RR.
    // wheelCenters: for each wheel, the position of the wheel mesh's centre
    //   in the wheel's own local space (the glb models them at car origin).
    //   Used to correct the draw transform.
    // wheelRadii: wheel radius in metres.
    struct Config {
        glm::vec3 halfExtents;
        glm::vec3 bodyCenterLocal;
        glm::vec3 wheelPositions[4];
        glm::vec3 wheelCenters[4];
        float     wheelRadii[4];
        float     mass = 1500.f;
        bool      frontWheel[4] = { true, true, false, false };
    };

    bool init(PhysicsWorld* phys, const glm::mat4& spawnXf, const Config& cfg);
    void shutdown();

    // Advance the vehicle. Must be called inside the fixed-timestep loop,
    // after PhysicsWorld::step().
    void update(float dt);

    // Drive inputs. Call before update().
    void setInput(float throttle, float steering, float brake);

    glm::mat4 chassisTransform() const;
    glm::mat4 wheelTransform(int i) const;   // 0..3 = FL FR RL RR

    btRigidBody* chassis() { return m_chassis; }

private:
    PhysicsWorld*         m_phys = nullptr;
    btRigidBody*          m_chassis = nullptr;
    btRaycastVehicle*     m_vehicle = nullptr;
    btVehicleRaycaster*   m_raycaster = nullptr;
    btRaycastVehicle::btVehicleTuning m_tuning;
    Config                m_cfg;

    // Cached wheel world-centres (from wheelPositions, in chassis local)
    // so we can offset the visual draw.
    glm::vec3 m_wheelDrawOffset[4];

    float m_throttle = 0.f;
    float m_steering = 0.f;
    float m_brake = 0.f;
};

#endif
