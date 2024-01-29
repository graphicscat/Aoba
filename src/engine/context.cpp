#include <engine/context.h>
#include <engine/engine.h>

#include<macro.h>

#include<pch.h>



const std::vector<const char*> validationLayers = {
  "VK_LAYER_KHRONOS_validation" 
};

VulkanContext* VulkanContext::get()
{
    static VulkanContext context;
    return &context;
}

void VulkanContext::init(Engine* engine)
{
    m_engine = engine;

    initInstance();

    m_swapchain.init(m_engine->m_win->getWidth(),m_engine->m_win->getHeight());

    initPipelineCache();

    initCommandPool();

    initDescriptorPool();
}

void VulkanContext::initInstance()
{
    uint32_t extCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&extCount);

    VkInstanceCreateInfo instanceInfo{};
    VkApplicationInfo appInfo{};
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pEngineName = "Vulkan Engine";
    appInfo.pApplicationName = "Vulkan App";


    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = uint32_t(validationLayers.size());
    instanceInfo.ppEnabledLayerNames = validationLayers.data();

    instanceInfo.enabledExtensionCount = extCount;
    instanceInfo.ppEnabledExtensionNames = glfwExtensions;

    VK_CHECK(vkCreateInstance(&instanceInfo,nullptr,&m_instance))
    LOG_TRACE("Init Instance Success");

    VK_CHECK(glfwCreateWindowSurface(m_instance,m_engine->m_win->window,nullptr,&m_surface));

    LOG_TRACE("Init Surface Success");
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance,&physicalDeviceCount,nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(m_instance,&physicalDeviceCount,physicalDevices.data());

    m_physicalDevice = physicalDevices[0];

    LOG_TRACE("Init PhysicalDevice Success");

    findQueueIndex();

    VkDeviceQueueCreateInfo queueInfo{};
    float queuePriority = 1.0f;
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &queuePriority;
    queueInfo.queueFamilyIndex = m_graphicsQueueFamily;

    //features
    VkPhysicalDeviceFeatures features{};
    features.fillModeNonSolid = VK_TRUE;
    features.tessellationShader = VK_TRUE;

    std::vector<const char*> arr = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.enabledExtensionCount = 1;
    deviceInfo.ppEnabledExtensionNames = arr.data();
    deviceInfo.pEnabledFeatures = &features;
    //deviceInfo.pNext = &shaderClockFeatures;

    VK_CHECK(vkCreateDevice(m_physicalDevice,&deviceInfo,nullptr,&m_device))
    vkGetDeviceQueue(m_device,m_graphicsQueueFamily,0,&m_graphicsQueue);
    LOG_TRACE("Init Device Success");

}

void VulkanContext::findQueueIndex()
{
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice,&count,nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice,&count,queueFamilyProperties.data());

    for(int i = 0 ;i<queueFamilyProperties.size();i++)
    {
        const auto& prop = queueFamilyProperties[i];
        if(prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            m_graphicsQueueFamily = i;
        }
    }
}

void VulkanContext::release()
{
    vkDeviceWaitIdle(m_device);

    vkDestroyDescriptorPool(m_device,m_descriptorPool,nullptr);

    vkDestroyPipelineCache(m_device,m_cache,nullptr);

    vkDestroyPipelineCache(m_device,m_CPcache,nullptr);

    vkDestroyCommandPool(m_device,m_commandPool,nullptr);

    m_swapchain.release();

    vkDestroySurfaceKHR(m_instance,m_surface,nullptr);

    vkDestroyDevice(m_device,nullptr);

    vkDestroyInstance(m_instance,nullptr);
}

VkDevice VulkanContext::getDevice()
{
    return m_device;
}

VkSurfaceKHR  VulkanContext::getSurface()
{
    return m_surface;
}

VkPhysicalDevice VulkanContext::getGPU()
{
    return m_physicalDevice;
}

Swapchain& VulkanContext::getSwapchain()
{
    return m_swapchain;
}

VkPipelineCache VulkanContext::getCache()
{
    return m_cache;
}

VkDescriptorPool VulkanContext::getDdescriptorPool()
{
    return m_descriptorPool;
}

uint32_t VulkanContext::getWidth()
{
    return m_swapchain.m_swapchainExtent.width;
}

uint32_t VulkanContext::getHeight()
{
    return m_swapchain.m_swapchainExtent.height;
}

VkQueue VulkanContext::getQueue()
{
    return m_graphicsQueue;
}

VkCommandPool VulkanContext::getCommandPool()
{
    return m_commandPool;
}

VkPipelineCache VulkanContext::getCPcache()
{
    return m_CPcache;
}

GLFWwindow* VulkanContext::getWindow()
{
    return m_engine->m_win.get()->window;
}

std::shared_ptr<Camera> VulkanContext::getCamera()
{
    return m_engine->m_win->m_cam;
}

void VulkanContext::initPipelineCache()
{
    VkPipelineCacheCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    createInfo.pInitialData = VK_NULL_HANDLE;
    createInfo.initialDataSize = 0;
    VK_CHECK(vkCreatePipelineCache(m_device,&createInfo,nullptr,&m_cache));
    LOG_TRACE("Init Pipeline Cache Success");

    VK_CHECK(vkCreatePipelineCache(m_device,&createInfo,nullptr,&m_CPcache));
    LOG_TRACE("Init Pipeline Cache Success");
}

void VulkanContext::initCommandPool()
{
    VkCommandPoolCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ci.queueFamilyIndex = m_graphicsQueueFamily;
    VK_CHECK(vkCreateCommandPool(m_device,&ci,nullptr,&m_commandPool));

    LOG_TRACE("Init CommandPool Success");
}

void VulkanContext::initDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> sizes = 
	{
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,10},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,10},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,10}
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.maxSets = 100;
	poolInfo.poolSizeCount = (uint32_t)sizes.size();
	poolInfo.pPoolSizes = sizes.data();

	VK_CHECK(vkCreateDescriptorPool(m_device,&poolInfo,nullptr,&m_descriptorPool))
    LOG_TRACE("Init DescriptorPool Success");
}