#include <editor/editor.h>
#include <editor/config.h>
#include<pch.h>

Editor* Editor::get()
{
    static Editor editor;
	return &editor;
}

void Editor::run()
{
    init();
    tick();
    release();
}

void Editor::init()
{
    Logger::init();
    
    Config config;
    config.appName = "Aoba";
    config.windowInfo.initWidth = 1920;
    config.windowInfo.initHeight = 1080;

    m_engine = std::make_unique<Engine>(config);

    m_engine->init();
}

void Editor::tick()
{
    m_engine->tick();
}

void Editor::release()
{
    m_engine->release();
    m_engine.reset();
}