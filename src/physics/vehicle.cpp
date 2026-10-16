#include "vehicle.h"
#include "physics_world.h"
#include "transform.h"

#ifdef LUMEN_HAVE_BULLET

#include <cstdio>
#include <cmath>
#include <algorithm>

bool Vehicle::init(PhysicsWorld* phys, const glm::mat4& spawnXf, const Config& cfg)
{
    m_phys = phys;
    m_cfg  = cfg;

    // ---- Collision shape: chassis box + skid plate ----
    btCompoundShape* compound = new btCompoundShape();

    btBoxShape* box = new btBoxShape(btVector3(
        cfg.halfExtents.x, cfg.halfExtents.y, cfg.halfExtents.z));
    btTransform boxT;
    boxT.setIdentity();
    boxT.setOrigin(btVector3(cfg.bodyCenterLocal.x,
                             cfg.bodyCenterLocal.y,
                             cfg.bodyCenterLocal.z));
    compound->addChildShape(boxT, box);

    // Skid plate: bottom at the lowest wheel bottom. This is what the
    // chassis rests on and collides with ramps against.
    float lowest = cfg.wheelCenters[0].y - cfg.wheelRadii[0];
    for (int i = 1; i < 4; ++i)
        lowest = std::min(lowest, cfg.wheelCenters[i].y - cfg.wheelRadii[i]);

    glm::vec3 wmin = cfg.wheelCenters[0], wmax = cfg.wheelCenters[0];
    for (int i = 1; i < 4; ++i) {
        wmin = glm::min(wmin, cfg.wheelCenters[i]);
        wmax = glm::max(wmax, cfg.wheelCenters[i]);
    }
    glm::vec3 plateHalf(
        (wmax.x - wmin.x) * 0.5f + 0.10f,
        0.05f,
        (wmax.z - wmin.z) * 0.5f + 0.10f);
    glm::vec3 plateC = (wmin + wmax) * 0.5f;
    plateC.y = lowest + plateHalf.y;

    btBoxShape* plate = new btBoxShape(
        btVector3(plateHalf.x, plateHalf.y, plateHalf.z));
    btTransform plateT;
    plateT.setIdentity();
    plateT.setOrigin(btVector3(plateC.x, plateC.y, plateC.z));
    compound->addChildShape(plateT, plate);

    m_shape = compound;

    // ---- Kinematic rigid body ----
    glm::vec3 startPos(spawnXf[3]);
    m_pos = startPos;
    m_yaw = 0.f;
    m_speed = 0.f;
    m_velDir = glm::vec3(0.f, 0.f, 1.f);   // face +Z at spawn

    btTransform t0;
    t0.setIdentity();
    t0.setOrigin(btVector3(m_pos.x, m_pos.y, m_pos.z));

    btDefaultMotionState* motion = new btDefaultMotionState(t0);
    btVector3 inertia(0, 0, 0);
    m_shape->calculateLocalInertia(cfg.mass, inertia);
    btRigidBody::btRigidBodyConstructionInfo ci(
        cfg.mass, motion, m_shape, inertia);
    m_chassis = new btRigidBody(ci);

    // Dynamic body: no kinematic flag.
    m_chassis->setActivationState(DISABLE_DEACTIVATION);

    phys->world()->addRigidBody(m_chassis);

    std::fprintf(stderr, "[vehicle] kinematic chassis, spawn=(%.2f %.2f %.2f)\n",
                 m_pos.x, m_pos.y, m_pos.z);
    return true;
}

void Vehicle::shutdown() {
    m_chassis = nullptr;
    m_shape = nullptr;
}

void Vehicle::setInput(float throttle, float steering, float brake) {
    m_throttle = std::max(-1.f, std::min(1.f, throttle));
    m_steering = std::max(-1.f, std::min(1.f, steering));
    m_brake    = std::max(0.f,  std::min(1.f, brake));
}

void Vehicle::update(float dt) {
    if (!m_chassis) return;

    // ---- Longitudinal: throttle, brake, drag ----
    if (m_brake > 0.f) {
        // Hard stop toward zero.
        float decel = m_cfg.brakeDecel * m_brake;
        if (m_speed > 0.f)      m_speed = std::max(0.f,  m_speed - decel * dt);
        else if (m_speed < 0.f) m_speed = std::min(0.f,  m_speed + decel * dt);
    } else if (m_throttle > 0.f) {
        m_speed += m_cfg.accel * m_throttle * dt;
        if (m_speed > m_cfg.maxSpeed) m_speed = m_cfg.maxSpeed;
    } else if (m_throttle < 0.f) {
        // Moving forward: brake first. Otherwise, reverse.
        if (m_speed > 0.5f) {
            m_speed -= m_cfg.brakeDecel * dt;
            if (m_speed < 0.f) m_speed = 0.f;
        } else {
            m_speed -= m_cfg.accel * 0.4f * dt;
            if (m_speed < -m_cfg.maxReverse) m_speed = -m_cfg.maxReverse;
        }
    } else {
        // Coast: drag toward zero.
        if (m_speed > 0.f)      m_speed = std::max(0.f, m_speed - m_cfg.coastDrag * dt);
        else if (m_speed < 0.f) m_speed = std::min(0.f, m_speed + m_cfg.coastDrag * dt);
    }

    // ---- Steering ----
    // Speed-gated so the car can't yaw in place.
    float speedAbs = std::abs(m_speed);
    float authority = 0.f;
    if (speedAbs > m_cfg.steerLowSpeed) {
        float range = std::max(0.001f,
            m_cfg.steerFullSpeed - m_cfg.steerLowSpeed);
        authority = std::min(
            (speedAbs - m_cfg.steerLowSpeed) / range, 1.f);
    }

    // Reverse steers the other way.
    float dir = (m_speed >= 0.f) ? 1.f : -1.f;
    float yawRate =  m_steering * m_cfg.maxYawRate * authority * dir;
    m_yaw += yawRate * dt;

    // ---- Drift: velocity direction lags behind facing ----
    glm::vec3 forward(std::sin(m_yaw), 0.f, std::cos(m_yaw));

    // Only blend velocity direction when moving. At near-zero speed there's
    // no meaningful direction, and blending would produce jitter.
    if (std::abs(m_speed) > 0.5f) {
        // Lerp velocity direction toward facing. Bigger grip => snappier.
        // grip * dt, capped at 1.0 so a single step can't over-rotate.
        float a = std::min(1.f, m_cfg.grip * dt * 60.f);

        // Reverse case: if speed is negative, we're travelling backward,
        // so the target direction is -forward.
        glm::vec3 target = (m_speed >= 0.f) ? forward : -forward;

        glm::vec3 blended = glm::mix(m_velDir, target, a);
        float len = glm::length(blended);
        if (len > 1e-4f) m_velDir = blended / len;
    } else {
        // Stopped: face where we're pointing.
        m_velDir = (m_speed >= 0.f) ? forward : -forward;
    }

    // ---- Integrate position along velocity direction ----
    m_pos += m_velDir * m_speed * dt;

    // ---- Push new transform into the physics body ----
    btTransform t;
    t.setIdentity();
    t.setOrigin(btVector3(m_pos.x, m_pos.y, m_pos.z));
    btQuaternion q(btVector3(0, 1, 0), m_yaw);
    t.setRotation(q);

    m_chassis->setWorldTransform(t);
    m_chassis->getMotionState()->setWorldTransform(t);

    // Also set the body's velocity so the solver sees a moving body.
    m_chassis->setLinearVelocity(
        btVector3(m_velDir.x * m_speed, 0.f, m_velDir.z * m_speed));

    // Expose the actual travel velocity to the physics body so collisions
    // with ramps interact correctly.
    m_chassis->setLinearVelocity(
        btVector3(m_velDir.x * m_speed, 0.f, m_velDir.z * m_speed));

    // ---- Wheel spin visual ----
    m_wheelSpin += (m_speed / std::max(m_cfg.wheelRadii[0], 0.05f)) * dt;
    while (m_wheelSpin >  6.2831853f) m_wheelSpin -= 6.2831853f;
    while (m_wheelSpin < -6.2831853f) m_wheelSpin += 6.2831853f;
}

glm::mat4 Vehicle::chassisTransform() const {
    glm::mat4 M(1.f);
    M = glm::translate(M, m_pos);
    M = glm::rotate(M, m_yaw, glm::vec3(0, 1, 0));
    return M;
}

glm::mat4 Vehicle::wheelTransform(int i) const {
    if (i < 0 || i >= 4) return glm::mat4(1.f);

    glm::mat4 chassis = chassisTransform();

    const glm::vec3& c = m_cfg.wheelCenters[i];
    glm::mat4 toCenter = glm::translate(glm::mat4(1.f),  c);
    glm::mat4 toOrigin = glm::translate(glm::mat4(1.f), -c);
    glm::mat4 spin     = glm::rotate(glm::mat4(1.f), m_wheelSpin,
                                     glm::vec3(1.f, 0.f, 0.f));

    return chassis * toCenter * spin * toOrigin;
}

#endif
