#include<engine/pipeline.h>
#include<util/vkinit.h>

VkPipeline GraphicsPipeline::getPipeline()
{
    return m_pipeline;
}

VkPipelineLayout GraphicsPipeline::getLayout()
{
    return m_pipelineLayout;
}
GraphicsPipeline::GraphicsPipeline(const std::string& vs,const std::string& fs,VkDescriptorSetLayout desLayout,const VkRenderPass& renderpass,uint32_t pcSize)
{
    m_renderpass = renderpass;

    if (!vkinit::loadShaderModule(vs.c_str(), &m_vertexShader))
	{
		LOG_ERROR("Error when building the vertex shader");
	}

    
    if (!vkinit::loadShaderModule(fs.c_str(), &m_fragShader))
	{
		LOG_ERROR("Error when building the fragment shader");
	}

    if(!fs.empty())
    m_shaderModulePool.push_back(m_fragShader);
    m_shaderModulePool.push_back(m_vertexShader);

    m_shaderStages.push_back(vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT,m_vertexShader));
    if(!fs.empty())
    m_shaderStages.push_back(vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT,m_fragShader));
    
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &desLayout;
    if(pcSize > 0 )
    {
        range.offset = 0;
        range.size = pcSize;
        range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &range;
        
    }

    VK_CHECK(vkCreatePipelineLayout(VulkanContext::get()->getDevice(),&layoutInfo,nullptr,&m_pipelineLayout));
    LOG_TRACE("Init GraphicsPipelineLayout success");

    m_vertexInputInfo = vkinit::vertexInputStateCreateInfo();
    m_inputAssembly = vkinit::inputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    m_rasterizer = vkinit::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

    m_multisampling = vkinit::multisamplingStateCreateInfo();
    m_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    m_colorBlendAttachment.push_back(vkinit::colorBlendAttachmentState());

    uint32_t w = VulkanContext::get()->getSwapchain().m_swapchainExtent.width;
    uint32_t h = VulkanContext::get()->getSwapchain().m_swapchainExtent.height;

    m_viewport.width = w;
    m_viewport.height = h;
    m_scissor.extent = VkExtent2D{w,h};

    m_viewport.x = 0.0;
    m_viewport.y = 0.0;
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;
   
    m_scissor.offset = {0, 0};

    m_depthStencil = vkinit::depthStencilCreateInfo(true,true,VK_COMPARE_OP_LESS_OR_EQUAL);

    VkPipelineViewportStateCreateInfo viewStateInfo{};
    viewStateInfo.pViewports = &m_viewport;
    viewStateInfo.pScissors = &m_scissor;
    viewStateInfo.scissorCount = 1;
    viewStateInfo.viewportCount = 1;
    viewStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_viewStateInfo = viewStateInfo;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;

	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = m_attachmentCount;
	colorBlending.pAttachments = m_colorBlendAttachment.data();

    m_colorBlendState = colorBlending;

    m_graphicsPipelineInfo = vkinit::graphicsPipelineCreateInfo();

}

void GraphicsPipeline::init()
{

}

void GraphicsPipeline::create()
{
    // VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
    // m_graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    m_graphicsPipelineInfo.pVertexInputState = &m_vertexInputInfo;
    m_graphicsPipelineInfo.pInputAssemblyState = &m_inputAssembly;
    m_graphicsPipelineInfo.pRasterizationState = &m_rasterizer;
    m_graphicsPipelineInfo.pColorBlendState = &m_colorBlendState;
    m_graphicsPipelineInfo.stageCount = uint32_t(m_shaderStages.size());
    m_graphicsPipelineInfo.pStages = m_shaderStages.data();
    m_graphicsPipelineInfo.pViewportState = &m_viewStateInfo;
    m_graphicsPipelineInfo.pDepthStencilState = &m_depthStencil;
    m_graphicsPipelineInfo.layout = m_pipelineLayout;
    m_graphicsPipelineInfo.subpass = 0;
    m_graphicsPipelineInfo.renderPass = m_renderpass;
    m_graphicsPipelineInfo.pMultisampleState = &m_multisampling;
    //graphicsPipelineInfo.pNext = &renderingInfo;

    VK_CHECK(vkCreateGraphicsPipelines(VulkanContext::get()->getDevice(),VulkanContext::get()->getCache(),1,&m_graphicsPipelineInfo,nullptr,&m_pipeline))
    LOG_TRACE("Init GraphicsPipeline Success");

    for(auto& module:m_shaderModulePool)
    {
        vkDestroyShaderModule(VulkanContext::get()->getDevice(),module,nullptr);
    }
    // vkDestroyShaderModule(VulkanContext::get()->getDevice(),m_fragShader,nullptr);
    // vkDestroyShaderModule(VulkanContext::get()->getDevice(),m_vertexShader,nullptr);
}

void GraphicsPipeline::setVertexInputState(VkPipelineVertexInputStateCreateInfo ci)
{
    m_vertexInputInfo = ci;
}

void GraphicsPipeline::setInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo ci)
{
    m_inputAssembly = ci;
}

void GraphicsPipeline::setRasterizationState(VkPipelineRasterizationStateCreateInfo ci)
{
    m_rasterizer = ci;
}

void GraphicsPipeline::setDepthStencilState(bool depthTest, bool depthWrite)
{
    m_depthStencil.depthTestEnable = depthTest?VK_TRUE:VK_FALSE;
    m_depthStencil.depthWriteEnable = depthWrite?VK_TRUE:VK_FALSE;
}

void GraphicsPipeline::setTessellationShaders(const char* tesc, const char* tese)
{
    if (!vkinit::loadShaderModule(tesc, &m_tescShader))
	{
		LOG_ERROR("Error when building the tesc shader");
	}

    
    if (!vkinit::loadShaderModule(tese, &m_teseShader))
	{
		LOG_ERROR("Error when building the tese shader");
	}

    m_shaderModulePool.push_back(m_tescShader);
    m_shaderModulePool.push_back(m_teseShader);

    m_shaderStages.push_back(vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,m_tescShader));
    m_shaderStages.push_back(vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,m_teseShader));
}

void GraphicsPipeline::setTessellationState(VkPipelineTessellationStateCreateInfo ci)
{
    m_tesselationState = ci;
    m_graphicsPipelineInfo.pTessellationState = &m_tesselationState;
}

void GraphicsPipeline::setColorBlendState(uint32_t num)
{
    m_colorBlendAttachment.clear();

    for(int i = 0 ;i<num;i++)
    {
        m_colorBlendAttachment.push_back(vkinit::colorBlendAttachmentState());
    }

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;

	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = num;
	colorBlending.pAttachments = m_colorBlendAttachment.data();
    
    m_colorBlendState = colorBlending;
}

void GraphicsPipeline::setColorBlendState(std::vector<VkPipelineColorBlendAttachmentState>& blendAttachments)
{
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;

	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = blendAttachments.size();
	colorBlending.pAttachments = blendAttachments.data();
    
    m_colorBlendState = colorBlending;
}

void GraphicsPipeline::setViewport(VkExtent2D ext)
{
    m_viewport.width = ext.width;
    m_viewport.height = ext.height;

    m_scissor.extent = ext;
}

void GraphicsPipeline::setDynamicState(std::vector<VkDynamicState>& dynamicStateEnables)
{
    m_dynamicState = vkinit::pipelineDynamicStateCreateInfo(dynamicStateEnables);
    m_graphicsPipelineInfo.pDynamicState = &m_dynamicState;
}

void GraphicsPipeline::setCullMode(VkCullModeFlags cullmode)
{
    VkPipelineRasterizationStateCreateInfo rsci = vkinit::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
    rsci.cullMode = cullmode;
    this->setRasterizationState(rsci);
}

GraphicsPipeline::~GraphicsPipeline()
{
    vkDestroyPipelineLayout(VulkanContext::get()->getDevice(),m_pipelineLayout,nullptr);
    vkDestroyPipeline(VulkanContext::get()->getDevice(),m_pipeline,nullptr);
}

VkPipeline ComputePipeline::getPipeline()
{
    return m_pipeline;
}

VkPipelineLayout ComputePipeline::getLayout()
{
    return m_pipelineLayout;
}

ComputePipeline::ComputePipeline(const char* compShader, VkDescriptorSetLayout desLayout,uint32_t pcsize)
{
    VkShaderModule m_compShader{};
    if (!vkinit::loadShaderModule(compShader, &m_compShader))
    {
        LOG_ERROR("Error when building the compute shader");
    }

    VkPipelineShaderStageCreateInfo shaderStageInfo = vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT,m_compShader);

    VkPushConstantRange range;
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &desLayout;

    if(pcsize)
    {
        range.offset = 0;
        range.size = pcsize;
        range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &range;
    }
    
    VK_CHECK(vkCreatePipelineLayout(VulkanContext::get()->getDevice(),&layoutInfo,nullptr,&m_pipelineLayout));
    LOG_TRACE("Init ComputePipelineLayout success");

    VkComputePipelineCreateInfo CPipelineCreateInfo{};
    CPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    CPipelineCreateInfo.layout = m_pipelineLayout;
    CPipelineCreateInfo.stage = shaderStageInfo;

    VK_CHECK(vkCreateComputePipelines(VulkanContext::get()->getDevice(),VulkanContext::get()->getCPcache(),1,&CPipelineCreateInfo,nullptr,&m_pipeline));

    //m_shaderStages.push_back(vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT,m_compShader));
    vkDestroyShaderModule(VulkanContext::get()->getDevice(),m_compShader,nullptr);
}

ComputePipeline::~ComputePipeline()
{
    vkDestroyPipelineLayout(VulkanContext::get()->getDevice(),m_pipelineLayout,nullptr);
    vkDestroyPipeline(VulkanContext::get()->getDevice(),m_pipeline,nullptr);
}