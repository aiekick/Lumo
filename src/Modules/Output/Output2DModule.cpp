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

#include "Output2DModule.h"
#include <Panes/View2DPane.h>
#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/BaseNode.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <Systems/CommonSystem.h>

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

Output2DModulePtr Output2DModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	auto res = std::make_shared<Output2DModule>(vVulkanCorePtr);
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

Output2DModule::Output2DModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: m_VulkanCorePtr(vVulkanCorePtr)
{
	ZoneScoped;	
}

Output2DModule::~Output2DModule()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool Output2DModule::Init()
{
	ZoneScoped;

	View2DPane::Instance()->SetOrUpdateOutput(m_This);
	
	return true;
}

void Output2DModule::Unit()
{
	ZoneScoped;

	View2DPane::Instance()->SetOrUpdateOutput(Output2DModuleWeak());
	m_ImGuiTexture.ClearDescriptor();
}

bool Output2DModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	ZoneScoped;

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		CommonSystem::Instance()->DrawImGui();

		ImGui::Header("Output 2D");

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

void Output2DModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& /*vRect*/, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void Output2DModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& /*vMaxSize*/, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

bool Output2DModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* /*vCmd*/, BaseNodeState* /*vBaseNodeState*/)
{
	ZoneScoped;

	if (m_LastExecutedFrame != vCurrentFrame)
	{
		m_LastExecutedFrame = vCurrentFrame;
	}

	return true;
}

ct::fvec2 Output2DModule::GetOutputSize() const
{
	return 0.0f;
}

void Output2DModule::SetTexture(const uint32_t& /*vBinding*/, vk::DescriptorImageInfo* /*vImageInfo*/)
{
	
}

vk::DescriptorImageInfo* Output2DModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	auto parentNodePtr = dynamic_pointer_cast<TextureOutputInterface>(GetParentNode().getValidShared());
	if (parentNodePtr)
	{
		auto desc = parentNodePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
		auto imguiRendererPtr = m_VulkanCorePtr->GetVulkanImGuiRenderer().getValidShared();
		if (imguiRendererPtr)
		{
			m_ImGuiTexture.SetDescriptor(imguiRendererPtr, desc);
			m_ImGuiTexture.ratio = 1.0f;
			if (vOutSize && !vOutSize->emptyAND())
			{
				m_ImGuiTexture.ratio = vOutSize->ratioXY<float>();
			}
			
		}
		return desc;
	}
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string Output2DModule::getXml(const std::string& /*vOffset*/, const std::string& /*vUserDatas*/)
{
	std::string str;

	return str;
}

bool Output2DModule::setFromXml(tinyxml2::XMLElement* /*vElem*/, tinyxml2::XMLElement* /*vParent*/, const std::string& /*vUserDatas*/)
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