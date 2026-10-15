#pragma once
#include <glm/glm.hpp>
#include <string>

// Thin wrapper around miniaudio's engine. One global engine,
// many short sounds. No streaming; sounds are loaded fully into memory.
class Audio {
public:
    bool init();
    void shutdown();

    // Listener state. Call once per frame from the camera.
    void setListener(const glm::vec3& pos,
                     const glm::vec3& forward,
                     const glm::vec3& up);

    // Fire-and-forget 3D one-shot. Loads lazily on first call for a given
    // path; caches the decoded buffer for subsequent calls.
    void play3d(const std::string& path,
                const glm::vec3& pos,
                float volume = 1.f,
                float pitch  = 1.f);

private:
    struct Impl;
    Impl* m_impl = nullptr;
};
