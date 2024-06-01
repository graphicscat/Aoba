#include<engine/descriptor.h>
#include<util/vkinit.h>

void Descriptor::bind(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding)
{
    VkDescriptorSetLayoutBinding bind = 
    vkinit::descriptorsetLayoutBinding(type,stageFlags,binding);
    m_bindings.push_back(bind);
}

void Descriptor::update()
{
    VkDescriptorSetLayoutCreateInfo info = vkinit::descriptorSetLayoutLayoutCreateInfo(m_bindings);
    
    vkCreateDescriptorSetLayout(VulkanContext::get()->getDevice(),&info,nullptr,&m_desLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = VulkanContext::get()->getDdescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_desLayout;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

    VK_CHECK(vkAllocateDescriptorSets(VulkanContext::get()->getDevice(),&allocInfo,&m_set))
}

VkDescriptorSet Descriptor::makeSet()
{
    VkDescriptorSet set;
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = VulkanContext::get()->getDdescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_desLayout;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

    VK_CHECK(vkAllocateDescriptorSets(VulkanContext::get()->getDevice(),&allocInfo,&set))
    return set;
}

VkDescriptorSetLayout Descriptor::getLayout()
{
    return m_desLayout;
}

VkDescriptorSet& Descriptor::getSet()
{
    return m_set;
}

void Descriptor::release()
{
    vkDestroyDescriptorSetLayout(VulkanContext::get()->getDevice(),m_desLayout,nullptr);
}

Descriptor::~Descriptor()
{
    release();
}