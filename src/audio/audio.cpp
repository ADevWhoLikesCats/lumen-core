#include "audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <cstdio>
#include <unordered_map>

struct Audio::Impl {
    ma_engine engine;
    bool     engineReady = false;

    // Decoded sound cache. Key = file path.
    std::unordered_map<std::string, ma_sound*> cache;
};

bool Audio::init() {
    m_impl = new Impl();

    ma_engine_config cfg = ma_engine_config_init();
    cfg.channels   = 2;
    cfg.sampleRate = 48000;

    if (ma_engine_init(&cfg, &m_impl->engine) != MA_SUCCESS) {
        std::fprintf(stderr, "[audio] engine init failed\n");
        delete m_impl;
        m_impl = nullptr;
        return false;
    }
    m_impl->engineReady = true;

    std::fprintf(stderr, "[audio] miniaudio engine ready (%d Hz)\n",
                 m_impl->engine.sampleRate);
    return true;
}

void Audio::shutdown() {
    if (!m_impl) return;
    for (auto& [path, snd] : m_impl->cache) {
        ma_sound_uninit(snd);
        delete snd;
    }
    m_impl->cache.clear();
    if (m_impl->engineReady) ma_engine_uninit(&m_impl->engine);
    delete m_impl;
    m_impl = nullptr;
}

void Audio::setListener(const glm::vec3& pos,
                        const glm::vec3& forward,
                        const glm::vec3& up) {
    if (!m_impl || !m_impl->engineReady) return;
    ma_engine_listener_set_position(&m_impl->engine, 0, pos.x, pos.y, pos.z);
    ma_engine_listener_set_direction(&m_impl->engine, 0,
                                     forward.x, forward.y, forward.z);
    ma_engine_listener_set_world_up(&m_impl->engine, 0, up.x, up.y, up.z);
}

void Audio::play3d(const std::string& path,
                   const glm::vec3& pos,
                   float volume,
                   float pitch) {
    if (!m_impl || !m_impl->engineReady) return;

    ma_sound* snd = nullptr;
    auto it = m_impl->cache.find(path);
    if (it != m_impl->cache.end()) {
        // miniaudio sounds can't be played twice simultaneously with the
        // same instance. We make a fresh one each call by copying the
        // cached buffer. For now, simplest: just create a new sound.
        snd = new ma_sound();
        if (ma_sound_init_from_file(&m_impl->engine, path.c_str(),
                                    MA_SOUND_FLAG_ASYNC, nullptr, nullptr,
                                    snd) != MA_SUCCESS) {
            delete snd;
            return;
        }
    } else {
        snd = new ma_sound();
        if (ma_sound_init_from_file(&m_impl->engine, path.c_str(),
                                    MA_SOUND_FLAG_ASYNC, nullptr, nullptr,
                                    snd) != MA_SUCCESS) {
            std::fprintf(stderr, "[audio] failed to load %s\n", path.c_str());
            delete snd;
            return;
        }
        m_impl->cache[path] = snd;
    }

    ma_sound_set_position(snd, pos.x, pos.y, pos.z);
    ma_sound_set_volume(snd, volume);
    ma_sound_set_pitch(snd, pitch);
    ma_sound_set_spatialization_enabled(snd, MA_TRUE);
    ma_sound_start(snd);
}
