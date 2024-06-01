#include <engine/framebuffer.h>

void Framebuffer::init(std::shared_ptr<RenderPass> renderpass, uint32_t w, uint32_t h)
{
    m_framebuffers.resize(VulkanContext::get()->getSwapchain().m_swapchainImageViews.size());
    LOG_INFO("Framebuffer size {0}",m_framebuffers.size());

    uint32_t width = w;
    uint32_t height = h;

    if(w == 0)
    {
        width = VulkanContext::get()->getSwapchain().m_swapchainExtent.width;
        height = VulkanContext::get()->getSwapchain().m_swapchainExtent.height;
    }
    VkImageCreateInfo depthCreateInfo = vkinit::imageCreateInfo(VK_FORMAT_D16_UNORM,VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,{width,height,1});

    //depth.init(depthCreateInfo,VK_IMAGE_ASPECT_DEPTH_BIT);

    m_depth = std::make_unique<Image>(depthCreateInfo,VK_IMAGE_ASPECT_DEPTH_BIT);
    m_depth->setDescriptorImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

    VkFramebufferCreateInfo fb_info = vkinit::framebufferCreateInfo(renderpass->m_renderPass,{width,height});

    

    if(renderpass->isPresent())
    {
        for(int i = 0;i<m_framebuffers.size();i++)
        {
            std::vector<VkImageView> attachments = {VulkanContext::get()->getSwapchain().m_swapchainImageViews[i]};
            attachments.push_back(m_depth->m_view);
            fb_info.pAttachments = attachments.data();
            fb_info.attachmentCount = uint32_t(attachments.size());
            VK_CHECK(vkCreateFramebuffer(VulkanContext::get()->getDevice(),&fb_info,nullptr,&m_framebuffers[i]))
        }
          LOG_TRACE("Init Framebuffers Success");
    }
    else
    {
        std::vector<VkFormat>formats = renderpass->getFormats();
        m_colorAttachments.resize(formats.size());
        for(int i = 0 ;i<formats.size();i++)
        {   
            VkImageCreateInfo ci = vkinit::imageCreateInfo(
                formats[i],
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                {width,height,1}
            );

            m_colorAttachments[i] = std::make_shared<Image>(ci,VK_IMAGE_ASPECT_COLOR_BIT);

        }

        std::vector<VkImageView> attachments;
        for(int i = 0 ;i<m_colorAttachments.size();i++)
        {
            attachments.push_back(m_colorAttachments[i]->m_view);
        }
        if(renderpass->hasDepth())
        attachments.push_back(m_depth->m_view);
        
        for(int i = 0;i<m_framebuffers.size();i++)
        {
            fb_info.pAttachments = attachments.data();
            fb_info.attachmentCount = uint32_t(attachments.size());
            VK_CHECK(vkCreateFramebuffer(VulkanContext::get()->getDevice(),&fb_info,nullptr,&m_framebuffers[i]))
        }
          LOG_TRACE("Init Framebuffers Success");
    }
    
}

void Framebuffer::release()
{
    //depth.release();
    m_depth.reset();
    for(int i = 0 ;i<m_framebuffers.size();i++)
    {
        vkDestroyFramebuffer(VulkanContext::get()->getDevice(),m_framebuffers[i],nullptr);
    }
   
}

Framebuffer::Framebuffer(std::shared_ptr<RenderPass> renderpass,uint32_t w, uint32_t h)
{
    init(renderpass,w,h);
}

Framebuffer::~Framebuffer()
{
    release();
}