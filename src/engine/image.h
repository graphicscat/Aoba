#pragma once

#include<pch.h>

// struct ImageBuilder
// {
//     VkImageType m_type;

//     uint32_t m_layerCount = 1;

//     uint32_t m_width = 0;
//     uint32_t m_height = 0;
//     uint32_t m_depth = 1;

//     uint32_t m_mipmapLevel = 1;

//     VkMemoryPropertyFlags m_memProperty;

//     VkFormat m_format;

//     VkImageUsageFlags m_usage;

//     ImageBuilder& setExtent(uint32_t w, uint32_t h);
//     ImageBuilder& setImageType(VkImageType type);
//     ImageBuilder& setLayerCount(uint32_t layerCount);
//     ImageBuilder& setMipMapLevel(uint32_t mipmaplevel);
//     ImageBuilder& setMemProperty(VkMemoryPropertyFlags memprop);
//     ImageBuilder& setFormat(VkFormat format);
//     ImageBuilder& setUsage(VkImageUsageFlags usages);

//     bool isComplete() const;

// };


class Image
{
    public:

    //Image(uint32_t w,uint32_t h,VkFormat format, uint32_t lc = 1, uint32_t ml = 1);
    Image() = default;

    Image(const VkImageCreateInfo& createinfo,VkImageAspectFlags aspect,
    VkMemoryPropertyFlags property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    void loadFromFile(const char* path, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB, bool hdr = false);
    
    ~Image();

    void init(const VkImageCreateInfo& createinfo,VkImageAspectFlags aspect,VkMemoryPropertyFlags property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    void release();

    void transitionImaglayout(VkImageLayout oldLayout,VkImageLayout newLayout);

    void writeToSet(VkDescriptorSet set, VkDescriptorType type, uint32_t binding);

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


    VkImageLayout m_initLayout;

    VkImageLayout m_finalLayout;

    VkImageCreateInfo m_createInfo{};

    bool m_sampled = false;

    void copyBufferToImage(VkBuffer);

};