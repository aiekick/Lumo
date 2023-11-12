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
#include <string>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Headers/LumoBackendDefs.h>
#include <LumoBackend/Base/Base.h>

#include <LumoBackend/Utils/Mesh/VertexStruct.h>

#include <Gaia/gaia.h>

#include <LumoBackend/Interfaces/ShaderUpdateInterface.h>
#include <LumoBackend/Interfaces/ResizerInterface.h>
#include <LumoBackend/Interfaces/GuiInterface.h>

#include <LumoBackend/Base/ShaderPass.h>

#ifdef PROFILER_INCLUDE
#include PROFILER_INCLUDE
#else
#define TracyVkCtx void*
#endif

// le BaseRenderer est le digne successeur de BaseRenderer en Opengl
// il va supporter les vertex/fragment en mode, point, quad et opengl
// et spêcifiquement les compute
// il pourra etre derivé et certaine focntionnalité dependant en temps normal du shader
// pouront etre bloqué en code pour specialisation

class LUMO_BACKEND_API BaseRenderer :
	public conf::ConfigAbstract,
	public GuiInterface,
	public ShaderUpdateInterface,
	public ResizerInterface {
private:
	bool m_NeedNewUBOUpload = true;			// true for first render
	bool m_NeedNewSBOUpload = true;			// true for first render

protected:
    BaseRendererWeak m_This;  // pointer to this for affected in other op

    TracyVkCtx m_TracyContext = nullptr;  // Tracy Profiler Context
    std::string m_FilePathName;           // file path name when loaded from it for GENERIC_BEHAVIOR_FILE
    uint32_t m_BufferIdToResize = 0U;     // buffer id to resize (mostly used in compute, because in pixel, all attachments must have same size)
    bool m_IsRenderPassExternal = false;  // true if the renderpass is not created here, but come from external (inportant for not destroy him)
	
	uint32_t m_CurrentFrame = 0U;			// current frame of double buffering
	uint32_t m_LastFrame = 0U;				// last frame of double buffering

	std::set<std::string> m_UniformSectionToShow;	// uniform shader stage to show

	std::string m_RendererName;				// will use it as main name for display widget

	bool m_NeedResize = false;				// will be resized if true
	bool m_Loaded = false;					// if shader operationnel
	bool m_CanWeRender = true;				// rendering is permited of paused
	bool m_JustReseted = false;				// when shader was reseted
	bool m_FirstRender = true;				// 1er rendu
    bool m_MergedRendering = false;         // rendering in merged mode, so pass fbo will not been used

	vk::DebugUtilsLabelEXT markerInfo;		// marker info for vkCmdBeginDebugUtilsLabelEXT
	bool m_DebugLabelWasUsed = false;		// used for say than a vkCmdBeginDebugUtilsLabelEXT was used for name a section in renderdoc

	int64_t m_FirstTimeMark = 0LL;			// first mark time for deltatime Calculation
	int64_t m_SecondTimeMark = 0LL;			// last mark time for deltatime Calculation
	float m_DeltaTime = 0.0f;				// last frame render time
	uint32_t m_Frame = 0U;					// frame count
	uint32_t m_CountBuffers = 0U;			// FRAGMENT count framebuffer color attachment from 0 to 7

	// vulkan creation
	GaiApi::VulkanCorePtr m_VulkanCorePtr = nullptr;	// vulkan core
	GaiApi::VulkanQueue m_Queue;					// queue
	vk::CommandPool m_CommandPool;				// command pool
	vk::DescriptorPool m_DescriptorPool;		// descriptor pool
	vk::Device m_Device;						// device copy

	// Submition
	std::vector<vk::Semaphore> m_RenderCompleteSemaphores;
	std::vector<vk::Fence> m_WaitFences;
	std::vector<vk::CommandBuffer> m_CommandBuffers;

	// dynamic state
	vk::Rect2D m_RenderArea = {};
	vk::Viewport m_Viewport = {};
	ct::uvec3 m_OutputSize;							// output size for compute stage
	float m_OutputRatio = 1.0f;

	// clear Color
	std::vector<vk::ClearValue> m_ClearColorValues;

	std::vector<ShaderPassWeak> m_ShaderPasses;

public: // contructor
	BaseRenderer(GaiApi::VulkanCorePtr vVulkanCorePtr);
	BaseRenderer(GaiApi::VulkanCorePtr vVulkanCorePtr, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);
	virtual ~BaseRenderer();

	// Generic Renderer Pass
	bool AddGenericPass(ShaderPassWeak vPass);
	ShaderPassWeak GetGenericPass(const uint32_t& vIdx);
	void ClearGenericPasses();

	// during init
	virtual void ActionBeforeInit();
	virtual void ActionAfterInitSucceed();
	virtual void ActionAfterInitFail();

	// init/unit
	bool InitPixel(const ct::uvec2& vSize);
	bool InitCompute1D(const uint32_t& vSize);
	bool InitCompute2D(const ct::uvec2& vSize);
	bool InitCompute3D(const ct::uvec3& vSize);
	bool InitRtx(const ct::uvec2& vSize);
	void Unit();

	// resize
	void NeedResizeByHand(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers = nullptr) override; // to call at any moment
	void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers = nullptr) override; // to call at any moment
	virtual bool ResizeIfNeeded();

	// Base : one render for one FBO
	bool BeginRender(const char* vSectionLabel);
    void EndRender();

	// rendering of pass in swapchain, set all passes with this flag
    void SetMergedRendering(const bool& vMergedRendering);

	// render
	virtual void RenderShaderPasses(vk::CommandBuffer* vCmdBufferPtr);
    void Render(const char* vSectionLabel = nullptr, vk::CommandBuffer* vCmdBufferPtr = nullptr);

	virtual void UpdateDescriptorsBeforeCommandBuffer();

	// Get
	vk::Viewport GetViewport() const;
	vk::Rect2D GetRenderArea() const;
	ct::fvec2 GetOutputSize() const;
	float GetOutputRatio() const;

	// Rendering
	bool ResetFence();
	vk::CommandBuffer* GetCommandBuffer();
	void BeginProfilerFrame(const char* vFrameName);
	void ResetCommandBuffer();
	void BeginCommandBuffer(const char* vSectionLabel);
	void EndCommandBuffer();
	void SubmitPixel();
	void SubmitCompute();
	bool WaitFence();
    void Swap();

    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;

	virtual void ResetFrame();

	void UpdateShaders(const std::set<std::string>& vFiles);

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas);
	// return true for continue xml parsing of childs in this node or false for interupt the child exploration
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas);

protected:
	virtual void DoBeforeEndCommandBuffer(vk::CommandBuffer* vCmdBufferPtr);

	// CommandBuffer
	bool CreateCommanBuffer();
	void DestroyCommanBuffer();

	// Sync / Semaphore / Fence
	bool CreateSyncObjects();
	void DestroySyncObjects();
};