#pragma once

#include <util/vkinit.h>

struct rpBuilder
{

};

class RenderPass
{
    public:
    RenderPass() = default;

    RenderPass(const std::vector<VkFormat>& formats, bool hasdepth = true);

    ~RenderPass();

    void init();

    bool hasDepth();

    bool isPresent();

    void getPresentRenderPass();

    std::vector<VkFormat> getFormats();

    private:
    void release();

    bool m_hasDepth;

    bool m_isPresent;

    std::vector<VkAttachmentDescription> m_colAttachDescriptions;

    std::vector<VkFormat> m_formats;

    public:
    VkRenderPass m_renderPass;
};