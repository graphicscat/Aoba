#pragma once

#include<pch.h>
#include<engine/renderpass.h>
#include<engine/framebuffer.h>
#include<engine/descriptor.h>
#include<engine/pipeline.h>
#include<engine/buffer.h>
#include<engine/objmesh.h>
#include<engine/ui.h>
#include<engine/factory.h>
#include<engine/assobj.h>

class Scene
{
    public:
    Scene();
    
    virtual ~Scene();
    virtual void init() = 0;

    //void release();

    virtual void buildCmd() = 0;

    virtual void tick() = 0;

    virtual void update(float delta) = 0;

    virtual void windowResize() = 0;

    protected:

    uint32_t m_frameNumber = 0;

    std::vector<VkCommandBuffer> m_mainCmdBuffers;

    // std::vector<VkSemaphore> m_presentSemaphores;

    // std::vector<VkSemaphore> m_renderSemaphores;

    //std::vector<VkFence> m_fences;

    VkSemaphore m_presentSemaphore;

    VkSemaphore m_renderSemaphore;

    std::shared_ptr<RenderPass> m_renderPass;
    //Framebuffer m_framebuffer;

    std::unique_ptr<Framebuffer> m_framebuffer;

    private:

    void initSyn();

    void initMainCmdBuffer();

};

class ShaderToy : public Scene
{
    public:
    ShaderToy() = default;
    ~ShaderToy();
    struct ShaderToyInputData
    {
        alignas(16) glm::vec3 iResolution;
        alignas(4) float iTime;
        alignas(4) float iTimeDelta;
        alignas(4) float iFrameRate;
        alignas(4) float iFrame;
        alignas(16) glm::vec4 iMouse;
        alignas(16) glm::vec4 iDate;
    };

    struct PushConstantData
    {
        float width;
        float height;
    }m_pcd;

    std::shared_ptr<Descriptor> m_descriptor;

    std::shared_ptr<ShaderToyInputData>m_inputUniform;
    std::shared_ptr<Buffer>m_inputUniformBuffer;

    std::unique_ptr<GraphicsPipeline>m_graphicsPipeline = nullptr;

    std::vector<VkDynamicState>m_dynamicStates;

    void init() override;

    void buildCmd() override;

    void tick() override;

    void update(float delta) override;

    void windowResize() override;


};

class Atmosphere : public Scene
{
    public:

    Atmosphere() = default;

    ~Atmosphere();

    void init() override;

    void buildCmd() override;

    void tick() override;

    void update(float delta) override;

    void windowResize() override;

    public:

    std::shared_ptr<Descriptor> m_presentDescriptor;

    std::shared_ptr<Descriptor> m_transmitDescriptor;

    //sky-view descriptor
    std::shared_ptr<Descriptor> m_skyViewDescriptor;

    std::unique_ptr<GraphicsPipeline> m_graphicsPipeline;

    std::unique_ptr<ComputePipeline> m_computePipeline;

    std::unique_ptr<ComputePipeline> m_skyViewPipeline;

    std::shared_ptr<Image> m_storageImage;

    std::shared_ptr<Image> m_skyViewLUT;

    struct TexConfig
    {
        uint32_t transmittanceTexWidth;
        uint32_t transmittanceTexHeight;
    }m_texConfig;

    struct AtmosphereParam
    {
        alignas(16) glm::vec4 transSkyLUTRes;//xy: transmittance zw: skyview
        alignas(16) glm::vec3 rayleighScatteringBase;
        alignas(4)  float rayleighScaleHeight;
        alignas(4)  float groundRadiusMM;
        alignas(4)  float atmosphereRadiusMM;  
        alignas(4)  float iTime;
    }m_atmosphereParam;

    std::shared_ptr<Buffer>m_atmosphereParamBuffer;

    VkSemaphore m_skyViewComputeSemaphore;

    VkSemaphore m_skyViewGraphicsSemaphore;

    VkCommandBuffer m_skyViewCmd;

    VkCommandBuffer m_transmittanceCmd;

    private:

    void precomputeTransmit();

    void initPipelines();

    void initDescriptors();

    void initResources();

    void buildSkyViewCmd();

    void release();

};

class Camera;
class OBJScene : public Scene
{
    public:

    OBJScene() = default;

    ~OBJScene();

    void init() override;

    void buildCmd() override;

    void tick() override;

    void update(float delta) override;

    void windowResize() override;

    protected:

    std::unique_ptr<GraphicsPipeline> m_GPipeline;

    std::shared_ptr<Descriptor> m_presentDescriptor;

    //cam setting
    std::shared_ptr<Buffer> m_camUBO;
    std::shared_ptr<vkinit::CameraUBO>m_camParam;
    std::shared_ptr<Camera> m_cam;

    //precompute grass pos
    std::shared_ptr<Buffer> m_grassposBuffer;

    std::unique_ptr<ComputePipeline> m_grassposCPipeline;
    std::shared_ptr<Descriptor>m_grassposDescriptor;

    std::shared_ptr<Mesh> m_grass;

    //wind tex
    float m_time;
    std::shared_ptr<Buffer>m_windTimeBuffer;
    std::shared_ptr<Image>m_windTex;
    std::unique_ptr<ComputePipeline> m_windCPipeline;
    std::shared_ptr<Descriptor> m_windDescriptor;

    VkCommandBuffer m_windCmd;
    VkSemaphore m_windGraphicsSemaphore;
    VkSemaphore m_windComputeSemaphore;

    

    private:

    void initPipelines();

    void initDescriptors();

    void initResources();

    void precomputeGrassPos();

    void buildWindCmd();

};

class Terrain : public Scene
{
    public:

    Terrain() = default;
    ~Terrain();

    void init() override;

    void buildCmd() override;

    void tick() override;

    void update(float delta) override;

    void windowResize() override; 

    protected:

    //cam    
    std::shared_ptr<Buffer> m_camUBO;
    std::shared_ptr<vkinit::CameraUBO>m_camParam;
    std::shared_ptr<Camera> m_cam;

    std::shared_ptr<Mesh> m_terrain;

    std::shared_ptr<Image> m_heightMap;
    std::shared_ptr<Image> m_terrainAlbedo;
    std::shared_ptr<Image> m_terrainNormalMap;

    std::shared_ptr<Descriptor>m_presentDescriptor;

    std::unique_ptr<GraphicsPipeline> m_presentGPipeline;

    //grass instancing
    std::unique_ptr<GraphicsPipeline> m_grassGPipeline;

    std::shared_ptr<Descriptor> m_grassDescriptor;
    std::shared_ptr<Buffer> m_grassposBuffer;

    std::unique_ptr<ComputePipeline> m_grassposCPipeline;
    std::shared_ptr<Descriptor>m_grassposDescriptor;

    //wind
    float m_time;
    std::shared_ptr<Buffer>m_windTimeBuffer;
    std::shared_ptr<Image>m_windTex;
    std::unique_ptr<ComputePipeline> m_windCPipeline;
    std::shared_ptr<Descriptor> m_windDescriptor;

    //grass others
    VkCommandBuffer m_windCmd;
    VkSemaphore m_windGraphicsSemaphore;
    VkSemaphore m_windComputeSemaphore;

    std::shared_ptr<Mesh> m_grass;

    private:

    void initHeightMap();

    void initPatchData();

    void initDescriptors();

    void initPipelines();

    void initResources();

    //init Grass
    void initGrass();

    void precomputeGrassPos();

    void initGrassPipelines();

    void initGrassDescriptors();

    void initGrassResources();

    void buildWindCmd();
};

class Deferred : public Scene
{
    public:
    Deferred() = default;
    ~Deferred();
    
    void init() override;

    void buildCmd() override;

    void tick() override;

    void update(float delta) override;

    void windowResize() override;

    protected:

    //deferred render pass res
    void initDeferredRenderPass();
    std::shared_ptr<RenderPass> m_deferredRenderPass;
    std::shared_ptr<Framebuffer> m_deferredFramebuffer;

    std::shared_ptr<RenderPass> m_filterRenderPass;
    std::shared_ptr<Framebuffer> m_filterFramebuffer;

    //spec
    std::shared_ptr<Image> m_specularFilterMap;
    std::unique_ptr<GraphicsPipeline> m_specularFilterGPipeline;
    VkDescriptorSet m_specularFilterSet;

    VkDescriptorSet m_diffuseFilterSet;

    std::unique_ptr<GraphicsPipeline> m_diffuseFilterGPipeline;
    std::vector<VkDynamicState> m_dynamicStates;
    std::shared_ptr<Descriptor> m_diffuseFilterDescriptor;
    std::shared_ptr<Image> m_diffuseCubeMap;
    glm::mat4 m_hdrToCubeMapMat;
    uint32_t m_diffuseMipNum;
    uint32_t m_specularMipNum;

    //brdf
    std::shared_ptr<Image> m_brdfLUT;
    std::unique_ptr<ComputePipeline> m_brdfLUTCPipeline;
    std::shared_ptr<Descriptor> m_brdfLUTDescriptor;
    VkCommandBuffer m_brdfLUTCmd;


    //deferred pipeline res
    void initDeferredDescriptor();
    void initDeferredPipelines();

    std::shared_ptr<Descriptor> m_deferredDescriptor;
    std::unique_ptr<GraphicsPipeline> m_deferredGPipeline;

    //present
    void initDescriptors();
    void initPipelines();
    std::shared_ptr<Descriptor> m_presentDescriptor;
    std::unique_ptr<GraphicsPipeline> m_presentGPipeline;

    //skybox
    VkCommandBuffer m_diffuseFilterCmd;
    void drawSkybox(VkCommandBuffer);
    std::shared_ptr<Descriptor>m_skyboxDescriptor;
    std::unique_ptr<GraphicsPipeline> m_skyboxGPipeline;
    std::shared_ptr<Mesh> m_skybox;
    std::shared_ptr<Image>m_skybox_tex;

    std::shared_ptr<Image>m_diffuseFilterMap;

    void generatePrecomputeMap();

    //buffers
    void initResources();
    std::shared_ptr<Buffer> m_camUBO;
    std::shared_ptr<vkinit::CameraUBO>m_camParam;
    std::shared_ptr<Camera> m_cam;

    std::shared_ptr<Mesh> m_mesh;

    struct PushConsts
    {
        glm::mat4 model;
        float roughness;
    };

    PushConsts m_specularPC;

    struct StaticObject
    {
        std::shared_ptr<Mesh>mesh;
        VkDescriptorSet material;
        PushConsts pc;
    };

    //toyman
    std::shared_ptr<Mesh> m_toyman_head;
    std::shared_ptr<Mesh> m_toyman_body;
    std::shared_ptr<Mesh> m_toyman_base;
    std::shared_ptr<Mesh> m_plane;
   

    std::shared_ptr<Image>m_head_albedo;
    std::shared_ptr<Image>m_body_albedo;
    std::shared_ptr<Image>m_base_albedo;

    std::shared_ptr<Image>m_head_roughness;
    std::shared_ptr<Image>m_body_roughness;
    std::shared_ptr<Image>m_base_roughness;

    std::shared_ptr<Image>m_empty;

    std::shared_ptr<Image>m_head_metallic;
    std::shared_ptr<Image>m_body_metallic;
    std::shared_ptr<Image>m_base_metallic;

    std::shared_ptr<Image>m_head_normal;
    std::shared_ptr<Image>m_body_normal;
    std::shared_ptr<Image>m_base_normal;

    VkDescriptorSet m_head_material;
    VkDescriptorSet m_body_material;
    VkDescriptorSet m_plane_material;
    VkDescriptorSet m_base_material;

    std::vector<std::shared_ptr<StaticObject>>m_scene;


};

class ImGuiTest : public Scene
{
    public:
    ImGuiTest() = default;
    ~ImGuiTest();

    void init() override;

    void buildCmd() override;

    void tick() override;

    void update(float delta) override;

    void windowResize() override;

    std::shared_ptr<Image>testTex;

    VkDescriptorSet m_des;

    VkFence m_fence;

    uint32_t m_frameNumber;

    //shadertoy
    struct ShaderToyInputData
    {
        alignas(16) glm::vec3 iResolution;
        alignas(4) float iTime;
        alignas(4) float iTimeDelta;
        alignas(4) float iFrameRate;
        alignas(4) float iFrame;
        alignas(16) glm::vec4 iMouse;
        alignas(16) glm::vec4 iDate;
    };

    struct PushConstantData
    {
        float width;
        float height;
    }m_pcd;

    std::shared_ptr<Descriptor> m_descriptor;

    std::shared_ptr<ShaderToyInputData>m_inputUniform;
    std::shared_ptr<Buffer>m_inputUniformBuffer;

    std::unique_ptr<GraphicsPipeline>m_graphicsPipeline = nullptr;

    std::vector<VkDynamicState>m_dynamicStates;

    std::shared_ptr<RenderPass>m_offscreenRenderPass;

    std::shared_ptr<Framebuffer>m_offscreenFramebuffer;

    uint32_t offscreenWidth;
    uint32_t offscreenHeight;
};

class FC : public Scene
{
    public:
    FC() = default;
    ~FC();

    void init() override;

    void buildCmd() override;

    void tick() override;

    void update(float delta) override;

    void windowResize() override;

    void loadAssets();

    void shadowPass(VkCommandBuffer cmd);

    vkinit::Plane initPlane(const glm::vec3& vmax, const glm::vec3& vmin);

    float m_time = 0.0f;

    struct RenderObject
    {
        std::shared_ptr<Mesh> mesh;
        VkDescriptorSet descriptorSet;
    };

    glm::mat4                           m_pcMatrix;
    std::shared_ptr<Mesh>               m_teapot;
    std::shared_ptr<Mesh>               m_planeLight;
    std::shared_ptr<Mesh>               m_plane;

    std::shared_ptr<Image>              m_empty;

    std::shared_ptr<Image>              m_plane_albedo;
    std::shared_ptr<Image>              m_plane_normal;
    std::shared_ptr<Image>              m_plane_ao;
    std::shared_ptr<Image>              m_plane_metal;
    std::shared_ptr<Image>              m_plane_roughness;

    std::vector<RenderObject>           m_scenes;

    std::unique_ptr<GraphicsPipeline>   m_presentGPipeline;
    std::unique_ptr<GraphicsPipeline>   m_boundingBoxGPipeline;

    std::shared_ptr<Buffer>             m_camUBO;
    std::shared_ptr<vkinit::CameraUBO>  m_camParam;
    std::shared_ptr<Camera>             m_cam;

    std::shared_ptr<Descriptor>         m_presentDescriptor;
    std::shared_ptr<Descriptor>         m_boxDescriptor;

    ShadowPass                          m_shadowPass;
    vkinit::LightUBO                    m_lightubo;
    std::shared_ptr<Buffer>             m_lightBuffer;

    //bloom
    std::unique_ptr<GraphicsPipeline>   m_offscreenGPipeline;
    std::shared_ptr<Descriptor>         m_offscreenDescriptor;
    std::shared_ptr<Framebuffer>        m_lightFramebuffer;
    std::shared_ptr<RenderPass>         m_lightRenderPass;

    //colorpick
    std::unique_ptr<GraphicsPipeline>   m_downSamplingPipeline;
    std::shared_ptr<Descriptor>         m_downSamplingDescriptor;
    std::shared_ptr<Framebuffer>        m_downSamplingFramebuffer;
    std::shared_ptr<RenderPass>         m_downSamplingRenderPass;
    std::vector<VkDynamicState>         m_dynamicStates;
    void initBlurPass();

    void downSamplingPass(VkCommandBuffer cmd);

    //downsampling 
    std::shared_ptr<Image>              m_downSamplingImage;

    //upsampling
    std::shared_ptr<GraphicsPipeline>   m_upSamplingPipeline;
    std::shared_ptr<Descriptor>         m_upSamplingDescriptor;
    void upSamplingPass(VkCommandBuffer cmd);
};

class ShadowMapping : public Scene
{
    public:
    ShadowMapping() = default;
    ~ShadowMapping();

    void init() override;

    void buildCmd() override;

    void tick() override;

    void update(float delta) override;

    void windowResize() override;

    void initPipeline();

    void initRenderPass();

    void initResource();

    void shadowPass(VkCommandBuffer cmd);

    struct LightParam
    {
        glm::mat4 lightSpace;
        alignas(16) glm::vec3 lightPos;
    }m_lightParam;

    public:
    std::shared_ptr<AssScene>           m_scene;
    std::shared_ptr<RenderPass>         m_staticObjectRenderPass;
    std::shared_ptr<Framebuffer>        m_staticObjectFramebuffer;
    std::shared_ptr<Descriptor>         m_staticObjectDescriptor;
    std::unique_ptr<GraphicsPipeline>   m_staticObjectGraphicsPipeline;

    std::shared_ptr<Descriptor>         m_offscreenDescriptor;
    std::unique_ptr<GraphicsPipeline>   m_offscreenGraphicsPipeline;

    //resource
    std::shared_ptr<Buffer>             m_camUBO;
    std::shared_ptr<vkinit::CameraUBO>  m_camParam;
    std::shared_ptr<Camera>             m_cam;
    glm::mat4                           m_model;
    ShadowPass                          m_shadowPass;
    glm::vec3                           m_lightPos;
    glm::mat4                           m_lightSpace;
    std::shared_ptr<Buffer>             m_lightBuffer;

};

class Particle : public Scene
{
    public:
    Particle() = default;
    Particle(uint32_t count, uint32_t size);
    ~Particle();

    void init() override;

    void buildCmd() override;

    void tick() override;

    void update(float delta) override;

    void windowResize() override;

    void initResource();

    void initParticlePipeline();

    float rnd(float range);

    private:
    uint32_t m_particleCount = 0;
    uint32_t m_particleSize  = 0;

    struct ParticleParam{
        glm::vec4 pos;
        glm::vec4 color;
        glm::vec4 vel;
        float alpha;
        float size;
        float rotation;
        float type;
        // Attributes not used in shader
        alignas(16)float rotationSpeed;
    };

    struct ParamUBO
    {
        glm::vec2 viewPortDim;
    }m_ubo;

    struct EnvParam
    {
        glm::vec4 timeRadius = glm::vec4(0.0f,-8.0f,0.0f,0.0f);
        glm::vec4 emitterPos = glm::vec4(0.0f, -8.0f + 2.0f, 0.0f,0.0f);
        glm::vec4 minVel = glm::vec4(-3.0f, 0.5f, -3.0f,0.0f);
        glm::vec4 maxVel = glm::vec4(3.0f, 7.0f, 3.0f,0.0f);
    }m_envParam;

    std::default_random_engine rndEngine;

    std::unique_ptr<GraphicsPipeline>       m_offscreenPipeline;
    std::shared_ptr<Descriptor>             m_offscreenDescriptor;

    std::unique_ptr<GraphicsPipeline>       m_staticObjectPipeline;
    std::shared_ptr<Descriptor>             m_staticObjectDescriptor;

    std::shared_ptr<Framebuffer>            m_staticObjectFramebuffer;
    std::shared_ptr<RenderPass>             m_staticObjectRenderPass;
    
    //resourse
    std::shared_ptr<Image>                  m_firePlaceTexture;
    std::shared_ptr<Image>                  m_firePlaceNormalTexture;
    std::shared_ptr<AssScene>               m_scene;

    std::shared_ptr<Buffer>                 m_camUBO;
    std::shared_ptr<vkinit::CameraUBO>      m_camParam;
    std::shared_ptr<Camera>                 m_cam;

    std::shared_ptr<Buffer>                 m_uboBuffer;

    //particle
    std::vector<ParticleParam>              m_particles;
    std::unique_ptr<GraphicsPipeline>       m_particleDrawPipeline;
    std::shared_ptr<Descriptor>             m_particleDrawDescriptor;
    std::shared_ptr<Image>                  m_fireTexture;
    std::shared_ptr<Image>                  m_smokeTexture;
    std::shared_ptr<Buffer>                 m_particleVertexBuffer;

    std::unique_ptr<ComputePipeline>        m_particleComputePipeline;
    std::shared_ptr<Descriptor>             m_particleComputeDescriptor;

    std::shared_ptr<Buffer>                 m_envParamBuffer;
    VkCommandBuffer                         m_updateParticleCmd;
    VkSemaphore                             m_updateParticleComputeSemaphore;
    VkSemaphore                             m_updateParticleGraphicsSemaphore;

    void initParticles();

    void updateParticleCmd();
};