#pragma once

#include<pch.h>

// class Pipeline
// {
//     public:
//     virtual void init() = 0;
//     virtual void create() = 0;

// };

class GraphicsPipeline
{
    public:

    GraphicsPipeline(const std::string& vs,const std::string& fs,VkDescriptorSetLayout desLayout, const VkRenderPass& renderpass,
    uint32_t pcSize = 0);
    
    ~GraphicsPipeline();

    void init();

    void create();

    VkPipeline getPipeline();

    VkPipelineLayout getLayout();

    void setVertexInputState(VkPipelineVertexInputStateCreateInfo);

    void setInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo);

    void setRasterizationState(VkPipelineRasterizationStateCreateInfo);

    void setTessellationShaders(const char* tesc, const char* tese);

    void setTessellationState(VkPipelineTessellationStateCreateInfo);

    void setColorBlendState(uint32_t);

    void setColorBlendState(std::vector<VkPipelineColorBlendAttachmentState>& blendAttachments);

    void setDepthStencilState(bool depthTest, bool depthWrite);

    void setViewport(VkExtent2D);

    void setDynamicState(std::vector<VkDynamicState>& dynamicStateEnables);

    void setCullMode(VkCullModeFlags cullmode);

    private:

    VkPipeline m_pipeline;
    
    VkPipelineLayout m_pipelineLayout;

    VkShaderModule m_vertexShader;

	VkShaderModule m_fragShader;

    VkShaderModule m_tescShader;

    VkShaderModule m_teseShader;

    std::vector<VkShaderModule> m_shaderModulePool;

    std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;

    VkPipelineVertexInputStateCreateInfo m_vertexInputInfo;

	VkPipelineInputAssemblyStateCreateInfo m_inputAssembly;

	VkViewport m_viewport;

	VkRect2D m_scissor;

	VkPipelineRasterizationStateCreateInfo m_rasterizer;

	std::vector<VkPipelineColorBlendAttachmentState> m_colorBlendAttachment;

	VkPipelineColorBlendStateCreateInfo m_colorBlendState;

	VkPipelineMultisampleStateCreateInfo m_multisampling;

	//VkPipelineLayout m_pipelineLayout;

	VkPipelineDepthStencilStateCreateInfo m_depthStencil;

    VkPipelineViewportStateCreateInfo m_viewStateInfo;

    VkPipelineColorBlendStateCreateInfo m_colorBlending;

    VkPipelineTessellationStateCreateInfo m_tesselationState;

    VkGraphicsPipelineCreateInfo m_graphicsPipelineInfo;

    VkRenderPass m_renderpass;

    uint32_t m_attachmentCount = 1;

    VkPipelineLayoutCreateInfo m_layoutInfo;

    VkPushConstantRange range{};

    VkPipelineDynamicStateCreateInfo m_dynamicState;
};

class ComputePipeline
{
    public:

    ComputePipeline() = default;

    ComputePipeline(const char* compShader, VkDescriptorSetLayout desLayout,uint32_t pcsize = 0);

    ~ComputePipeline();

    VkPipeline getPipeline();

    VkPipelineLayout getLayout();

    private:

    VkPipeline m_pipeline;
    VkPipelineLayout m_pipelineLayout;

};