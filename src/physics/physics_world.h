#pragma once

#ifdef LUMEN_HAVE_BULLET

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "collider.h"

class PhysicsWorld {
public:
    void init();
    void shutdown();

    void step(float dt);

    void addStaticOBB(const OBB& obb);
    void addStaticBox(const glm::vec3& halfExtents, const glm::mat4& xf);

    btRigidBody* addDynamicBox(const glm::vec3& halfExtents,
                               const glm::mat4& xf,
                               float mass);

    void sweepConvex(const btConvexShape* shape,
                     const btTransform& from,
                     const btTransform& to,
                     btCollisionWorld::ClosestConvexResultCallback& cb);

    btDiscreteDynamicsWorld* world()             { return m_world.get(); }
    const btDiscreteDynamicsWorld* world() const { return m_world.get(); }

private:
    std::unique_ptr<btBroadphaseInterface>               m_broadphase;
    std::unique_ptr<btDefaultCollisionConfiguration>     m_config;
    std::unique_ptr<btCollisionDispatcher>               m_dispatcher;
    std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
    std::unique_ptr<btDiscreteDynamicsWorld>             m_world;

    std::vector<std::unique_ptr<btCollisionShape>> m_shapes;
    std::vector<std::unique_ptr<btMotionState>>    m_motions;
    std::vector<std::unique_ptr<btRigidBody>>      m_bodies;
};

#endif
