#include "vehicle.h"
#include "physics_world.h"
#include "transform.h"
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <BulletDynamics/Vehicle/btVehicleRaycaster.h>

#ifdef LUMEN_HAVE_BULLET

#include <cstdio>

bool Vehicle::init(PhysicsWorld* phys, const glm::mat4& spawnXf,
                   const Config& cfg)
{
    m_phys = phys;
    m_cfg  = cfg;

    // --- chassis shape + body ---
    // The visible mesh's origin is NOT at the box center — the box needs
    // to sit at bodyCenterLocal inside the mesh. We use a compound shape
    // so the physics body origin matches the mesh origin exactly. That
    // way wheels (added in body-local space) and the body mesh share a
    // single coordinate frame.
    btVector3 he(cfg.halfExtents.x, cfg.halfExtents.y, cfg.halfExtents.z);
    btBoxShape* box = new btBoxShape(he);

    btCompoundShape* shape = new btCompoundShape();
    btTransform boxLocal;
    boxLocal.setIdentity();
    boxLocal.setOrigin(btVector3(cfg.bodyCenterLocal.x,
                                 cfg.bodyCenterLocal.y,
                                 cfg.bodyCenterLocal.z));
    shape->addChildShape(boxLocal, box);

    btVector3 inertia(0, 0, 0);
    shape->calculateLocalInertia(cfg.mass, inertia);

    btTransform t = glmToBt(spawnXf);
    btDefaultMotionState* motion = new btDefaultMotionState(t);

    btRigidBody::btRigidBodyConstructionInfo ci(cfg.mass, motion, shape, inertia);
    m_chassis = new btRigidBody(ci);
    m_chassis->setFriction(1.0f);
    m_chassis->setRestitution(0.1f);
    m_chassis->setDamping(0.1f, 0.3f);

    phys->world()->addRigidBody(m_chassis);

    // --- vehicle ---
    m_raycaster = new btDefaultVehicleRaycaster(phys->world());
    m_vehicle   = new btRaycastVehicle(m_tuning, m_chassis, m_raycaster);

    m_chassis->setActivationState(DISABLE_DEACTIVATION);
    phys->world()->addAction(m_vehicle);

    // --- tuning: sporty-ish ---
    m_tuning.m_suspensionStiffness = 40.f;
    m_tuning.m_suspensionDamping   = 2.3f;
    m_tuning.m_suspensionCompression = 4.4f;
    m_tuning.m_maxSuspensionTravelCm = 30.f;
    m_tuning.m_frictionSlip        = 3.0f;
    m_tuning.m_maxSuspensionForce  = 6000.f;

    m_vehicle->setCoordinateSystem(0, 1, 2);  // X right, Y up, Z forward

    // --- wheels ---
    btVector3 wheelDirection(0, -1, 0);
    btVector3 wheelAxle(-1, 0, 0);

    for (int i = 0; i < 4; ++i) {
        btVector3 conn(cfg.wheelPositions[i].x,
                       cfg.wheelPositions[i].y,
                       cfg.wheelPositions[i].z);

        m_vehicle->addWheel(conn, wheelDirection, wheelAxle,
                            0.30f /* suspension rest length, m */,
                            cfg.wheelRadii[i],
                            m_tuning,
                            cfg.frontWheel[i]);

        // The visual mesh's origin is the car origin, not the wheel centre.
        // When drawing, we want to place the wheel at its world position
        // and rotate around its own centre.
        m_wheelDrawOffset[i] = -cfg.wheelCenters[i];
    }

    for (int i = 0; i < m_vehicle->getNumWheels(); ++i) {
        m_vehicle->getWheelInfo(i).m_suspensionRestLength1 = 0.30f;
    }

    std::fprintf(stderr, "[vehicle] chassis mass=%.0fkg wheels=%d\n",
                 cfg.mass, m_vehicle->getNumWheels());
    return true;
}

void Vehicle::shutdown() {
    if (m_vehicle) {
        if (m_phys && m_phys->world()) m_phys->world()->removeAction(m_vehicle);
        // chassis rigid body is owned by PhysicsWorld, do not delete here.
        delete m_vehicle;
        m_vehicle = nullptr;
    }
    if (m_raycaster) {
        delete m_raycaster;
        m_raycaster = nullptr;
    }
    m_chassis = nullptr;
}

void Vehicle::setInput(float throttle, float steering, float brake) {
    m_throttle = throttle;
    m_steering = steering;
    m_brake    = brake;
}

void Vehicle::update(float dt) {
    if (!m_vehicle) return;

    const int wheelCount = m_vehicle->getNumWheels();
    const float engineForce = 2500.f;   // N, per driven wheel
    const float brakeForce  = 30.f;

    // Apply engine + steering + brake.
    for (int i = 0; i < wheelCount; ++i) {
        bool front = m_cfg.frontWheel[i];

        // All-wheel drive: every wheel gets engine force.
        float ef = m_throttle * engineForce;
        if (m_brake > 0.f) ef = -m_brake * brakeForce;

        m_vehicle->applyEngineForce(ef, i);

        if (front) {
            m_vehicle->setSteeringValue(m_steering * 0.4f, i);
        }
    }

    // Bullet does the work here. This must be called *after* the world step
    // for the vehicle to see the updated physics, but Bullet's raycast
    // vehicle is happy either way as long as dt is consistent.
    m_vehicle->updateVehicle(dt);
}

glm::mat4 Vehicle::chassisTransform() const {
    if (!m_chassis) return glm::mat4(1.f);
    btTransform t;
    m_chassis->getMotionState()->getWorldTransform(t);
    // Physics body origin == mesh origin (see init, which uses a
    // btCompoundShape with the box offset inside). No correction needed.
    return btToGlm(t);
}

glm::mat4 Vehicle::wheelTransform(int i) const {
    if (!m_vehicle || i < 0 || i >= 4) return glm::mat4(1.f);
    btWheelInfo& wi = m_vehicle->getWheelInfo(i);
    btTransform w = wi.m_worldTransform;

    // The glb wheel meshes are modeled with pivots at car origin. To place
    // them at the wheel world transform, apply the draw offset in the
    // wheel's local frame.
    glm::mat4 world  = btToGlm(w);
    glm::mat4 offset = glm::translate(glm::mat4(1.f), m_wheelDrawOffset[i]);
    return world * offset;
}

#endif
