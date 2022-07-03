/*
Copyright 2022-2022 Stephane Cuillerdier (aka aiekick)

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

#include "OutputModule.h"
#include <Panes/View3DPane.h>
#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/BaseNode.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <Systems/CommonSystem.h>

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

OutputModulePtr OutputModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	auto res = std::make_shared<OutputModule>(vVulkanCorePtr);
	res->m_This = res;
	res->SetParentNode(vParentNode);
	if (!res->Init())
	{
		res.reset();
	}
	return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

OutputModule::OutputModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: m_VulkanCorePtr(vVulkanCorePtr)
{
	ZoneScoped;	
}

OutputModule::~OutputModule()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool OutputModule::Init()
{
	ZoneScoped;

	View3DPane::Instance()->SetOrUpdateOutput(m_This);
	
	return true;
}

void OutputModule::Unit()
{
	ZoneScoped;

	View3DPane::Instance()->SetOrUpdateOutput(OutputModuleWeak());
	m_ImGuiTexture.ClearDescriptor();
}

bool OutputModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		CommonSystem::Instance()->DrawImGui();

		ImGui::Header("Output");

		if (m_ImGuiTexture.canDisplayPreview)
		{
			auto rect = ct::GetScreenRectWithRatio<int32_t>(m_ImGuiTexture.ratio, (int)ImGui::GetContentRegionAvail().x, false);
			const ImVec2 pos = ImVec2((float)rect.x, (float)rect.y);
			const ImVec2 siz = ImVec2((float)rect.w, (float)rect.h);
			ImGui::ImageRect((ImTextureID)&m_ImGuiTexture.descriptor, pos, siz);
		}
	}

	return false;
}

void OutputModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& /*vRect*/, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void OutputModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& /*vMaxSize*/, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void OutputModule::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	if (vNewSize)
	{
		m_fOutputSize = ct::fvec2((float)vNewSize->x, (float)vNewSize->y);
	}

	auto parentNodePtr = dynamic_pointer_cast<ResizerInterface>(GetParentNode().getValidShared());
	if (parentNodePtr)
	{
		parentNodePtr->NeedResize(vNewSize, vCountColorBuffer);
	}
}

bool OutputModule::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer* /*vCmd*/)
{
	ZoneScoped;

	if (m_LastExecutedFrame != vCurrentFrame)
	{
		m_LastExecutedFrame = vCurrentFrame;
	}

	return true;
}

void OutputModule::SetTexture(const uint32_t& /*vBinding*/, vk::DescriptorImageInfo* /*vImageInfo*/)
{
	
}

vk::DescriptorImageInfo* OutputModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	auto parentNodePtr = dynamic_pointer_cast<TextureOutputInterface>(GetParentNode().getValidShared());
	if (parentNodePtr)
	{
		auto desc = parentNodePtr->GetDescriptorImageInfo(vBindingPoint);
		auto imguiRendererPtr = m_VulkanCorePtr->GetVulkanImGuiRenderer().getValidShared();
		if (imguiRendererPtr)
		{
			m_ImGuiTexture.SetDescriptor(imguiRendererPtr, desc);
			m_ImGuiTexture.ratio = m_fOutputSize.ratioXY<float>();
		}
		return desc;
	}
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string OutputModule::getXml(const std::string& /*vOffset*/, const std::string& /*vUserDatas*/)
{
	std::string str;

	return str;
}

bool OutputModule::setFromXml(tinyxml2::XMLElement* /*vElem*/, tinyxml2::XMLElement* /*vParent*/, const std::string& /*vUserDatas*/)
{
	// The value of this child identifies the name of this element
	/*std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();*/

	return true;
}