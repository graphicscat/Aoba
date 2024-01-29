#pragma once
#include <engine/engine.h>


class Editor
{
    public:

    void run();
    static Editor* get();

    private:

    Editor() = default;
    void init();
    void tick();
    void release();

    std::unique_ptr<Engine> m_engine = nullptr;
};