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

#include "Texture2DModule.h"
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <vkFramework/VulkanCore.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<Texture2DModule> Texture2DModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<Texture2DModule>(vVulkanCorePtr);
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

Texture2DModule::Texture2DModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: m_VulkanCorePtr(vVulkanCorePtr)
{
	unique_OpenPictureFileDialog_id = ct::toStr("OpenPictureFileDialog%u", (uintptr_t)this);
}

Texture2DModule::~Texture2DModule()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// PUBLIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool Texture2DModule::Init()
{
	ZoneScoped;

	return true;
}

void Texture2DModule::Unit()
{
	ZoneScoped;

	m_ImGuiTexture.ClearDescriptor();
	m_Texture2DPtr.reset();
}

void Texture2DModule::NeedResize(ct::ivec2* vNewSize)
{
	ZoneScoped;
}

bool Texture2DModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	ZoneScoped;

	if (ImGui::ContrastedButton("Load Texture 2D", nullptr, nullptr, -1.0f))
	{
		ImGuiFileDialog::Instance()->OpenDialog(
			unique_OpenPictureFileDialog_id, "Open Texture 2D File", "Texture 2D files{.png,.jpg,.jpeg}", m_FilePath, m_FilePathName,
			1, nullptr, ImGuiFileDialogFlags_Modal);
	}

	if (ImGui::ContrastedButton("Reset Texture 2D", nullptr, nullptr, -1.0f))
	{
		m_ImGuiTexture.ClearDescriptor();
	}

	if (!m_FileName.empty())
	{
		ImGui::Text("File name : %s", m_FileName.c_str());
		ImGui::TextWrapped("File path name: %s", m_FilePathName.c_str());

		DrawTexture((int)ImGui::GetContentRegionAvail().x);
	}

	return false;
}

void Texture2DModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	ZoneScoped;
}

void Texture2DModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	ImVec2 max = ImVec2((float)vMaxSize.x, (float)vMaxSize.y);
	ImVec2 min = max * 0.5f;

	if (ImGuiFileDialog::Instance()->Display(unique_OpenPictureFileDialog_id,
		ImGuiWindowFlags_NoCollapse, min, max))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			m_FilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			m_FilePath = ImGuiFileDialog::Instance()->GetCurrentPath();

			LoadTexture2D(m_FilePathName);
		}

		ImGuiFileDialog::Instance()->Close();
	}
}

void Texture2DModule::DrawTexture(ct::ivec2 vMaxSize)
{
	if (m_ImGuiTexture.canDisplayPreview)
	{
		auto rect = ct::GetScreenRectWithRatio<int32_t>(m_ImGuiTexture.ratio, vMaxSize, false);

		const ImVec2 pos = ImVec2((float)rect.x, (float)rect.y);
		const ImVec2 siz = ImVec2((float)rect.w, (float)rect.h);
		ImGui::ImageRect((ImTextureID)&m_ImGuiTexture.descriptor, pos, siz);
	}
}

vk::DescriptorImageInfo* Texture2DModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	ZoneScoped;

	if (m_Texture2DPtr)
	{
		if (vOutSize)
		{
			vOutSize->x = (float)m_Texture2DPtr->m_Width;
			vOutSize->y = (float)m_Texture2DPtr->m_Height;
		}
		return &m_Texture2DPtr->m_DescriptorImageInfo;
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Texture2DModule::LoadTexture2D(const std::string& vFilePathName)
{
	m_Texture2DPtr = Texture2D::CreateFromFile(m_VulkanCorePtr, vFilePathName);
	if (m_Texture2DPtr)
	{
		auto ps = FileHelper::Instance()->ParsePathFileName(m_FilePathName);
		if (ps.isOk)
		{
			m_FileName = ps.name;
		}

		auto imguiRendererPtr = m_VulkanCorePtr->GetVulkanImGuiRenderer().getValidShared();
		if (imguiRendererPtr)
		{
			m_ImGuiTexture.SetDescriptor(imguiRendererPtr, &m_Texture2DPtr->m_DescriptorImageInfo, m_Texture2DPtr->m_Ratio);
		}

		auto parentNodePtr = GetParentNode().getValidShared();
		if (parentNodePtr)
		{
			parentNodePtr->SendFrontNotification(TextureUpdateDone);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string Texture2DModule::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<texture_2d_module>\n";

	str += vOffset + "\t<file_path_name>" + m_FilePathName + "</file_path_name>\n";
	str += vOffset + "\t<file_path>" + m_FilePath + "</file_path>\n";

	str += vOffset + "</texture_2d_module>\n";

	return str;
}

bool Texture2DModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "texture_2d_module")
	{
		if (strName == "file_path_name")
		{
			m_FilePathName = strValue;
			LoadTexture2D(m_FilePathName);
		}
		if (strName == "file_path")
			m_FilePath = strValue;
	}

	return true;
}
