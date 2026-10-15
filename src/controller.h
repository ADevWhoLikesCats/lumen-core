#pragma once
#include "collider.h"
#include "collision.h"
#include "physics_world.h"
#include <btBulletDynamicsCommon.h>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <vector>
#include <algorithm>

struct CharacterController {
    glm::vec3 pos{0, 1.0f, 5};
    float height     = 1.7f;
    float radius     = 0.3f;
    float eyeOffset  = 0.65f;
    bool  grounded   = false;

    Capsule capsule() const {
        Capsule c;
        float halfSeg = std::max(0.0f, (height - 2.f*radius)) * 0.5f;
        c.A = pos - glm::vec3(0, halfSeg, 0);
        c.B = pos + glm::vec3(0, halfSeg, 0);
        c.r = radius;
        return c;
    }

    glm::vec3 eye() const { return pos + glm::vec3(0, eyeOffset, 0); }

    void move(const glm::vec3& desired, PhysicsWorld* phys) {
        grounded = false;
        glm::vec3 remaining = desired;

        // Bullet capsule: axis is Y, total length is `height`, radius `radius`.
        // btCapsuleShape takes the half-height of the cylindrical section.
        float halfCyl = std::max(0.0f, (height - 2.f*radius)) * 0.5f;
        btCapsuleShape shape(radius, 2.0f * halfCyl);

        // Up to 4 planes so we can resolve a 3-face corner cleanly.
        glm::vec3 planes[4];
        int nPlanes = 0;

        for (int iter = 0; iter < 4; ++iter) {
            if (glm::dot(remaining, remaining) < 1e-8f) break;

            // Build from/to transforms for the sweep. Capsule local origin
            // is its centre, so world centre = pos.
            btTransform from, to;
            from.setIdentity();
            to.setIdentity();
            from.setOrigin(btVector3(pos.x, pos.y, pos.z));
            to.setOrigin(btVector3(pos.x + remaining.x,
                                   pos.y + remaining.y,
                                   pos.z + remaining.z));

            // Ignore the player-vs-player case; there's only one player, so
            // sweep against everything in the world.
            btCollisionWorld::ClosestConvexResultCallback cb(
                btVector3(pos.x, pos.y, pos.z),
                btVector3(pos.x + remaining.x,
                          pos.y + remaining.y,
                          pos.z + remaining.z));
            cb.m_collisionFilterGroup = btBroadphaseProxy::AllFilter;
            cb.m_collisionFilterMask  = btBroadphaseProxy::StaticFilter;

            phys->sweepConvex(&shape, from, to, cb);

            float bestT = cb.m_closestHitFraction;
            glm::vec3 bestN(0.f);
            bool any = cb.hasHit() &&
                       (bestT < 1.f - 1e-6f);
            if (any) {
                const btVector3& n = cb.m_hitNormalWorld;
                bestN = glm::vec3(n.x(), n.y(), n.z());
            }

            // Advance close to contact, then push out along N by a skin
            // so the next sweep starts with real clearance.
            float safeT = std::max(0.f, bestT - 1e-3f);
            pos += remaining * safeT;

            if (!any) break;

            pos += bestN * 1e-3f;

            if (bestN.y > 0.7f) grounded = true;

            bool dup = false;
            for (int i = 0; i < nPlanes; ++i) {
                if (glm::dot(planes[i], bestN) > 0.99f) { dup = true; break; }
            }
            if (!dup && nPlanes < 4) planes[nPlanes++] = bestN;

            remaining = remaining * (1.f - bestT);
            for (int i = 0; i < nPlanes; ++i) {
                float d = glm::dot(remaining, planes[i]);
                if (d < 0.f) remaining -= planes[i] * d;
            }
        }
    }
};
