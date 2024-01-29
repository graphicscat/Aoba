#pragma once
#include<pch.h>

class Buffer
{
    public:
    
    VkBuffer m_buffer;
    VkDeviceMemory m_mem;
    VkDescriptorBufferInfo m_descriptor{};
    void* m_mapped;
    uint32_t m_size = 0;
    VkBufferUsageFlags m_flag;

    Buffer() = delete;
    ~Buffer();

    Buffer(uint32_t s,VkBufferUsageFlags flag = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VkMemoryPropertyFlags memproperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    //void create(VkBufferUsageFlags flag = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    //void createDeviceBuffer(VkBufferUsageFlags flag, void* pData);
    void writeToSet(VkDescriptorSet set, uint32_t binding,VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    //void update(const void * data);

    void map(void* data);
    void unmap();

    void release();

    private:

    void createBuffer(
        VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& memory,
		void* data = nullptr
    );
};