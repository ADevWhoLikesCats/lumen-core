#include "physics_debug.h"

#ifdef LUMEN_HAVE_BULLET

#include <glm/gtc/type_ptr.hpp>
#include <cstdio>

static const char* kVS = R"GLSL(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;
uniform mat4 uViewProj;
out vec3 vColor;
void main() {
    vColor = aColor;
    gl_Position = uViewProj * vec4(aPos, 1.0);
}
)GLSL";

static const char* kFS = R"GLSL(
#version 330 core
in vec3 vColor;
out vec4 fragColor;
void main() { fragColor = vec4(vColor, 1.0); }
)GLSL";

static GLuint compile(GLenum t, const char* s) {
    GLuint sh = glCreateShader(t);
    glShaderSource(sh, 1, &s, nullptr);
    glCompileShader(sh);
    GLint ok; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(sh, 1024, nullptr, log);
        std::fprintf(stderr, "physdbg shader: %s\n", log);
    }
    return sh;
}

void PhysicsDebugDraw::init() {
    GLuint vs = compile(GL_VERTEX_SHADER, kVS);
    GLuint fs = compile(GL_FRAGMENT_SHADER, kFS);
    m_prog = glCreateProgram();
    glAttachShader(m_prog, vs);
    glAttachShader(m_prog, fs);
    glLinkProgram(m_prog);
    glDeleteShader(vs);
    glDeleteShader(fs);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6,
                          (void*)(sizeof(float)*3));
    glBindVertexArray(0);
}

void PhysicsDebugDraw::shutdown() {
    if (m_vbo)  glDeleteBuffers(1, &m_vbo);
    if (m_vao)  glDeleteVertexArrays(1, &m_vao);
    if (m_prog) glDeleteProgram(m_prog);
    m_vbo = m_vao = m_prog = 0;
}

void PhysicsDebugDraw::begin() { m_verts.clear(); }

void PhysicsDebugDraw::drawLine(const btVector3& from, const btVector3& to,
                                const btVector3& color) {
    m_verts.insert(m_verts.end(), {
        from.x(), from.y(), from.z(),
        color.x(), color.y(), color.z(),
        to.x(),   to.y(),   to.z(),
        color.x(), color.y(), color.z()
    });
}

void PhysicsDebugDraw::flush(const glm::mat4& viewProj) {
    if (m_verts.empty() || !m_prog) return;
    glUseProgram(m_prog);
    glUniformMatrix4fv(glGetUniformLocation(m_prog, "uViewProj"),
                       1, GL_FALSE, glm::value_ptr(viewProj));
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_verts.size() * sizeof(float),
                 m_verts.data(), GL_STREAM_DRAW);
    glDrawArrays(GL_LINES, 0, (GLsizei)(m_verts.size() / 6));
    glBindVertexArray(0);
}

#endif
