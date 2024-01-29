#pragma once
#include<pch.h>

class Descriptor
{
    public:
    Descriptor() = default;
    ~Descriptor();

    void bind(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding);

    void update();

    VkDescriptorSetLayout getLayout();

    VkDescriptorSet& getSet();

    private:

    void release();

    std::vector<VkDescriptorSetLayoutBinding> m_bindings;

    VkDescriptorSetLayout m_desLayout;

    VkDescriptorSet m_set;
};