#pragma once
#include<pch.h>

#include<engine/renderpass.h>
#include<engine/pipeline.h>
#include<engine/descriptor.h>
#include<engine/buffer.h>
#include<util/vkinit.h>
#include<engine/framebuffer.h>
#include<engine/camera.h>

struct ShadowPass
{
    std::unique_ptr<GraphicsPipeline> pipeline;
    std::shared_ptr<RenderPass> renderpass;
    std::shared_ptr<Framebuffer> framebuffer;
    std::shared_ptr<Descriptor> descriptor;

    std::shared_ptr<Buffer> camUBO;
    std::shared_ptr<vkinit::CameraUBO>camParam;
    glm::mat4 m_lightSpace;
};

namespace Factory
{
    void CreateShadowPassResource(glm::vec3 lightPos,ShadowPass& shadowpass,glm::mat4& lightSpace);
};