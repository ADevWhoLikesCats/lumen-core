#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstring>

#ifdef LUMEN_HAVE_BULLET
#include <btBulletDynamicsCommon.h>

inline glm::mat4 btToGlm(const btTransform& t) {
    btScalar m[16];
    t.getOpenGLMatrix(m);
    glm::mat4 out;
    static_assert(sizeof(btScalar) == sizeof(float), "btScalar must be float");
    std::memcpy(glm::value_ptr(out), m, sizeof(float) * 16);
    return out;
}

inline btTransform glmToBt(const glm::mat4& m) {
    btTransform t;
    btScalar bm[16];
    std::memcpy(bm, glm::value_ptr(m), sizeof(float) * 16);
    t.setFromOpenGLMatrix(bm);
    return t;
}
#endif
