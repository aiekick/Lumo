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
//// GENERATOR MODULLE ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorNode::GenerateModuleClasses(const std::string& vPath, const SlotDico& vDico) {
	fs::path module_path = vPath + "/Modules/" + m_CategoryName;
	if (!std::filesystem::exists(module_path))
        fs::create_directory(module_path);

    if (m_IsAnEffect) {
        module_path = vPath + "/Modules/" + m_CategoryName + "/Effects/";
        if (!std::filesystem::exists(module_path))
            fs::create_directory(module_path);
    }

	std::string module_class_name = m_ClassName + "Module";
	std::string cpp_module_file_name = module_path.string() + "/" + module_class_name + ".cpp";
	std::string h_module_file_name = module_path.string() + "/" + module_class_name + ".h";

	std::string pass_renderer_display_type = GetRendererDisplayName();
	std::string pass_class_name = m_ClassName + "Module_" + pass_renderer_display_type + "Pass";
	std::string cpp_pass_file_name = pass_class_name + ".cpp";
	std::string h_pass_file_name = pass_class_name + ".h";

	std::string cpp_module_file_code;
	std::string h_module_file_code;

	cpp_module_file_code += GetLicenceHeader();
	h_module_file_code += GetLicenceHeader();
	cpp_module_file_code += GetPVSStudioHeader();

	// MODULE_XML_NAME							grayscott_module_sim
	// MODULE_DISPLAY_NAME						Gray Scott
	// MODULE_CATEGORY_NAME						Simulation
	// MODULE_CLASS_NAME		
	// PASS_CLASS_NAME
	// MODULE_RENDERER_INIT_FUNC				InitCompute2D
	// RENDERER_DISPLAY_TYPE (ex (Comp)			Comp

	cpp_module_file_code += u8R"(
#include "MODULE_CLASS_NAME.h"

#include <cinttypes>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <ImGuiPack.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Buffer/FrameBuffer.h>
)";
	if (m_GenerateAPass)
	{
        if (m_IsAnEffect) {
            cpp_module_file_code +=
                u8R"(
#include <Modules/MODULE_CATEGORY_NAME/Effects/Pass/PASS_CLASS_NAME.h>
)";
        } else {
            cpp_module_file_code +=
                u8R"(
#include <Modules/MODULE_CATEGORY_NAME/Pass/PASS_CLASS_NAME.h>
)";
		}
	}

	cpp_module_file_code += u8R"(
using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<MODULE_CLASS_NAME> MODULE_CLASS_NAME::Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	
	auto res = std::make_shared<MODULE_CLASS_NAME>(vVulkanCore);
	res->SetParentNode(vParentNode);
	res->m_This = res;
	if (!res->Init()) {
		res.reset();
	}

	return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

MODULE_CLASS_NAME::MODULE_CLASS_NAME(GaiApi::VulkanCoreWeak vVulkanCore))";
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		cpp_module_file_code += u8R"(
	: BaseRenderer(vVulkanCore))";
	}
	if (!m_GenerateAPass)
	{
		if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
		{
			cpp_module_file_code += u8R"(,)";
		}
		cpp_module_file_code += u8R"(
	: m_VulkanCore(vVulkanCore))";
	}
		cpp_module_file_code += u8R"(
{
	ZoneScoped;)";
	cpp_module_file_code += u8R"(
}

MODULE_CLASS_NAME::~MODULE_CLASS_NAME()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool MODULE_CLASS_NAME::Init()
{
	ZoneScoped;
)";
	if (m_GenerateAPass)
	{
		cpp_module_file_code += u8R"(
	m_Loaded = false;
)";
	}

	if (m_GenerateAPass)
	{
		if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
		{
			cpp_module_file_code += u8R"(
	ct::uvec2 map_size = 512;
	if (BaseRenderer::InitPixel(map_size)) {
		//SetExecutionWhenNeededOnly(true);
		m_PASS_CLASS_NAME_Ptr = PASS_CLASS_NAME::Create(map_size, m_VulkanCore);
		if (m_PASS_CLASS_NAME_Ptr) {
			// by default but can be changed via widget
			m_PASS_CLASS_NAME_Ptr->AllowResizeOnResizeEvents(true);
			m_PASS_CLASS_NAME_Ptr->AllowResizeByHandOrByInputs(false);
			AddGenericPass(m_PASS_CLASS_NAME_Ptr);
			m_Loaded = true;
		}
	}
)";
		}
		else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D)
		{
			cpp_module_file_code += u8R"(
	uint32_t map_size = 512;
	if (BaseRenderer::InitCompute1D(map_size)) {
		//SetExecutionWhenNeededOnly(true);
		m_PASS_CLASS_NAME_Ptr = PASS_CLASS_NAME::Create(map_size, m_VulkanCore);
		if (m_PASS_CLASS_NAME_Ptr) {
			// by default but can be changed via widget
			m_PASS_CLASS_NAME_Ptr->AllowResizeOnResizeEvents(true);
			m_PASS_CLASS_NAME_Ptr->AllowResizeByHandOrByInputs(false);
			AddGenericPass(m_PASS_CLASS_NAME_Ptr);
			m_Loaded = true;
		}
	}
)";
		}
		else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D)
		{
			cpp_module_file_code += u8R"(
	ct::uvec2 map_size = 512;
	if (BaseRenderer::InitCompute2D(map_size)) {
		//SetExecutionWhenNeededOnly(true);
		m_PASS_CLASS_NAME_Ptr = PASS_CLASS_NAME::Create(map_size, m_VulkanCore);
		if (m_PASS_CLASS_NAME_Ptr) {
			// by default but can be changed via widget
			m_PASS_CLASS_NAME_Ptr->AllowResizeOnResizeEvents(true);
			m_PASS_CLASS_NAME_Ptr->AllowResizeByHandOrByInputs(false);
			AddGenericPass(m_PASS_CLASS_NAME_Ptr);
			m_Loaded = true;
		}
	}
)";
		}
		else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D)
		{
			cpp_module_file_code += u8R"(
	ct::uvec3 map_size = 512;
	if (BaseRenderer::InitCompute3D(map_size)) {
		//SetExecutionWhenNeededOnly(true);
		m_PASS_CLASS_NAME_Ptr = PASS_CLASS_NAME::Create(map_size, m_VulkanCore);
		if (m_PASS_CLASS_NAME_Ptr) {
			// by default but can be changed via widget
			m_PASS_CLASS_NAME_Ptr->AllowResizeOnResizeEvents(true);
			m_PASS_CLASS_NAME_Ptr->AllowResizeByHandOrByInputs(false);
			AddGenericPass(m_PASS_CLASS_NAME_Ptr);
			m_Loaded = true;
		}
	}
)";
		}
		else if (m_RendererType == RENDERER_TYPE_RTX)
		{
			cpp_module_file_code += u8R"(
	ct::uvec2 map_size = 512;
	if (BaseRenderer::InitRtx(map_size)) {
		//SetExecutionWhenNeededOnly(true);
		m_PASS_CLASS_NAME_Ptr = PASS_CLASS_NAME::Create(map_size, m_VulkanCore);
		if (m_PASS_CLASS_NAME_Ptr) {
			// by default but can be changed via widget
			m_PASS_CLASS_NAME_Ptr->AllowResizeOnResizeEvents(true);
			m_PASS_CLASS_NAME_Ptr->AllowResizeByHandOrByInputs(false);
			AddGenericPass(m_PASS_CLASS_NAME_Ptr);
			m_Loaded = true;
		}
	}
)";
		}
	}

	if (m_GenerateAPass)
	{
		cpp_module_file_code += u8R"(
	return m_Loaded;
}
)";
	}
	else
	{
		cpp_module_file_code += u8R"(
	return true;
}
)";
	}

	if (!m_GenerateAPass)
	{
		cpp_module_file_code += u8R"(
void MODULE_CLASS_NAME::Unit()
{
	ZoneScoped;
}
)";
	}

	if (m_IsATask)
	{
		cpp_module_file_code += u8R"(
//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool MODULE_CLASS_NAME::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;)";
        if (m_RendererType != RENDERER_TYPE_NONE) {
            cpp_module_file_code +=
                u8R"(
		BaseRenderer::Render("MODULE_DISPLAY_NAME", vCmd);)";
        }

        cpp_module_file_code +=
            u8R"(
	return true;
}

bool MODULE_CLASS_NAME::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;)";
        if (m_RendererType != RENDERER_TYPE_NONE) {
            cpp_module_file_code +=
                u8R"(
	BaseRenderer::Render("MODULE_DISPLAY_NAME", vCmd);)";
        }

        cpp_module_file_code +=
            u8R"(
	return true;
}
)";
		
	}

	cpp_module_file_code += u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool MODULE_CLASS_NAME::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
)";
	if (m_IsATask)
	{
		cpp_module_file_code += u8R"(
	if (m_LastExecutedFrame == vCurrentFrame)
	{)";
	}
	
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		cpp_module_file_code += u8R"(
		if (m_PASS_CLASS_NAME_Ptr) {
			return m_PASS_CLASS_NAME_Ptr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
		}
		)";
	}

	if (m_IsATask)
	{
		cpp_module_file_code += u8R"(
	}
)";
	}
	cpp_module_file_code += u8R"(
	return false;
}

bool MODULE_CLASS_NAME::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);)";
	if (m_IsATask)
	{
        cpp_module_file_code +=
            u8R"(
	if (m_LastExecutedFrame == vCurrentFrame)
	{)";
    }

    if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE) {
        cpp_module_file_code +=
            u8R"(
		if (m_PASS_CLASS_NAME_Ptr) {
			return m_PASS_CLASS_NAME_Ptr->DrawOverlays(vCurrentFrame, vRect, vContextPtr, vUserDatas);
		})";
    }

    if (m_IsATask) {
        cpp_module_file_code +=
            u8R"(
	}
)";
    }
	cpp_module_file_code += u8R"(
	return false;
}

bool MODULE_CLASS_NAME::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);)";
        if (m_IsATask) {
            cpp_module_file_code +=
                u8R"(
	if (m_LastExecutedFrame == vCurrentFrame)
	{)";
        }

        if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE) {
        cpp_module_file_code +=
            u8R"(
		if (m_PASS_CLASS_NAME_Ptr) {
			return m_PASS_CLASS_NAME_Ptr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
		})";
        }

        if (m_IsATask) {
        cpp_module_file_code +=
            u8R"(
	}
)";
        }
        cpp_module_file_code +=
            u8R"(
	return false;
}
)";

	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{

		cpp_module_file_code += u8R"(
void MODULE_CLASS_NAME::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	// do some code
	
	BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}
)";
	}

	cpp_module_file_code += GetModuleInputCppFuncs(vDico);
	cpp_module_file_code += GetModuleOutputCppFuncs(vDico);
	cpp_module_file_code += u8R"(
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MODULE_CLASS_NAME::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;
	std::string str;
	str += vOffset + "<MODULE_XML_NAME>\n";)";
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		cpp_module_file_code += u8R"(
	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";)";
	}

	if (m_GenerateAPass)
	{
		cpp_module_file_code += u8R"(
	if (m_PASS_CLASS_NAME_Ptr) {
		str += m_PASS_CLASS_NAME_Ptr->getXml(vOffset + "\t", vUserDatas);
	})";
	}

	cpp_module_file_code += u8R"(
	str += vOffset + "</MODULE_XML_NAME>\n";
	return str;
}

bool MODULE_CLASS_NAME::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	ZoneScoped;

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "MODULE_XML_NAME")	{)";
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		cpp_module_file_code += u8R"(
		if (strName == "can_we_render") {
			m_CanWeRender = ct::ivariant(strValue).GetB();
		} )";
    }

    cpp_module_file_code +=
        u8R"(
	})";

	if (m_GenerateAPass)
	{
		cpp_module_file_code += u8R"(
		if (m_PASS_CLASS_NAME_Ptr) {
			m_PASS_CLASS_NAME_Ptr->setFromXml(vElem, vParent, vUserDatas);
		})";
	}

	cpp_module_file_code += u8R"(

	return true;
}

void MODULE_CLASS_NAME::AfterNodeXmlLoading()
{
	ZoneScoped;)";
	if (m_GenerateAPass)
	{
		cpp_module_file_code += u8R"(
	if (m_PASS_CLASS_NAME_Ptr) {
		m_PASS_CLASS_NAME_Ptr->AfterNodeXmlLoading();
	})";
	}

	cpp_module_file_code += u8R"(
}
)";

	/////////////////////////////////////////////////////////////////
	////// HEADER ///////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
	h_module_file_code += u8R"(

#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <LumoBackend/Headers/LumoBackendDefs.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/BaseRenderer.h>
#include <LumoBackend/Base/QuadShaderPass.h>

#include <Gaia/gaia.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>
)";

	h_module_file_code += u8R"(
#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/TaskInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/ResizerInterface.h>
)";
	h_module_file_code += GetNodeModuleInputIncludesInterfaces(vDico);
    h_module_file_code += GetNodeModuleOutputIncludesInterfaces(vDico);
	h_module_file_code += u8R"(
)";
	if (m_GenerateAPass)
	{
		h_module_file_code += u8R"(
class PASS_CLASS_NAME;)";
	}

	h_module_file_code += u8R"(
class MODULE_CLASS_NAME :)";
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		h_module_file_code += u8R"(
	public BaseRenderer,)";
	}
	else
	{
		h_module_file_code += u8R"(
	public conf::ConfigAbstract,)";
	}

	if (m_IsATask)
	{
		h_module_file_code += u8R"(
	public TaskInterface,)";
	}

	h_module_file_code += GetNodeModuleInputPublicInterfaces(vDico);
    h_module_file_code += GetNodeModuleOutputPublicInterfaces(vDico);

	h_module_file_code += u8R"(
	public NodeInterface
{
public:
	static std::shared_ptr<MODULE_CLASS_NAME> Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode);

private:
	std::weak_ptr<MODULE_CLASS_NAME> m_This;
)";
    h_module_file_code += GetModuleInputPrivateVars(vDico);
    h_module_file_code += GetModuleOutputPrivateVars(vDico);
	if (!m_GenerateAPass)
	{
		h_module_file_code += u8R"(
	GaiApi::VulkanCoreWeak m_VulkanCore;
)";
	}

	if (m_GenerateAPass)
	{
		h_module_file_code += u8R"(
	std::shared_ptr<PASS_CLASS_NAME> m_PASS_CLASS_NAME_Ptr = nullptr;
)";
	}

	h_module_file_code += u8R"(
public:
	MODULE_CLASS_NAME(GaiApi::VulkanCoreWeak vVulkanCore);
	~MODULE_CLASS_NAME())";
	if (m_GenerateAPass)
	{
		h_module_file_code += u8R"( override;
)";
	}
	else
	{
		h_module_file_code += u8R"(;
)";
	}
	h_module_file_code += u8R"(
	bool Init();)";

	if (!m_GenerateAPass)
	{
		h_module_file_code += u8R"(
	void Unit();)";
	}
	h_module_file_code += u8R"(
)";

	if (m_IsATask)
	{
		h_module_file_code += u8R"(
	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	bool ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
)";
	}
		h_module_file_code += u8R"(
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
)";
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		h_module_file_code += u8R"(
	void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
)";
	}

	h_module_file_code += GetNodeInputHFuncs(vDico);
	h_module_file_code += GetNodeOutputHFuncs(vDico);
	h_module_file_code += u8R"(
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
	void AfterNodeXmlLoading() override;
};
)";
    
    ct::replaceString(cpp_module_file_code, "PASS_XML_NAME", m_PassXmlName);
    ct::replaceString(h_module_file_code, "PASS_XML_NAME", m_PassXmlName);

	ct::replaceString(cpp_module_file_code, "MODULE_XML_NAME", m_ModuleXmlName);
	ct::replaceString(h_module_file_code, "MODULE_XML_NAME", m_ModuleXmlName);

	ct::replaceString(cpp_module_file_code, "MODULE_DISPLAY_NAME", m_ModuleDisplayName);
	ct::replaceString(h_module_file_code, "MODULE_DISPLAY_NAME", m_ModuleDisplayName);

	ct::replaceString(cpp_module_file_code, "MODULE_CATEGORY_NAME", m_CategoryName);
	ct::replaceString(h_module_file_code, "MODULE_CATEGORY_NAME", m_CategoryName);

	ct::replaceString(cpp_module_file_code, "MODULE_CLASS_NAME", module_class_name);
	ct::replaceString(h_module_file_code, "MODULE_CLASS_NAME", module_class_name);

	ct::replaceString(cpp_module_file_code, "PASS_CLASS_NAME", pass_class_name);
	ct::replaceString(h_module_file_code, "PASS_CLASS_NAME", pass_class_name);

	ct::replaceString(cpp_module_file_code, "MODULE_RENDERER_INIT_FUNC", m_ModuleRendererInitFunc);
	ct::replaceString(h_module_file_code, "MODULE_RENDERER_INIT_FUNC", m_ModuleRendererInitFunc);

	ct::replaceString(cpp_module_file_code, "RENDERER_DISPLAY_TYPE", m_ModuleRendererDisplayType);
	ct::replaceString(h_module_file_code, "RENDERER_DISPLAY_TYPE", m_ModuleRendererDisplayType);

	FileHelper::Instance()->SaveStringToFile(cpp_module_file_code, cpp_module_file_name);
	FileHelper::Instance()->SaveStringToFile(h_module_file_code, h_module_file_name);

	if (m_GenerateAPass)
	{
		GeneratePasseClasses(vPath, vDico);
	}
}
