#include <engine/engine.h>
#include <engine/context.h>

Engine::Engine(const Config& config):m_config(config){};

void Engine::init()
{
    m_win = std::make_unique<MWindow>(
        m_config.appName,
        m_config.windowInfo.initWidth,
        m_config.windowInfo.initHeight
    );

    m_win->init();

    VulkanContext::get()->init(this);

    m_scene = std::make_unique<ShaderToy>();

    m_scene->init();

    m_frameCount = 0;
}

void Engine::tick()
{
    lastFrameStamp = std::chrono::high_resolution_clock::now();
    while(!glfwWindowShouldClose(m_win->window))
    {
        glfwPollEvents();

        auto tStart = std::chrono::high_resolution_clock::now();

        m_scene->tick();
        m_frameCount++;

        auto tEnd = std::chrono::high_resolution_clock::now();

        auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

        deltaTime = (float)tDiff / 1000.0f;

        m_scene->update(deltaTime);

        m_win->tick(deltaTime);

        float fpsTimer = (float)std::chrono::duration<double, std::milli>(tEnd - lastFrameStamp).count();

        if(fpsTimer >1000.f)
        {
            auto FPS = static_cast<uint32_t>((float)m_frameCount*1000.0f/fpsTimer);
            std::string title = std::to_string(FPS) + "FPS";
            m_win->setWinTitle(title);

            m_frameCount = 0;
            lastFrameStamp = tEnd;
        }
        
    }
}

void Engine::release()
{
    m_scene.reset();
    VulkanContext::get()->release();
    m_win.reset();
}

