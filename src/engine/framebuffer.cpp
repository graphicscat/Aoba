#include <engine/framebuffer.h>

void Framebuffer::init(std::shared_ptr<RenderPass> renderpass)
{
    m_framebuffers.resize(VulkanContext::get()->getSwapchain().m_swapchainImageViews.size());
    LOG_INFO("Framebuffer size {0}",m_framebuffers.size());

    uint32_t width = VulkanContext::get()->getSwapchain().m_swapchainExtent.width;
    uint32_t height = VulkanContext::get()->getSwapchain().m_swapchainExtent.height;
    VkImageCreateInfo depthCreateInfo = vkinit::imageCreateInfo(VK_FORMAT_D32_SFLOAT,VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,{width,height,1});

    //depth.init(depthCreateInfo,VK_IMAGE_ASPECT_DEPTH_BIT);

    m_depth = std::make_unique<Image>(depthCreateInfo,VK_IMAGE_ASPECT_DEPTH_BIT);

    VkFramebufferCreateInfo fb_info = vkinit::framebufferCreateInfo(renderpass->m_renderPass,{width,height});

    if(true)
    {
        for(int i = 0;i<m_framebuffers.size();i++)
        {
            
            std::vector<VkImageView> attachments = {VulkanContext::get()->getSwapchain().m_swapchainImageViews[i],m_depth->m_view};
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

Framebuffer::Framebuffer(std::shared_ptr<RenderPass> renderpass)
{
    init(renderpass);
}

Framebuffer::~Framebuffer()
{
    release();
}