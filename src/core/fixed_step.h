#pragma once

struct FixedStep {
    float accumulator = 0.f;
    float dt          = 1.f / 60.f;
    float maxFrame    = 0.25f;

    int tick(float frameTime) {
        if (frameTime > maxFrame) frameTime = maxFrame;
        if (frameTime < 0.f)      frameTime = 0.f;
        accumulator += frameTime;
        int n = 0;
        while (accumulator >= dt) { accumulator -= dt; ++n; }
        return n;
    }

    float alpha() const { return accumulator / dt; }
};
