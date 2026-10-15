#pragma once
#include "glb_loader.h"
#include <vector>

// Holds the six parts of the car glb: body, glass, four wheels.
// Does not own the meshes; they live in the vector returned by loadGLB.
struct CarRenderer {
    const GLBMesh* body  = nullptr;
    const GLBMesh* glass = nullptr;
    const GLBMesh* wheel[4] = { nullptr, nullptr, nullptr, nullptr };

    // Car glb is in centimetres; we work in metres.
    float assetScale = 0.01f;

    // Axis correction. The glb was authored in 3DS Max's Y-up
    // left-handed convention, which doesn't match the engine's
    // Y-up right-handed system. This matrix is applied before
    // the asset scale at draw time. Cycles with the K key.
    glm::mat4 modelFix = glm::mat4(1.f);
    int       modelFixIndex = 0;

    // Candidate fixes. Cycle through them and pick the one that
    // looks correct. Index 0 = identity (no fix).
    void cycleFix();

    // parts must have at least 6 entries in this order:
    //   [0] body  [1] glass  [2] wheel_FL  [3] wheel_FR
    //   [4] wheel_RL  [5] wheel_RR
    // Returns false if the vector is too small.
    bool init(const std::vector<GLBMesh>& parts) {
        if (parts.size() < 6) return false;
        body  = &parts[0];
        glass = &parts[1];
        for (int i = 0; i < 4; ++i) wheel[i] = &parts[2 + i];
        return true;
    }
};
