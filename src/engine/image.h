#pragma once

#include<pch.h>
#include <ktx.h>
#include <ktxvulkan.h>

class Image
{
    public:

    //Image(uint32_t w,uint32_t h,VkFormat format, uint32_t lc = 1, uint32_t ml = 1);
    Image() = default;

    Image(const char*, VkFormat ,bool mipmap = false);

    Image(const VkImageCreateInfo& createinfo,VkImageAspectFlags aspect,
    VkMemoryPropertyFlags property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    void loadFromFile(const char* path, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB, bool hdr = false , bool mipmap = false);
    
    ~Image();


    void loadKTXFromFile(std::string filename,VkFormat format, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    void init(const VkImageCreateInfo& createinfo,VkImageAspectFlags aspect,VkMemoryPropertyFlags property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    void release();

    void transitionImaglayout(VkImageLayout oldLayout,VkImageLayout newLayout);

    void transitionImaglayout(VkImageLayout oldLayout,VkImageLayout newLayout,VkCommandBuffer cmd);

    void writeToSet(VkDescriptorSet set, VkDescriptorType type, uint32_t binding);

    void  setDescriptorImageLayout(VkImageLayout);

    void generateMipmaps();

    void generateMipmaps(VkCommandBuffer cmd);

    uint32_t getWidth();

    uint32_t getHeight();

    VkImage m_image;

    VkDeviceMemory m_mem;

	VkImageView m_view;

    VkSampler m_sampler;

    VkDescriptorImageInfo m_descriptor{};

   
    private:

    void createImage(VkImageCreateInfo createinfo);

    void createImageView();

    void createSampler();

    ktxResult loadKTXFile(std::string filename, ktxTexture **target);

    VkImageLayout m_currentLayout;

    VkImageCreateInfo m_createInfo{};

    bool m_sampled = false;

    void copyBufferToImage(VkBuffer);

    void copyBufferToImage(VkBuffer buffer, std::vector<VkBufferImageCopy>& regions);

};