#pragma once

#include <util/vkinit.h>

struct rpBuilder
{

};

class RenderPass
{
    public:
    RenderPass() = default;

    ~RenderPass();

    void init(const rpBuilder& builder);

   

    void getPresentRenderPass();

    private:
    void release();

    public:
    VkRenderPass m_renderPass;
};