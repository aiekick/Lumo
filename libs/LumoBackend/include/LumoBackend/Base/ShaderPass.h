/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once
#pragma warning(disable : 4251)
#pragma warning(disable : 4275)

#include <set>
#include <map>
#include <string>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

#include <Gaia/gaia.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Buffer/ComputeBuffer.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>
#include <Gaia/Interfaces/OutputSizeInterface.h>
#include <Gaia/Resources/VulkanComputeImageTarget.h>

#include <LumoBackend/Utils/Mesh/VertexStruct.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/ResizerInterface.h>
#include <LumoBackend/Interfaces/ShaderUpdateInterface.h>

enum class GenericType : uint8_t {
    NONE = 0,
    PIXEL,       // vertex + fragment
    COMPUTE_1D,  // compute image
    COMPUTE_2D,  // compute image
    COMPUTE_3D,  // compute free
    RTX,         // compute free
    Count
};

class LUMO_BACKEND_API ShaderPass : public conf::ConfigAbstract, public ShaderUpdateInterface, public ResizerInterface, public GuiInterface {
#ifdef VULKAN_DEBUG
public:
    const char* m_RenderDocDebugName = nullptr;
    ct::fvec4 m_RenderDocDebugColor = 0.0f;
#endif

protected:  // internal struct
    struct ShaderCode {
        std::vector<unsigned int> m_SPIRV;  // SPIRV Bytes
        std::string m_Code;                 // Shader Code (in clear)
        std::string m_FilePathName;         // file path name on disk drive
        std::string m_EntryPoint;           // entry point
        vk::ShaderModule m_ShaderModule = nullptr;
        bool m_Used = false;  // say if a sahder mut be take into account
        vk::ShaderStageFlagBits m_ShaderId = vk::ShaderStageFlagBits::eVertex;
    };

    struct DescriptorSetStruct {
        vk::DescriptorSet m_DescriptorSet = {};
        vk::DescriptorSetLayout m_DescriptorSetLayout = {};
        std::vector<vk::DescriptorSetLayoutBinding> m_LayoutBindings = {};
        std::vector<vk::WriteDescriptorSet> m_WriteDescriptorSets = {};
    };

    struct PipelineStruct {
        vk::PipelineLayout m_PipelineLayout = {};
        vk::Pipeline m_Pipeline = {};
    };

private:
    bool m_NeedNewUBOUpload = false;
    bool m_NeedNewSBOUpload = false;
    bool m_NeedNewModelUpdate = false;

private:
    // compute stage
    ct::uvec3 m_DispatchSize = 1U;    // COMPUTE dispatch size
    ct::uvec3 m_LocalGroupSize = 1U;  // COMPUTE local group size

private:
    bool m_CanDynamicallyChangePrimitiveTopology = false;
    vk::PrimitiveTopology m_BasePrimitiveTopology = vk::PrimitiveTopology::eTriangleList;
    vk::PrimitiveTopology m_DynamicPrimitiveTopology = vk::PrimitiveTopology::eTriangleList;

private:  // Tesselation
    uint32_t m_PatchControlPoints = 3U;

protected:
    bool m_Loaded = false;
    bool m_DontUseShaderFilesOnDisk = false;

    GaiApi::VulkanCoreWeak m_VulkanCore;  // vulkan core
    GaiApi::VulkanQueue m_Queue;          // queue
    vk::CommandPool m_CommandPool;        // command pool
    vk::DescriptorPool m_DescriptorPool;  // descriptor pool
    vk::Device m_Device;                  // device copy

    vk::SampleCountFlagBits m_SampleCount = vk::SampleCountFlagBits::e1;

    uint32_t m_CountColorBuffers = 0U;

    bool m_ForceFBOClearing = false;

    bool m_CanWeRenderPass = true;

    vk::RenderPass* m_NativeRenderPassPtr = nullptr;
    vk::RenderPass* m_RenderPassPtr = nullptr;

    // Framebuffer
    FrameBufferWeak m_LoanedFrameBufferWeak;  // when loaned
    FrameBufferPtr m_FrameBufferPtr = nullptr;
    ComputeBufferPtr m_ComputeBufferPtr = nullptr;
    bool m_ResizingByResizeEventIsAllowed = true;
    bool m_ResizingByHandIsAllowed = true;
    float m_BufferQuality = 1.0f;  // smaller value give bigger resolution

    // dynamic state
    vk::Rect2D m_RenderArea = {};
    vk::Viewport m_Viewport = {};
    ct::fvec2 m_OutputSize;
    float m_OutputRatio = 1.0f;

    std::map<vk::ShaderStageFlagBits, std::set<ShaderEntryPoint>> m_ShaderEntryPoints;
    std::map<vk::ShaderStageFlagBits, std::map<ShaderEntryPoint, std::vector<ShaderCode>>> m_ShaderCodes;

    bool m_IsShaderCompiled = false;
    bool m_DescriptorWasUpdated = false;

    bool m_CanUpdateMipMapping = false;

    // ressources
    std::vector<DescriptorSetStruct> m_DescriptorSets = {DescriptorSetStruct()};

    // m_Pipelines[0]
    std::vector<PipelineStruct> m_Pipelines = {PipelineStruct()};  // one entry by default
    vk::PipelineCache m_PipelineCache = {};
    std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderCreateInfos;
    std::vector<vk::PipelineColorBlendAttachmentState> m_BlendAttachmentStates;
    std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_RayTracingShaderGroups;  // Shader groups

    // x:inf, y:sup, z:defaut, w:value
    ct::uvec4 m_CountVertexs = ct::uvec4(0U, 0U, 6U, 6U);      // count vertex to draw
    ct::uvec4 m_CountInstances = ct::uvec4(1U, 1U, 1U, 1U);    // count instances to draw
    ct::uvec4 m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);  // rendering iterations loop

    std::unordered_map<std::string, bool> m_UsedUniforms;  // Used Uniforms

    GenericType m_RendererType = GenericType::NONE;  // Renderer Type

    bool m_BlendingEnabled = false;

    bool m_MergedRendering = false;

    uint32_t m_LastExecutedFrame = 0U;

    ct::fvec4 m_LineWidth = ct::fvec4(0.0f, 0.0f, 0.0f, 1.0f);  // line width
    vk::PolygonMode m_PolygonMode = vk::PolygonMode::eFill;
    vk::CullModeFlagBits m_CullMode = vk::CullModeFlagBits::eNone;
    vk::FrontFace m_FrontFaceMode = vk::FrontFace::eCounterClockwise;

    VertexStruct::PipelineVertexInputState m_InputState;

    vk::PushConstantRange m_Internal_PushConstants;

    bool m_Tesselated = false;
    std::string m_HeaderCode;
    std::string m_VertexCode;
    std::string m_FragmentCode;
    std::string m_GeometryCode;
    std::string m_TesselationEvaluationCode;
    std::string m_TesselationControlCode;
    std::string m_ComputeCode;
    std::string m_RayGenerationCode;
    std::string m_RayIntersectionCode;
    std::string m_RayMissCode;
    std::string m_RayAnyHitCode;
    std::string m_RayClosestHitCode;

public:
    ShaderPass(GaiApi::VulkanCoreWeak vVulkanCore);
    ShaderPass(GaiApi::VulkanCoreWeak vVulkanCore, const GenericType& vRendererTypeEnum);
    ShaderPass(GaiApi::VulkanCoreWeak vVulkanCore, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);
    ShaderPass(
        GaiApi::VulkanCoreWeak vVulkanCore, const GenericType& vRendererTypeEnum, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);
    virtual ~ShaderPass();

    // during init
    virtual void ActionBeforeInit();
    virtual void ActionAfterInitSucceed();
    virtual void ActionAfterInitFail();

    // init
    virtual bool InitPixelWithoutFBO(                                                // Init for use with external fbo aned renderpass
        const ct::uvec2& vSize,                                                      // fbo size
        const uint32_t& vCountColorBuffers,                                          // count coor attachments
        const bool& vTesselated,                                                     // use tesselation pipeline
        vk::RenderPass* vRenderPassPtr,                                              // external render pass
        const vk::SampleCountFlagBits& vSampleCount = vk::SampleCountFlagBits::e1);  // image sampling
    virtual bool InitPixel(                                                          // Init fbo 2d for pixel
        const ct::uvec2& vSize,                                                      // fbo size
        const uint32_t& vCountColorBuffers,                                          // count color attachments
        const bool& vUseDepth,                                                       // use depth merging
        const bool& vNeedToClear,                                                    // need to clear
        const ct::fvec4& vClearColor,                                                // color used to clear
        const bool& vPingPongBufferMode,                                             // will create pingpong fbos
        const bool& vTesselated,                                                     // use tesselation pipeline
        const vk::Format& vFormat = vk::Format::eR32G32B32A32Sfloat,                 // image format
        const vk::SampleCountFlagBits& vSampleCount = vk::SampleCountFlagBits::e1);  // image sampling
    virtual bool InitCompute1D(                                                      // compute 1D
        const uint32_t& vDispatchSize);                                              // compute size
    virtual bool InitCompute2D(                                                      // compute 2D
        const ct::uvec2& vDispatchSize,                                              // compute size
        const uint32_t& vCountColorBuffers,                                          // count color attachments
        const bool& vPingPongBufferMode,                                             // will create pingpong fbos
        const vk::Format& vFormat);                                                  // image format, (no sampling possible with compute images)
    virtual bool InitCompute3D(                                                      // compute 3D
        const ct::uvec3& vDispatchSize);                                             // compute size
    virtual bool InitRtx(                                                            // RTX pipeline
        const ct::uvec2& vDispatchSize,                                              // compute size
        const uint32_t& vCountColorBuffers,                                          // count color attachments
        const bool& vPingPongBufferMode,                                             // will create pingpong fbos
        const vk::Format& vFormat);                                                  // image format, (no sampling possible with compute images)
    virtual void Unit();

    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

    void SetFrameBuffer(FrameBufferWeak vFrameBufferWeak);

    void SetMergedRendering(const bool& vMergedRendering);

    // for have widget display when used as SHADER_PASS in MERGER instead of module as classic task
    void SetLastExecutedFrame(const uint32_t& vFrame) {
        m_LastExecutedFrame = vFrame;
    }
    uint32_t GetLastExecutedFrame() const {
        return m_LastExecutedFrame;
    }

    void SetRenderDocDebugName(const char* vLabel, ct::fvec4 vColor);

    /// <summary>
    /// to call when from resize event
    /// </summary>
    /// <param name="vNewSize"></param>
    /// <param name="vCountColorBuffers"></param>
    void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers = nullptr) override;  // to call at any moment

    /// <summary>
    /// will resize if the new size is different
    /// </summary>
    /// <param name="vNewSize"></param>
    /// <returns></returns>
    bool NeedResizeByResizeEventIfChanged(ct::ivec2 vNewSize);

    /// <summary>
    /// to call when from hand
    /// </summary>
    /// <param name="vNewSize"></param>
    /// <param name="vCountColorBuffers"></param>
    void NeedResizeByHand(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers = nullptr) override;  // to call at any moment

    /// <summary>
    /// will resize if the new size is different
    /// </summary>
    bool NeedResizeByHandIfChanged(ct::ivec2 vNewSize);

    /// <summary>
    /// will resize if the resize is need by FBO or compute buffers
    /// </summary>
    /// <returns></returns>
    bool ResizeIfNeeded();

    /// <summary>
    /// to call (herited), inform than a resized was jsut done
    /// </summary>
    virtual void WasJustResized();

    /// <summary>
    /// will update RenderArea and Viewport for the viewport state of the pipeline event if the rnederpass is external
    /// </summary>
    /// <param name="vNewSize"></param>
    void UpdatePixel2DViewportSize(const ct::uvec2& vNewSize);

    ct::fvec2 GetOutputSize();
    ct::fvec2 GetOutputRatio();

    // Renderer Type
    bool IsPixelRenderer();
    bool IsCompute1DRenderer();
    bool IsCompute2DRenderer();
    bool IsCompute3DRenderer();
    bool IsRtxRenderer();

    // FBO
    FrameBufferWeak GetFrameBuffer() {
        return m_FrameBufferPtr;
    }
    void UpdateFBOColorBuffersCount(const uint32_t& vNewColorBufferCount);

    // anble the MipMapping Generation update after rendering
    void SetMipMappingGenerationAfterRendering(const bool& vFlag) {
        m_CanUpdateMipMapping = vFlag;
    }

    /// <summary>
    /// set the compute local group size
    /// must be set before call to SetDispatchSize functions
    /// </summary>
    /// <param name="vLocalGroupSize"></param>
    void SetLocalGroupSize(const ct::uvec3& vLocalGroupSize);

    /// <summary>
    /// set the dispatch size according to local group size
    /// must be called after call to SetLocalGroupSize
    /// </summary>
    /// <param name="vDispatchSize"></param>
    void SetDispatchSize1D(const uint32_t& vDispatchSize);
    void SetDispatchSize2D(const ct::uvec2& vDispatchSize);
    void SetDispatchSize3D(const ct::uvec3& vDispatchSize);

    /// <summary>
    /// get the dispatch size
    /// </summary>
    const ct::uvec3& GetDispatchSize();

    /// <summary>
    /// will execute vCmdBufferPtr->disaptch according to the defined dispatchsize
    /// </summary>
    void Dispatch(vk::CommandBuffer* vCmdBufferPtr, const char* vDebugLabel);

    // Set Wigdet limits
    // vertex / count point to draw / count instances / etc..
    void SetCountVertexs(const uint32_t& vCountVertexs);
    void SetCountInstances(const uint32_t& vCountInstances);
    void SetCountIterations(const uint32_t& vCountIterations);
    void SetLineWidth(const float& vLineWidth);

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    // return true for continue xml parsing of childs in this node or false for interupt the child exploration
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

    // swap and write output descriptors imageinfos, in case of multipass
    // xecuted jsut before compute()
    virtual void SwapMultiPassFrontBackDescriptors();

    bool StartDrawPass(vk::CommandBuffer* vCmdBufferPtr);
    void DrawPass(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber = 1U);
    void EndDrawPass(vk::CommandBuffer* vCmdBufferPtr);

    // used to set another rnederpass from another fbo, like in scene merger
    // will rebuild the pipeline
    void SetRenderPass(vk::RenderPass* vRenderPassPtr);

    // reset renderpass to nativz renderpass
    void ReSetRenderPassToNative();

    /// <summary>
    /// chnage the primitive topology if enebled with m_CanDynamicallyChangePrimitiveTopology
    /// return true if the vPrimitiveTopology was accepted
    /// vForce will recreate the pipeline
    /// </summary>
    /// <returns>return true if the vPrimitiveTopology was accepeted</returns>
    bool ChangeDynamicPrimitiveTopology(const vk::PrimitiveTopology& vPrimitiveTopology, const bool& vForce = false);

    /// <summary>
    /// enable or disable the dynamic change of primitive topology
    /// </summary>
    void SetDynamicallyChangePrimitiveTopology(const bool& vFlag);

    /// <summary>
    /// will define the base primitive topology
    /// </summary>
    /// <param name="vNewSize"></param>
    void SetPrimitveTopology(const vk::PrimitiveTopology& vPrimitiveTopology);

    /// <summary>
    /// get the family of a Primitive Topology
    /// </summary>
    vk::PrimitiveTopology GetPrimitiveTopologyFamily(const vk::PrimitiveTopology& vPrimitiveTopology);

    // draw primitives
    bool AreWeValidForRender();  // pre condition check, like pipeline validity
    virtual bool CanRender();    // user defined
    virtual void ActionBeforeDrawInCommandBuffer(vk::CommandBuffer* vCmdBufferPtr);
    virtual void DrawModel(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber);
    virtual void ActionAfterDrawInCommandBuffer(vk::CommandBuffer* vCmdBufferPtr);
    virtual void Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber);
    virtual void TraceRays(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber);

    virtual bool CanUpdateDescriptors();
    virtual void UpdateRessourceDescriptor();

    // shader update from file
    void UpdateShaders(const std::set<std::string>& vFiles) override;

    void NeedToClearFBOThisFrame();

    void SetHeaderCode(const std::string& vHeaderCode);
    void SetVertexShaderCode(const std::string& vShaderCode);
    void SetFragmentShaderCode(const std::string& vShaderCode);
    void SetGeometryShaderCode(const std::string& vShaderCode);
    void SetTesselationEvaluationShaderCode(const std::string& vShaderCode);
    void SetTesselationControlShaderCode(const std::string& vShaderCode);
    void SetComputeShaderCode(const std::string& vShaderCode);
    void SetRayGenerationShaderCode(const std::string& vShaderCode);
    void SetRayIntersectionShaderCode(const std::string& vShaderCode);
    void SetRayMissShaderCode(const std::string& vShaderCode);
    void SetRayAnyHitShaderCode(const std::string& vShaderCode);
    void SetRayClosestHitShaderCode(const std::string& vShaderCode);

    virtual bool ReCompilCode();

    // Texture Use Helper
    void EnableTextureUse(const uint32_t& vBindingPoint, const uint32_t& vTextureSLot, float& vTextureUseVar);
    void DisableTextureUse(const uint32_t& vBindingPoint, const uint32_t& vTextureSLot, float& vTextureUseVar);

    vk::DescriptorImageInfo GetBackImageInfo(uint32_t vIndex);
    vk::DescriptorImageInfo GetFrontImageInfo(uint32_t vIndex);
    GaiApi::VulkanFrameBuffer* GetBackFbo();
    GaiApi::VulkanFrameBuffer* GetFrontFbo();
    VulkanImageObjectPtr GetBackImage(uint32_t vIndex);
    VulkanImageObjectPtr GetFrontImage(uint32_t vIndex);
    GaiApi::VulkanComputeImageTarget* GetBackTarget(uint32_t vIndex);
    GaiApi::VulkanComputeImageTarget* GetFrontTarget(uint32_t vIndex);

protected:  // IMGUI
    bool DrawResizeWidget();

private:
    void NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers);  // to call at any moment
    void NeedResize(ct::ivec2* vNewSize);                                      // to call at any moment
    void Resize(const ct::uvec2& vNewSize);

protected:  // auto resize helper
    /// <summary>
    /// will ask to the buffer for resize if the buffer size and the parent size mismatch
    /// its just an helper
    /// </summary>
    /// <param name="vOutputSizePtr">the buffer</param>
    /// <param name="vOutSizePtr">the parent sier</param>
    void AutoResizeBuffer(OutputSizeInterface* vBufferOutSizePtr, ct::fvec2* vParentOutSizePtr);

protected:
    virtual void ActionBeforeCompilation();
    virtual void ActionAfterCompilation();

    void ClearShaderEntryPoints();
    void AddShaderEntryPoints(const vk::ShaderStageFlagBits& vShaderId, const ShaderEntryPoint& vEntryPoint);
    void SetShaderUse(const vk::ShaderStageFlagBits& vShaderId, const ShaderEntryPoint& vEntryPoint, const bool& vUsed);

    // Get Shaders
    virtual std::string GetVertexShaderCode(std::string& vOutShaderName);
    virtual std::string GetFragmentShaderCode(std::string& vOutShaderName);
    virtual std::string GetGeometryShaderCode(std::string& vOutShaderName);
    virtual std::string GetTesselationEvaluationShaderCode(std::string& vOutShaderName);
    virtual std::string GetTesselationControlShaderCode(std::string& vOutShaderName);
    virtual std::string GetComputeShaderCode(std::string& vOutShaderName);
    virtual std::string GetRayGenerationShaderCode(std::string& vOutShaderName);
    virtual std::string GetRayIntersectionShaderCode(std::string& vOutShaderName);
    virtual std::string GetRayMissShaderCode(std::string& vOutShaderName);
    virtual std::string GetRayAnyHitShaderCode(std::string& vOutShaderName);
    virtual std::string GetRayClosestHitShaderCode(std::string& vOutShaderName);

    ShaderCode& AddShaderCode(const ShaderCode& vShaderCode, const ShaderEntryPoint& vEntryPoint);
    ShaderCode CompilShaderCode(const vk::ShaderStageFlagBits& vShaderType,
        const std::string& vCode,
        const std::string& vShaderName,
        const std::string& vEntryPoint = "main");
    ShaderCode CompilShaderCode(const vk::ShaderStageFlagBits& vShaderType, const std::string& vEntryPoint = "main");
    virtual const std::vector<unsigned int> CompilGLSLToSpirv(const std::string& vCode,
        const std::string& vShaderSuffix,
        const std::string& vOriginalFileName,
        const ShaderEntryPoint& vEntryPoint = "main");
    virtual bool CompilPixel();
    virtual bool CompilCompute();
    virtual bool CompilRtx();

    // this special descriptor update will be executed each frames
    virtual void EachFramesDescriptorUpdate();

    virtual bool BuildModel();
    void NeedNewModelUpdate();
    virtual void DestroyModel(const bool& vReleaseDatas = false);
    void UpdateModel(const bool& vLoaded);

    // Uniform Buffer Object
    virtual bool CreateUBO();
    void NeedNewUBOUpload();
    virtual void UploadUBO();
    virtual void DestroyUBO();

    // Storage Buffer Object
    virtual bool CreateSBO();
    void NeedNewSBOUpload();
    virtual void UploadSBO();
    virtual void DestroySBO();

    // Ressource Descriptor
    virtual bool UpdateRessourceDescriptors();
    virtual bool UpdateLayoutBindingInRessourceDescriptor();
    virtual bool UpdateBufferInfoInRessourceDescriptor();

    // MipMapping
    void UpdateMipMappingIfNeeded();

    void ClearWriteDescriptors();
    void ClearWriteDescriptors(const uint32_t& vDescriptorSetIndex);
    bool AddOrSetWriteDescriptorImage(const uint32_t& vBindingPoint,
        const vk::DescriptorType& vType,
        const vk::DescriptorImageInfo* vImageInfo,
        const uint32_t& vCount = 1U,
        const uint32_t& vDescriptorSetIndex = 0U);
    bool AddOrSetWriteDescriptorBuffer(const uint32_t& vBindingPoint,
        const vk::DescriptorType& vType,
        const vk::DescriptorBufferInfo* vBufferInfo,
        const uint32_t& vCount = 1U,
        const uint32_t& vDescriptorSetIndex = 0U);
    bool AddOrSetWriteDescriptorBufferView(const uint32_t& vBindingPoint,
        const vk::DescriptorType& vType,
        const vk::BufferView* vBufferView,
        const uint32_t& vCount = 1U,
        const uint32_t& vDescriptorSetIndex = 0U);
    bool AddOrSetWriteDescriptorNext(const uint32_t& vBindingPoint,
        const vk::DescriptorType& vType,
        const void* vNext,
        const uint32_t& vCount = 1U,
        const uint32_t& vDescriptorSetIndex = 0U);

    void ClearLayoutDescriptors();
    void ClearLayoutDescriptors(const uint32_t& vDescriptorSetIndex);
    bool AddOrSetLayoutDescriptor(const uint32_t& vBindingPoint,
        const vk::DescriptorType& vType,
        const vk::ShaderStageFlags& vStage,
        const uint32_t& vCount = 1U,
        const uint32_t& vDescriptorSetIndex = 0U);

    // will produce errors eventually only in debug, for dev debugging
    void CheckLayoutBinding(const uint32_t& vDescriptorSetIndex = 0U);
    void CheckWriteDescriptors(const uint32_t& vDescriptorSetIndex = 0U);

    bool CreateRessourceDescriptor();
    void DestroyRessourceDescriptor();

    // push constants
    void SetPushConstantRange(const vk::PushConstantRange& vPushConstantRange);

    // Pipelines
    virtual void SetInputStateBeforePipelineCreation();  // for doing this kind of thing VertexStruct::P2_T2::GetInputState(m_InputState);
    virtual bool CreateComputePipeline();
    virtual bool CreatePixelPipeline();
    virtual bool CreateRtxPipeline();
    void DestroyPipeline();
};
