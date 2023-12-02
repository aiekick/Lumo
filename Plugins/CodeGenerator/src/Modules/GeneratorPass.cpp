#include "GeneratorNode.h"

#include <filesystem>
#include <ImGuiPack.h>
#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <Headers/GeneratorCommon.h>
#include <Slots/GeneratorNodeSlotInput.h>
#include <Slots/GeneratorNodeSlotOutput.h>
#include <LumoBackend/Graph/Base/NodeSlot.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Graph/Slots/NodeSlotTaskInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTaskOutput.h>
#include <LumoBackend/Graph/Slots/NodeSlotModelInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotModelOutput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureOutput.h>
#include <LumoBackend/Graph/Slots/NodeSlotVariableInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotVariableOutput.h>
#include <LumoBackend/Graph/Slots/NodeSlotLightGroupInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotLightGroupOutput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexelBufferInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexelBufferOutput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureGroupInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureGroupOutput.h>
#include <LumoBackend/Graph/Slots/NodeSlotStorageBufferInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotStorageBufferOutput.h>

namespace fs = std::filesystem;

//////////////////////////////////////////////////////////////////////////////////////////////
//// GENERATOR PASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorNode::GeneratePasseClasses(const std::string& vPath, const SlotDico& vDico) {
    fs::path path_path = vPath + "/Modules/" + m_CategoryName + "/Pass/";

    if (m_IsAnEffect) {
        path_path = vPath + "/Modules/" + m_CategoryName + "/Effects/Pass/";
    } 

    if (!std::filesystem::exists(path_path))
        fs::create_directory(path_path);

	std::string pass_renderer_display_type = GetRendererDisplayName();
	std::string pass_class_name = m_ClassName + "Module_" + pass_renderer_display_type + "Pass";
	std::string cpp_pass_file_name = path_path.string() + "/" + pass_class_name + ".cpp";
	std::string h_pass_file_name = path_path.string() + "/" + pass_class_name + ".h";

	std::string cpp_pass_file_code;
	std::string h_pass_file_code;

	cpp_pass_file_code += GetLicenceHeader();
	h_pass_file_code += GetLicenceHeader();
	cpp_pass_file_code += GetPVSStudioHeader();

	// MODULE_XML_NAME							grayscott_module_sim
	// MODULE_DISPLAY_NAME						Gray Scott
	// MODULE_CATEGORY_NAME						Simulation
	// MODULE_CLASS_NAME		
	// PASS_CLASS_NAME
	// MODULE_RENDERER_INIT_FUNC				InitCompute2D
	// RENDERER_DISPLAY_TYPE (ex (Comp)			Comp

    cpp_pass_file_code +=
        u8R"(
#include "PASS_CLASS_NAME.h"

#include <cinttypes>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImGuiPack.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Buffer/FrameBuffer.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif
#ifndef VKFPScoped
#define VKFPScoped
#endif

//////////////////////////////////////////////////////////////
///// STATIC /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
)";

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D) {
            cpp_pass_file_code +=
                u8R"(
std::shared_ptr<PASS_CLASS_NAME> PASS_CLASS_NAME::Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore) {
	auto res_ptr = std::make_shared<PASS_CLASS_NAME>(vVulkanCore);
	if (!res_ptr->InitPixel(vSize, 1U, true, true, 0.0f, false, false, vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1)))";
    } else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D) {
        cpp_pass_file_code +=
            u8R"(
std::shared_ptr<PASS_CLASS_NAME> PASS_CLASS_NAME::Create(const uint32_t& vSize, GaiApi::VulkanCoreWeak vVulkanCore) {
	auto res_ptr = std::make_shared<PASS_CLASS_NAME>(vVulkanCore);
	if (!res_ptr->InitCompute1D(vSize)) {)";
    } else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D) {
        cpp_pass_file_code +=
            u8R"(
std::shared_ptr<PASS_CLASS_NAME> PASS_CLASS_NAME::Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore) {
	auto res_ptr = std::make_shared<PASS_CLASS_NAME>(vVulkanCore);
	if (!res_ptr->InitCompute2D(vSize, 1U, false, vk::Format::eR32G32B32A32Sfloat)) {)";
    } else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D) {
        cpp_pass_file_code +=
            u8R"(
std::shared_ptr<PASS_CLASS_NAME> PASS_CLASS_NAME::Create(const ct::uvec3& vSize, GaiApi::VulkanCoreWeak vVulkanCore) {
	auto res_ptr = std::make_shared<PASS_CLASS_NAME>(vVulkanCore);
	if (!res_ptr->InitCompute3D(vSize)) {)";
    } else if (m_RendererType == RENDERER_TYPE_RTX) {
        cpp_pass_file_code +=
            u8R"(
std::shared_ptr<PASS_CLASS_NAME> PASS_CLASS_NAME::Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore) {
	auto res_ptr = std::make_shared<PASS_CLASS_NAME>(vVulkanCore);
	if (!res_ptr->InitRtx(vSize, 1U, false, vk::Format::eR32G32B32A32Sfloat)) {)";
    }
cpp_pass_file_code += u8R"(
		res_ptr.reset();
	}
	return res_ptr;
}

//////////////////////////////////////////////////////////////
///// CTOR / DTOR ////////////////////////////////////////////
//////////////////////////////////////////////////////////////

PASS_CLASS_NAME::PASS_CLASS_NAME(GaiApi::VulkanCoreWeak vVulkanCore))";

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD)
		{
			cpp_pass_file_code += u8R"(
	: QuadShaderPass(vVulkanCore, MeshShaderPassType::PIXEL))";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH ||
			m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_TESSELATION)
		{
			cpp_pass_file_code += ct::toStr(u8R"(
	: MeshShaderPass<VertexStruct::%s>(vVulkanCore, MeshShaderPassType::PIXEL))", 
				m_BaseTypes.m_VertexStructTypes[m_VertexStructTypesIndex].c_str());
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			cpp_pass_file_code += u8R"(
	: VertexShaderPass(vVulkanCore))";
		}
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D)
	{
		cpp_pass_file_code += u8R"(
	: ShaderPass(vVulkanCore))";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D)
	{
		cpp_pass_file_code += u8R"(
	: ShaderPass(vVulkanCore))";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		cpp_pass_file_code += u8R"(
	: ShaderPass(vVulkanCore))";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
		cpp_pass_file_code += u8R"(
	: RtxShaderPass(vVulkanCore))";
	}

	cpp_pass_file_code += u8R"( {)";

	cpp_pass_file_code += u8R"(
	ZoneScoped;
	SetRenderDocDebugName("RENDERER_DISPLAY_TYPE Pass : MODULE_DISPLAY_NAME", COMPUTE_SHADER_PASS_DEBUG_COLOR);)";

	cpp_pass_file_code +=
        u8R"(
	m_DontUseShaderFilesOnDisk = true;)";

	if (m_IsAnEffect) {
        cpp_pass_file_code +=
            u8R"(
	*IsEffectEnabled() = true;)";
    }

    cpp_pass_file_code +=
        u8R"(
}

PASS_CLASS_NAME::~PASS_CLASS_NAME() {
	ZoneScoped;
	Unit();
}
)";
	if (m_RendererType == RENDERER_TYPE_RTX)
	{
		cpp_pass_file_code += u8R"(
void PASS_CLASS_NAME::ActionBeforeCompilation() {
	AddShaderCode(CompilShaderCode(vk::ShaderStageFlagBits::eRaygenKHR, "main"), "main");
	AddShaderCode(CompilShaderCode(vk::ShaderStageFlagBits::eMissKHR, "main"), "main");
	AddShaderCode(CompilShaderCode(vk::ShaderStageFlagBits::eClosestHitKHR, "main"), "main");
}
)";
	}

		cpp_pass_file_code += u8R"(
void PASS_CLASS_NAME::ActionBeforeInit() {
	ZoneScoped;
	//m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);)";

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		//if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			cpp_pass_file_code += u8R"(
	//SetPrimitveTopology(vk::PrimitiveTopology::eTriangleList); // display Triangles
	//m_LineWidth.x = 0.5f;	// min value
	//m_LineWidth.y = 10.0f;	// max value
	//m_LineWidth.z = 2.0f;	// default value
	//m_LineWidth.w;			// value to change)";
		}
	}

	if (m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_StorageBuffer])
	{
		cpp_pass_file_code += u8R"(
	for (auto& info : m_StorageBuffers)	{
		info = corePtr->getEmptyDescriptorBufferInfo();
	})";
	}
	else if (m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_TexelBuffer])
	{
		cpp_pass_file_code += u8R"(
	for (auto& info : m_TexelBuffers) {
		info = nullptr;
	}
	for (auto& info : m_TexelBufferViews) {
		info = corePtr->getEmptyBufferView();
	})";
	}
	else if (m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_Texture])
	{
		cpp_pass_file_code += u8R"(
	for (auto& info : m_ImageInfos)	{
		info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
	})";
	}
	else if (m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_TextureCube])
	{
		cpp_pass_file_code += u8R"(
	for (auto& info : m_ImageCubeInfos)	{
		info = corePtr->getEmptyTextureCubeDescriptorImageInfo();
	})";
	}
	else if (m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_TextureGroup])
	{
		cpp_pass_file_code += u8R"(
	for (auto& infos : m_ImageGroups) {
		for(auto& info : infos)
		{
			info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
		}
	})";
	}

	cpp_pass_file_code +=
        u8R"(
}

bool PASS_CLASS_NAME::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	bool change = false;)";
    if (m_IsAnEffect) {
        cpp_pass_file_code +=
            u8R"(
	//change |= DrawResizeWidget();)";
    } else {
        cpp_pass_file_code +=
            u8R"(
	change |= DrawResizeWidget();)";
	}

	if (m_IsAnEffect) {
        cpp_pass_file_code +=
            u8R"(
	if (ImGui::CollapsingHeader_CheckBox("MODULE_DISPLAY_NAME##PASS_CLASS_NAME", -1.0f, false, true, IsEffectEnabled())) {)";
	} else {
        cpp_pass_file_code +=
            u8R"(
	if (ImGui::CollapsingHeader_CheckBox("MODULE_DISPLAY_NAME##PASS_CLASS_NAME", -1.0f, true, true, &m_CanWeRender)) {)";
	}
        
	cpp_pass_file_code += m_UBOEditors.Get_Widgets_Header();

	cpp_pass_file_code +=
            u8R"(
	}
	return change;
}

bool PASS_CLASS_NAME::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	return false;
}

bool PASS_CLASS_NAME::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	return false;
}
)";

	cpp_pass_file_code += GetPassInputCppFuncs(vDico);
	cpp_pass_file_code += GetPassOutputCppFuncs(vDico);

	cpp_pass_file_code += u8R"(
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PASS_CLASS_NAME::WasJustResized() {
	ZoneScoped;
}
)";
	cpp_pass_file_code += GetPassRendererFunctionHeader();
	cpp_pass_file_code += m_UBOEditors.Get_Cpp_Functions_Imp(m_IsAnEffect);

	if (m_UseASbo)
	{
		cpp_pass_file_code += u8R"(
bool PASS_CLASS_NAME::CreateSBO() {
	ZoneScoped;
	NeedNewSBOUpload();
	return true;
}

void PASS_CLASS_NAME::UploadSBO() {
	ZoneScoped;
	//VulkanRessource::upload(m_VulkanCore, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void PASS_CLASS_NAME::DestroySBO() {
	ZoneScoped;
	//m_SBOCompPtr.reset();
	//m_SBO_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}
)";
	}

	if (m_RendererType == RENDERER_TYPE_RTX)
	{
		cpp_pass_file_code += u8R"(
bool PASS_CLASS_NAME::CanUpdateDescriptors() {
	ZoneScoped;
	if (!m_SceneAccelStructure.expired()) {
		auto accelStructurePtr = m_SceneAccelStructure.lock();
		if (accelStructurePtr) {
			return accelStructurePtr->IsOk();
		}
	}
	return false;
}
)";
	}

	cpp_pass_file_code += u8R"(
bool PASS_CLASS_NAME::UpdateLayoutBindingInRessourceDescriptor() {
	ZoneScoped;
	bool res = true;)";
	cpp_pass_file_code += GetPassUpdateLayoutBindingInRessourceDescriptorHeader();
	cpp_pass_file_code += u8R"(
	return res;
}

bool PASS_CLASS_NAME::UpdateBufferInfoInRessourceDescriptor() {
	ZoneScoped;
	bool res = true;)";
	cpp_pass_file_code += GetPassUpdateBufferInfoInRessourceDescriptorHeader();
	cpp_pass_file_code += u8R"(
	return res;
}
)";

	cpp_pass_file_code += GetPassShaderCode();

	cpp_pass_file_code += u8R"(

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string PASS_CLASS_NAME::getXml(const std::string& vOffset, const std::string& vUserDatas) {
	ZoneScoped;
	std::string str;
    str += vOffset + "<PASS_XML_NAME>\n";
	str += ShaderPass::getXml(vOffset + "\t", vUserDatas);)";
	cpp_pass_file_code += m_UBOEditors.Get_Cpp_GetXML(m_IsAnEffect);	
	cpp_pass_file_code += u8R"(
    str += vOffset + "</PASS_XML_NAME>\n";
	return str;
}

bool PASS_CLASS_NAME::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
	ZoneScoped;
	std::string strName;
	std::string strValue;
	std::string strParentName;
	strName = vElem->Value();
	if (vElem->GetText()) {
		strValue = vElem->GetText();
	}
	if (vParent != nullptr) {
		strParentName = vParent->Value();
	}
	if (strParentName == "PASS_XML_NAME") {
		ShaderPass::setFromXml(vElem, vParent, vUserDatas);)";
    cpp_pass_file_code += m_UBOEditors.Get_Cpp_SetXML(m_IsAnEffect);
	cpp_pass_file_code += u8R"(
	}
	return true;
}

void PASS_CLASS_NAME::AfterNodeXmlLoading() {
	ZoneScoped;
	NeedNewUBOUpload();
}
)";

	h_pass_file_code += u8R"(
#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <LumoBackend/Headers/LumoBackendDefs.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/BaseRenderer.h>
)";

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD)
		{
			h_pass_file_code += u8R"(#include <LumoBackend/Base/QuadShaderPass.h>)";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH ||
			m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_TESSELATION)
		{
			h_pass_file_code += u8R"(#include <LumoBackend/Base/MeshShaderPass.h>)";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			h_pass_file_code += u8R"(#include <LumoBackend/Base/VertexShaderPass.h>)";
		}
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D)
	{
		h_pass_file_code += u8R"(#include <LumoBackend/Base/ShaderPass.h>)";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D)
	{
		h_pass_file_code += u8R"(#include <LumoBackend/Base/ShaderPass.h>)";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		h_pass_file_code += u8R"(#include <LumoBackend/Base/ShaderPass.h>)";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
		h_pass_file_code += u8R"(#include <LumoBackend/Base/RtxShaderPass.h>)";
	}

	h_pass_file_code += u8R"(
#include <Gaia/gaia.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>)";
	h_pass_file_code += GetNodeSlotsInputIncludesInterfaces(vDico);
	h_pass_file_code += GetNodeSlotsOutputIncludesInterfaces(vDico);
	h_pass_file_code += u8R"(

class PASS_CLASS_NAME :)";

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD)
		{
			h_pass_file_code += u8R"(
	public QuadShaderPass,)";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH ||
			m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_TESSELATION)
		{
			h_pass_file_code += ct::toStr(u8R"(
	public MeshShaderPass<VertexStruct::%s>,)",
				m_BaseTypes.m_VertexStructTypes[m_VertexStructTypesIndex].c_str());
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			h_pass_file_code += u8R"(
	public VertexShaderPass,)";
		}
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D)
	{
		h_pass_file_code += u8R"(
	public ShaderPass,)";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D)
	{
		h_pass_file_code += u8R"(
	public ShaderPass,)";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		h_pass_file_code += u8R"(
	public ShaderPass,)";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
		h_pass_file_code += u8R"(
	public RtxShaderPass,)";
	}

	h_pass_file_code += GetPassInputPublicInterfaces(vDico);
	h_pass_file_code += GetNodeSlotsOutputPublicInterfaces(vDico);

	h_pass_file_code +=
        u8R"(
	public NodeInterface
{
public:)";
    if (m_RendererType == RENDERER_TYPE_PIXEL_2D) {
        h_pass_file_code +=
            u8R"(
	static std::shared_ptr<PASS_CLASS_NAME> Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore);)";
    } else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D) {
        h_pass_file_code +=
            u8R"(
	static std::shared_ptr<PASS_CLASS_NAME> Create(const uint32_t& vSize, GaiApi::VulkanCoreWeak vVulkanCore);)";
    } else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D) {
        h_pass_file_code +=
            u8R"(
	static std::shared_ptr<PASS_CLASS_NAME> Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore);)";
    } else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D) {
        h_pass_file_code +=
            u8R"(
	static std::shared_ptr<PASS_CLASS_NAME> Create(const ct::uvec3& vSize, GaiApi::VulkanCoreWeak vVulkanCore);)";
    } else if (m_RendererType == RENDERER_TYPE_RTX) {
        h_pass_file_code +=
            u8R"(
	static std::shared_ptr<PASS_CLASS_NAME> Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore);)";
    }

    h_pass_file_code += u8R"(

private:)";

    h_pass_file_code += GetPassInputPrivateVars(vDico);
    h_pass_file_code += GetPassOutputPrivateVars(vDico);

	h_pass_file_code += m_UBOEditors.Get_Cpp_Header(m_IsAnEffect);

	h_pass_file_code += u8R"(

public:
	PASS_CLASS_NAME(GaiApi::VulkanCoreWeak vVulkanCore);
	~PASS_CLASS_NAME() override;
)";

	if (m_RendererType == RENDERER_TYPE_RTX)
	{
		h_pass_file_code += u8R"(
	void ActionBeforeCompilation() override;)";
	}
	h_pass_file_code += u8R"(
	void ActionBeforeInit() override;
	void WasJustResized() override;
)";
	
	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD)
		{
			// nothing because derived
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH || 
			m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_TESSELATION)
		{
			h_pass_file_code += u8R"(
	void DrawModel(vk::CommandBuffer * vCmdBufferPtr, const int& vIterationNumber) override;)";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			// nothing because derived
		}
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D ||
		m_RendererType == RENDERER_TYPE_COMPUTE_2D ||
		m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		h_pass_file_code += u8R"(
	void Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) override;)";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
		// nothing because derived
	}
	
	h_pass_file_code += u8R"(
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
)";

	h_pass_file_code += GetNodeSlotsInputHFuncs(vDico);
	h_pass_file_code += GetNodeSlotsOutputHFuncs(vDico);

	h_pass_file_code += u8R"(

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
	void AfterNodeXmlLoading() override;

protected:)";

	h_pass_file_code += m_UBOEditors.Get_Cpp_Functions_Header();

	if (m_UseASbo)
	{
		h_pass_file_code += u8R"(
	bool CreateSBO() override;
	void UploadSBO() override;
	void DestroySBO() override;
)";
	}

	if (m_RendererType == RENDERER_TYPE_RTX)
	{
		h_pass_file_code += u8R"(
	bool CanUpdateDescriptors() override;)";
	}
	h_pass_file_code += u8R"(
	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;
)";

	h_pass_file_code += GetPassShaderHeader();

	h_pass_file_code += u8R"(
};)";

    ct::replaceString(cpp_pass_file_code, "PASS_XML_NAME", m_PassXmlName);
    ct::replaceString(h_pass_file_code, "PASS_XML_NAME", m_PassXmlName);

	ct::replaceString(cpp_pass_file_code, "MODULE_XML_NAME", m_ModuleXmlName);
    ct::replaceString(h_pass_file_code, "MODULE_XML_NAME", m_ModuleXmlName);

	ct::replaceString(cpp_pass_file_code, "MODULE_DISPLAY_NAME", m_ModuleDisplayName);
	ct::replaceString(h_pass_file_code, "MODULE_DISPLAY_NAME", m_ModuleDisplayName);

	ct::replaceString(cpp_pass_file_code, "MODULE_CATEGORY_NAME", m_CategoryName);
	ct::replaceString(h_pass_file_code, "MODULE_CATEGORY_NAME", m_CategoryName);

	ct::replaceString(cpp_pass_file_code, "NODE_CLASS_NAME", pass_class_name);
	ct::replaceString(h_pass_file_code, "NODE_CLASS_NAME", pass_class_name);

	ct::replaceString(cpp_pass_file_code, "PASS_CLASS_NAME", pass_class_name);
	ct::replaceString(h_pass_file_code, "PASS_CLASS_NAME", pass_class_name);

	ct::replaceString(cpp_pass_file_code, "MODULE_RENDERER_INIT_FUNC", m_ModuleRendererInitFunc);
	ct::replaceString(h_pass_file_code, "MODULE_RENDERER_INIT_FUNC", m_ModuleRendererInitFunc);

	ct::replaceString(cpp_pass_file_code, "RENDERER_DISPLAY_TYPE", m_ModuleRendererDisplayType);
	ct::replaceString(h_pass_file_code, "RENDERER_DISPLAY_TYPE", m_ModuleRendererDisplayType);

	FileHelper::Instance()->SaveStringToFile(cpp_pass_file_code, cpp_pass_file_name);
	FileHelper::Instance()->SaveStringToFile(h_pass_file_code, h_pass_file_name);
}
