#pragma once

#include<pch.h>
#include<engine/renderpass.h>
#include<engine/framebuffer.h>
#include<engine/descriptor.h>
#include<engine/pipeline.h>
#include<engine/buffer.h>
#include<engine/objmesh.h>

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

    void init() override;

    void buildCmd() override;

    void tick() override;

    void update(float delta) override;


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