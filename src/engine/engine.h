#pragma once
#include<memory>
#include <editor/config.h>
#include<engine/scene.h>
#include<window/mwindow.h>

class Engine
{
    public:

    Engine() = delete;
    Engine(const Config& config);
    ~Engine() = default;

    void init();
    void tick();
    void release();

    public:
    std::shared_ptr<MWindow> m_win = nullptr;
    std::unique_ptr<Scene> m_scene = nullptr;

    private:
    Config m_config;

    float deltaTime = 0.0f;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameStamp;

    uint32_t m_frameCount;
};