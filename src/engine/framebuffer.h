#pragma once
#include <macro.h>
#include <pch.h>
#include <engine/renderpass.h>
#include<engine/image.h>

class Framebuffer
{
    public:
    Framebuffer() = default;
    Framebuffer(std::shared_ptr<RenderPass> renderpass,uint32_t w = 0, uint32_t h = 0);
    ~Framebuffer();


    public:
    std::vector<VkFramebuffer> m_framebuffers;

    //Image depth;

    std::unique_ptr<Image> m_depth;
    std::vector<std::shared_ptr<Image>> m_colorAttachments;

    private:

    void init(std::shared_ptr<RenderPass> renderpass,uint32_t w, uint32_t h);
    void release();
};