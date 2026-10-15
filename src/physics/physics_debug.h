#pragma once
#ifdef LUMEN_HAVE_BULLET

#include <btBulletDynamicsCommon.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class PhysicsDebugDraw : public btIDebugDraw {
public:
    void init();
    void shutdown();
    void begin();
    void flush(const glm::mat4& viewProj);

    void drawLine(const btVector3& from, const btVector3& to,
                  const btVector3& color) override;
    void drawContactPoint(const btVector3&, const btVector3&, btScalar,
                          int, const btVector3&) override {}
    void reportErrorWarning(const char*) override {}
    void draw3dText(const btVector3&, const char*) override {}
    void setDebugMode(int mode) override { m_mode = mode; }
    int  getDebugMode() const override  { return m_mode; }

private:
    int m_mode = btIDebugDraw::DBG_DrawWireframe |
                 btIDebugDraw::DBG_DrawContactPoints;
    GLuint m_vao = 0, m_vbo = 0, m_prog = 0;
    std::vector<float> m_verts;
};

#endif
