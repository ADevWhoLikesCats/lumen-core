#include "car_renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>

void CarRenderer::cycleFix() {
    // Try these in order. Stop when the car looks right.
    static const char* names[] = {
        "identity",
        "rotY(180)",
        "rotY(90)",
        "rotY(-90)",
        "rotX(-90)",
        "rotX(90)",
        "rotY(180) * rotX(-90)",
        "scale(-1,1,1)  // mirror X",
        "scale(1,1,-1)  // mirror Z",
        "scale(-1,1,-1) // mirror X and Z (== rotY 180)",
    };

    modelFixIndex = (modelFixIndex + 1) % 10;
    const float D = 3.14159265f / 180.f;

    switch (modelFixIndex) {
        case 0: modelFix = glm::mat4(1.f); break;
        case 1: modelFix = glm::rotate(glm::mat4(1.f), 180.f*D, {0,1,0}); break;
        case 2: modelFix = glm::rotate(glm::mat4(1.f),  90.f*D, {0,1,0}); break;
        case 3: modelFix = glm::rotate(glm::mat4(1.f), -90.f*D, {0,1,0}); break;
        case 4: modelFix = glm::rotate(glm::mat4(1.f), -90.f*D, {1,0,0}); break;
        case 5: modelFix = glm::rotate(glm::mat4(1.f),  90.f*D, {1,0,0}); break;
        case 6: modelFix = glm::rotate(glm::mat4(1.f), 180.f*D, {0,1,0})
                          * glm::rotate(glm::mat4(1.f), -90.f*D, {1,0,0}); break;
        case 7: modelFix = glm::scale(glm::mat4(1.f), {-1.f, 1.f, 1.f}); break;
        case 8: modelFix = glm::scale(glm::mat4(1.f), {1.f, 1.f, -1.f}); break;
        case 9: modelFix = glm::scale(glm::mat4(1.f), {-1.f, 1.f, -1.f}); break;
    }
    std::fprintf(stderr, "[car] modelFix #%d: %s\n",
                 modelFixIndex, names[modelFixIndex]);
}
