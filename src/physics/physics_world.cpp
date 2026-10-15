#include "physics_world.h"

#ifdef LUMEN_HAVE_BULLET

#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <cstring>

void PhysicsWorld::init() {
    m_broadphase = std::make_unique<btDbvtBroadphase>();
    m_config     = std::make_unique<btDefaultCollisionConfiguration>();
    m_dispatcher = std::make_unique<btCollisionDispatcher>(m_config.get());
    m_solver     = std::make_unique<btSequentialImpulseConstraintSolver>();

    m_world = std::make_unique<btDiscreteDynamicsWorld>(
        m_dispatcher.get(), m_broadphase.get(),
        m_solver.get(), m_config.get());
    m_world->setGravity(btVector3(0.f, -20.f, 0.f));

    std::fprintf(stderr, "[physics] Bullet world initialised\n");
}

void PhysicsWorld::shutdown() {
    m_bodies.clear();
    m_motions.clear();
    m_shapes.clear();
    m_world.reset();
    m_solver.reset();
    m_dispatcher.reset();
    m_config.reset();
    m_broadphase.reset();
}

void PhysicsWorld::step(float dt) {
    if (!m_world) return;
    m_world->stepSimulation(dt, 2, dt);
}

void PhysicsWorld::addStaticBox(const glm::vec3& halfExtents,
                                const glm::mat4& xf) {
    auto shape = std::make_unique<btBoxShape>(
        btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
    btVector3 inertia(0, 0, 0);
    shape->calculateLocalInertia(0.f, inertia);

    btScalar bm[16];
    std::memcpy(bm, glm::value_ptr(xf), sizeof(float) * 16);
    btTransform t;
    t.setFromOpenGLMatrix(bm);

    auto motion = std::make_unique<btDefaultMotionState>(t);

    btRigidBody::btRigidBodyConstructionInfo ci(
        0.f, motion.get(), shape.get(), inertia);
    auto body = std::make_unique<btRigidBody>(ci);
    body->setFriction(0.8f);

    m_world->addRigidBody(body.get(),
                          btBroadphaseProxy::StaticFilter,
                          btBroadphaseProxy::AllFilter);

    m_shapes.push_back(std::move(shape));
    m_motions.push_back(std::move(motion));
    m_bodies.push_back(std::move(body));
}

void PhysicsWorld::addStaticOBB(const OBB& obb) {
    glm::mat4 xf(1.f);
    xf[0] = glm::vec4(obb.X, 0.f);
    xf[1] = glm::vec4(obb.Y, 0.f);
    xf[2] = glm::vec4(obb.Z, 0.f);
    xf[3] = glm::vec4(obb.C, 1.f);
    addStaticBox(obb.H, xf);
}

btRigidBody* PhysicsWorld::addDynamicBox(const glm::vec3& halfExtents,
                                         const glm::mat4& xf,
                                         float mass) {
    auto shape = std::make_unique<btBoxShape>(
        btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
    btVector3 inertia(0, 0, 0);
    if (mass > 0.f) shape->calculateLocalInertia(mass, inertia);

    btScalar bm[16];
    std::memcpy(bm, glm::value_ptr(xf), sizeof(float) * 16);
    btTransform t;
    t.setFromOpenGLMatrix(bm);

    auto motion = std::make_unique<btDefaultMotionState>(t);

    btRigidBody::btRigidBodyConstructionInfo ci(
        mass, motion.get(), shape.get(), inertia);
    auto body = std::make_unique<btRigidBody>(ci);
    body->setFriction(0.8f);
    body->setRestitution(0.1f);

    btRigidBody* raw = body.get();
    m_world->addRigidBody(raw);

    m_shapes.push_back(std::move(shape));
    m_motions.push_back(std::move(motion));
    m_bodies.push_back(std::move(body));
    return raw;
}

void PhysicsWorld::sweepConvex(
    const btConvexShape* shape,
    const btTransform& from,
    const btTransform& to,
    btCollisionWorld::ClosestConvexResultCallback& cb)
{
    if (!m_world) return;
    m_world->convexSweepTest(shape, from, to, cb);
}

#endif
