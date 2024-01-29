#include<engine/buffer.h>
#include<util/vkinit.h>

Buffer::Buffer(uint32_t s, VkBufferUsageFlags flag, VkMemoryPropertyFlags memproperty):m_size(s),m_flag(flag)
{
    createBuffer(m_size,flag,
	memproperty,
	m_buffer,
	m_mem);

    m_descriptor.buffer = m_buffer;
    m_descriptor.offset = 0;
    m_descriptor.range = m_size;

    if(memproperty & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    VK_CHECK(vkMapMemory(VulkanContext::get()->getDevice(),m_mem,0,m_size,0,&m_mapped))
}

void Buffer::map(void* data)
{
    memcpy(m_mapped,data,m_size);
}

void Buffer::unmap()
{
    vkUnmapMemory(VulkanContext::get()->getDevice(),m_mem);
}

void Buffer::createBuffer(
        VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& memory,
		void* data 
    )
    {
        VkBufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.size = size;
        createInfo.usage = usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK(vkCreateBuffer(VulkanContext::get()->getDevice(),&createInfo,nullptr,&buffer))

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        VkMemoryRequirements memRequirements{};
        vkGetBufferMemoryRequirements(VulkanContext::get()->getDevice(),buffer,&memRequirements);
        auto typeFilter = memRequirements.memoryTypeBits;
        allocInfo.memoryTypeIndex = vkinit::findMemoryType(typeFilter,properties);
        allocInfo.allocationSize = memRequirements.size;

        VK_CHECK(vkAllocateMemory(VulkanContext::get()->getDevice(),&allocInfo,nullptr,&memory))
        VK_CHECK(vkBindBufferMemory(VulkanContext::get()->getDevice(),buffer,memory,0))
        // void* mapped = nullptr;

        // if(data != nullptr)
        // {
        //     vkMapMemory(VulkanContext::get()->getDevice(),memory,0,size,0,&mapped);
        //     memcpy(VulkanContext::get()->getDevice(),data,size);
        //     vkUnmapMemory(VulkanContext::get()->getDevice(),memory);
        // }
    }

void Buffer::release()
{

}

void Buffer::writeToSet(VkDescriptorSet set, uint32_t binding,VkDescriptorType type)
{
    VkWriteDescriptorSet writer{};
    writer.descriptorCount = 1;
    writer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writer.dstBinding = binding;
    writer.dstSet = set;
    writer.pBufferInfo = &m_descriptor;
    writer.descriptorType = type;
    vkUpdateDescriptorSets(VulkanContext::get()->getDevice(),1,&writer,0,nullptr);
}

Buffer::~Buffer()
{
    vkFreeMemory(VulkanContext::get()->getDevice(),m_mem,nullptr);
    vkDestroyBuffer(VulkanContext::get()->getDevice(),m_buffer,nullptr);
}