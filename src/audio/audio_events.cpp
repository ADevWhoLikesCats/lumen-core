#include "audio_events.h"

#ifdef LUMEN_HAVE_BULLET
#include <btBulletDynamicsCommon.h>
#include <algorithm>
#endif

ImpactQueue* gImpactQueue = nullptr;

#ifdef LUMEN_HAVE_BULLET
static bool contact_added(btManifoldPoint& cp,
                          const btCollisionObjectWrapper* aWrap,
                          int, int,
                          const btCollisionObjectWrapper* bWrap,
                          int, int)
{
    if (!gImpactQueue) return false;

    // Relative velocity at the contact point. Sum of the two bodies'
    // linear velocities projected onto the contact normal.
    const btCollisionObject* a = aWrap ? aWrap->getCollisionObject() : nullptr;
    const btCollisionObject* b = bWrap ? bWrap->getCollisionObject() : nullptr;
    const btRigidBody* rbA = a ? btRigidBody::upcast(a) : nullptr;
    const btRigidBody* rbB = b ? btRigidBody::upcast(b) : nullptr;

    btVector3 vA(0,0,0), vB(0,0,0);
    if (rbA) vA = rbA->getLinearVelocity();
    if (rbB) vB = rbB->getLinearVelocity();
    btVector3 vRel = vA - vB;

    float speed = std::abs(vRel.dot(cp.m_normalWorldOnB));
    // Threshold: below this we'd get a sound for resting contacts.
    if (speed < 0.8f) return false;

    // Map to 0..1. 8 m/s is a fairly hard impact; scale from there.
    float strength = std::min(speed / 8.0f, 1.f);

    btVector3 p = cp.getPositionWorldOnB();
    ImpactEvent e;
    e.position = glm::vec3(p.x(), p.y(), p.z());
    e.strength = strength;
    gImpactQueue->push(e);
    return false;   // don't modify the contact
}
#endif

void audio_events_install() {
#ifdef LUMEN_HAVE_BULLET
    gContactAddedCallback = contact_added;
#endif
}
