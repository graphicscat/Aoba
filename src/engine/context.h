#pragma once

#include <vulkan/vulkan.h>
#include <engine/swapchain.h>

class Engine;
class Camera;
class VulkanContext final
{
    public:

    static VulkanContext* get();

    void init(Engine* engine);

    void release();

    VkDevice getDevice();

    VkPhysicalDevice getGPU();

    VkSurfaceKHR getSurface();

    Swapchain& getSwapchain();

    VkPipelineCache getCache();

    VkPipelineCache getCPcache();

    VkDescriptorPool getDdescriptorPool();

    VkQueue getQueue();

    uint32_t getWidth();

    uint32_t getHeight();

    VkCommandPool getCommandPool();

    GLFWwindow* getWindow();

    std::shared_ptr<Camera> getCamera();

    Swapchain m_swapchain;

    VkInstance getInstance();

    private:

    void initInstance();

    void findQueueIndex();

    void initPipelineCache();

    void initCommandPool();

    void initDescriptorPool();
    

    Engine* m_engine;

    VkDevice m_device;

    VkInstance m_instance;

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkPhysicalDevice m_physicalDevice;

    VkQueue m_graphicsQueue;

	uint32_t m_graphicsQueueFamily;

    VkPipelineCache m_cache;

    VkPipelineCache m_CPcache;
    
    VkCommandPool m_commandPool;

    VkDescriptorPool m_descriptorPool;

};