#pragma once
#include <macro.h>
#include <pch.h>
#include <engine/renderpass.h>
#include<engine/image.h>

class Framebuffer
{
    public:
    Framebuffer() = default;
    Framebuffer(std::shared_ptr<RenderPass> renderpass);
    ~Framebuffer();


    public:
    std::vector<VkFramebuffer> m_framebuffers;

    //Image depth;

    std::unique_ptr<Image> m_depth;

    private:

    void init(std::shared_ptr<RenderPass> renderpass);
    void release();
};