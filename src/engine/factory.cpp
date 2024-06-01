#include<engine/factory.h>
#include<macro.h>
#include<engine/assobj.h>

void Factory::CreateShadowPassResource(glm::vec3 lightPos,ShadowPass& shadowpass,glm::mat4& lightSpace)
{
    //ShadowPass shadowpass;
    std::vector<VkFormat> formats;

    shadowpass.descriptor = std::make_shared<Descriptor>();
    shadowpass.descriptor->bind(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
    shadowpass.descriptor->update();

    shadowpass.renderpass = std::make_shared<RenderPass>(formats);
    shadowpass.framebuffer = std::make_shared<Framebuffer>(shadowpass.renderpass,SHADOW_DIM,SHADOW_DIM);
    shadowpass.pipeline = std::make_unique<GraphicsPipeline>(
        "../../shaders/shadow/shadow.vert.spv",
        "",
        shadowpass.descriptor->getLayout(),
        shadowpass.renderpass->m_renderPass,
        sizeof(glm::mat4)
    );

    AssVertexInputDescription des = AssVertex::get_vertex_description();

    VkPipelineVertexInputStateCreateInfo ci = vkinit::vertexInputStateCreateInfo();

    ci.vertexAttributeDescriptionCount = des.attributes.size();
	ci.pVertexAttributeDescriptions = des.attributes.data();
	ci.vertexBindingDescriptionCount = des.bindings.size();
	ci.pVertexBindingDescriptions = des.bindings.data();

    shadowpass.pipeline->setVertexInputState(ci);

    VkPipelineRasterizationStateCreateInfo rsci = vkinit::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
    rsci.cullMode = VK_CULL_MODE_NONE;
    shadowpass.pipeline->setRasterizationState(rsci);
    shadowpass.pipeline->setColorBlendState(0);

    shadowpass.pipeline->setViewport({SHADOW_DIM,SHADOW_DIM});

    std::vector<VkDynamicState> m_dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    shadowpass.pipeline->setDynamicState(m_dynamicStates);

    shadowpass.pipeline->create();

    shadowpass.camUBO = std::make_shared<Buffer>(sizeof(glm::mat4));
    shadowpass.camUBO->writeToSet(shadowpass.descriptor->getSet(),0);

    shadowpass.camParam = std::make_shared<vkinit::CameraUBO>();

    glm::mat4 projection = glm::perspective(glm::radians(45.f), 1.0f, 1.0f, 96.0f);
    projection[1][1] *= -1;
    glm::mat4 orth = glm::ortho(-50.f,50.f,-50.f,50.f,0.1f,50.f);

    glm::vec3 lightpos = glm::vec3(lightPos);
    shadowpass.camParam->view = glm::lookAt(lightpos,glm::vec3(0.0,0.0,0.0),glm::vec3(0,1,0));
    shadowpass.camParam->project = projection;
    shadowpass.camParam->camPos = glm::vec4(lightpos,0.0f);

    lightSpace = projection*shadowpass.camParam->view ;
    shadowpass.m_lightSpace = projection*shadowpass.camParam->view;

    shadowpass.camUBO->map(&shadowpass.m_lightSpace);
    //shadowpass.camUBO->unmap();
}