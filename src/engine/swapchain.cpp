#include<engine/swapchain.h>
#include<engine/context.h>
#include<util/vkinit.h>


void Swapchain::init(const int w,const int h)
{
    VulkanContext* m_context = VulkanContext::get();

    //VkSwapchainKHR oldswapchain = m_swapchain;

    querySwapchainSupport(w,h);
    VkSwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageColorSpace = m_details.format.colorSpace;
    swapchainInfo.imageExtent = m_details.imageExtent;
    swapchainInfo.imageFormat = m_details.format.format;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.minImageCount = m_details.imageCount;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.presentMode = m_details.present;
    swapchainInfo.preTransform = m_details.transform;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
    swapchainInfo.surface = m_context->getSurface();
    VK_CHECK(vkCreateSwapchainKHR(m_context->getDevice(),&swapchainInfo,nullptr,&m_swapchain))

    m_swapchainExtent = m_details.imageExtent;
    LOG_INFO("Swapchain Image Width:{0}, Height:{1}",swapchainInfo.imageExtent.width,swapchainInfo.imageExtent.height);
    LOG_TRACE("Init Swapchain Success");

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(m_context->getDevice(),m_swapchain,&imageCount,nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_context->getDevice(),m_swapchain,&imageCount,m_swapchainImages.data());

    m_swapchainImageViews.resize(m_swapchainImages.size());

    for(int i = 0;i<m_swapchainImageViews.size();i++)
    {
        auto createInfo = vkinit::imageview_begin_info(m_swapchainImages[i],m_details.format.format,VK_IMAGE_ASPECT_COLOR_BIT);
        VK_CHECK(vkCreateImageView(m_context->getDevice(),&createInfo,nullptr,&m_swapchainImageViews[i]))
    }

    m_swapchainImageFormat = m_details.format.format;
}
void Swapchain::querySwapchainSupport(const int w,const int h)
{
    VulkanContext* m_context = VulkanContext::get();
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context->getGPU(),m_context->getSurface(),&m_details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->getGPU(),m_context->getSurface(),&formatCount,nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->getGPU(),m_context->getSurface(),&formatCount,surfaceFormats.data());

    m_details.format = surfaceFormats[0];

    for(const auto& format:surfaceFormats)
    {
        if(format.format == VK_FORMAT_B8G8R8A8_UNORM&&
        format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR){
            LOG_WARN("SwapChian Format Set");
            m_details.format = format;
            break;
        }
    }

    uint32_t imageCount = m_details.capabilities.minImageCount + 1;
    if (m_details.capabilities.maxImageCount > 0 && imageCount > m_details.capabilities.maxImageCount)
    {
        imageCount = m_details.capabilities.maxImageCount;
    }

    m_details.imageCount = imageCount;

    LOG_INFO("Swapchain Images Counts {0}",imageCount);
    m_details.imageExtent.width = w;
    m_details.imageExtent.height = h;
    m_details.transform = m_details.capabilities.currentTransform;

    m_details.present = VK_PRESENT_MODE_IMMEDIATE_KHR;
}

void Swapchain::release()
{
    for(int i = 0;i<m_swapchainImageViews.size();i++)
    {
        vkDestroyImageView(VulkanContext::get()->getDevice(),m_swapchainImageViews[i],nullptr);
    }
    vkDestroySwapchainKHR(VulkanContext::get()->getDevice(),m_swapchain,nullptr);
    //m_context = nullptr;
   
}