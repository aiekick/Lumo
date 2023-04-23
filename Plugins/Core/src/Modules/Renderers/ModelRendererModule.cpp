/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ModelRendererModule.h"

#include <cinttypes>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <Graph/Base/BaseNode.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanSubmitter.h>
#include <utils/Mesh/VertexStruct.h>
#include <Base/FrameBuffer.h>

#include <Modules/Renderers/Pass/ModelRendererModule_Mesh_Pass.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<ModelRendererModule> ModelRendererModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<ModelRendererModule>(vVulkanCorePtr);
	res->SetParentNode(vParentNode);
	res->m_This = res;
	if (!res->Init())
	{
		res.reset();
	}

	return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

ModelRendererModule::ModelRendererModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: TaskRenderer(vVulkanCorePtr)
{
	ZoneScoped;

	m_SceneShaderPassPtr = SceneShaderPass::Create();
}

ModelRendererModule::~ModelRendererModule()
{
	ZoneScoped;

	Unit();

	m_SceneShaderPassPtr.reset();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ModelRendererModule::Init()
{
	ZoneScoped;

	m_Loaded = false;

	ct::uvec2 map_size = 512;

	if (TaskRenderer::InitPixel(map_size))
	{
		//SetExecutionWhenNeededOnly(true);

		m_ModelRendererModule_Mesh_Pass_Ptr = std::make_shared<ModelRendererModule_Mesh_Pass>(m_VulkanCorePtr);
		if (m_ModelRendererModule_Mesh_Pass_Ptr)
		{
			// by default but can be changed via widget
			//m_ModelRendererModule_Mesh_Pass_Ptr->AllowResizeOnResizeEvents(false);
			//m_ModelRendererModule_Mesh_Pass_Ptr->AllowResizeByHandOrByInputs(true);

			if (m_ModelRendererModule_Mesh_Pass_Ptr->InitPixel(map_size, 1U, true, true, 0.0f,
				false, false, vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e2))
			{
				AddGenericPass(m_ModelRendererModule_Mesh_Pass_Ptr);
				m_SceneShaderPassPtr->Add(m_ModelRendererModule_Mesh_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ModelRendererModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	TaskRenderer::Render("Model Renderer", vCmd);

	return true;
}

bool ModelRendererModule::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	TaskRenderer::Render("Model Renderer", vCmd);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool ModelRendererModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (IsTheGoodFrame(vCurrentFrame))
	{
		if (ImGui::CollapsingHeader_CheckBox("Model Renderer##ModelRendererModule", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			if (m_ModelRendererModule_Mesh_Pass_Ptr)
			{
				change |= m_ModelRendererModule_Mesh_Pass_Ptr->DrawWidgets(vCurrentFrame, vContext);
			}

			return change;
		}
	}

	return false;
}

void ModelRendererModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void ModelRendererModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void ModelRendererModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	// do some code
	
	TaskRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL INPUT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void ModelRendererModule::SetModel(SceneModelWeak vSceneModel)
{	
	ZoneScoped;

	if (m_ModelRendererModule_Mesh_Pass_Ptr)
	{
		m_ModelRendererModule_Mesh_Pass_Ptr->SetModel(vSceneModel);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* ModelRendererModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;

	if (m_ModelRendererModule_Mesh_Pass_Ptr)
	{
		return m_ModelRendererModule_Mesh_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SHADER PASS SLOT OUTPUT /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneShaderPassWeak ModelRendererModule::GetShaderPasses(const uint32_t& vBindingPoint)
{
	return m_SceneShaderPassPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ModelRendererModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += vOffset + "<model_renderer_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	if (m_ModelRendererModule_Mesh_Pass_Ptr)
	{
		str += m_ModelRendererModule_Mesh_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</model_renderer_module>\n";

	return str;
}

bool ModelRendererModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "model_renderer_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();

		if (m_ModelRendererModule_Mesh_Pass_Ptr)
		{
			m_ModelRendererModule_Mesh_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
		}
	}

	return true;
}

void ModelRendererModule::AfterNodeXmlLoading()
{
	ZoneScoped;

	if (m_ModelRendererModule_Mesh_Pass_Ptr)
	{
		m_ModelRendererModule_Mesh_Pass_Ptr->AfterNodeXmlLoading();
	}
}
