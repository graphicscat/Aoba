#pragma once

#include <vulkan/vulkan.h>
#include <macro.h>
#include<pch.h>

class VulkanContext;
class Swapchain
{
public:

    // Swapchain images.
    std::vector<VkImage> m_swapchainImages = {};

    // Swapchain image views.
    std::vector<VkImageView> m_swapchainImageViews = {};

    // Swapchain format.
    VkFormat m_swapchainImageFormat = {};

    // Extent of swapchain.
    VkExtent2D m_swapchainExtent = {};

    // Swapchain handle.
    VkSwapchainKHR m_swapchain = {};

    // Current swapchain using surface format.
    VkSurfaceFormatKHR m_surfaceFormat = {};

    // Current swapchain present mode.
    VkPresentModeKHR m_presentMode = {};

    struct SwapchainDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		VkSurfaceFormatKHR format;
		VkPresentModeKHR present;
		VkSurfaceTransformFlagBitsKHR transform;
		VkExtent2D imageExtent;
		uint32_t imageCount;
	}m_details;


    void init(const int w,const int h) ;
    void release();
    private:
    
    void querySwapchainSupport(const int w,const int h);

};