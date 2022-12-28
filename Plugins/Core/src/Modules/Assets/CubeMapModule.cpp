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

#include "CubeMapModule.h"

#include <cinttypes>
#include <filesystem>
#include <functional>
#include <ctools/Logger.h>
#include <Base/FrameBuffer.h>
#include <ctools/FileHelper.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/BaseNode.h>
#include <Systems/CommonSystem.h>
#include <imgui/imgui_internal.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <utils/Mesh/VertexStruct.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanSubmitter.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
using namespace vkApi;

#define MAX_THUMBNAIL_HEIGHT 50

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<CubeMapModule> CubeMapModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<CubeMapModule>(vVulkanCorePtr);
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

CubeMapModule::CubeMapModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: m_VulkanCorePtr(vVulkanCorePtr)
{
	ZoneScoped;

	unique_OpenPictureFileDialog_id = ct::toStr("OpenCubeMapFacesFileDialog%u", (uintptr_t)this);
}

CubeMapModule::~CubeMapModule()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool CubeMapModule::Init()
{
	ZoneScoped;

	return true;
}

void CubeMapModule::Unit()
{
	ZoneScoped;

	ClearTextures();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool CubeMapModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (ImGui::ContrastedButton("Load Textures", nullptr, nullptr, -1.0f))
	{
		ImGuiFileDialog::Instance()->OpenDialog(
			unique_OpenPictureFileDialog_id, "Open CubeMap Face Texture File",
			"CubeMap Faces Textures 2D Files{([a-zA-Z0-9]+_[0-9]+.((png)|(jpg)|(jpeg)))}", // pattern file_09.png (png, jpg, jpeg)
			m_SelectedFilePath, m_SelectedFilePathName,
			1, nullptr, ImGuiFileDialogFlags_Modal);
	}

	if (ImGui::ContrastedButton("Reset Textures", nullptr, nullptr, -1.0f))
	{
		ClearTextures();
	}

	if (m_TextureCubePtr)
	{
		ImGui::Text("File path : %s", m_SelectedFilePath.c_str());

		ImGui::Text("File X+ : %s", m_FileNames[0U].c_str());
		ImGui::Text("File X- : %s", m_FileNames[1U].c_str());
		ImGui::Text("File Y+ : %s", m_FileNames[2U].c_str());
		ImGui::Text("File Y- : %s", m_FileNames[3U].c_str());
		ImGui::Text("File Z+ : %s", m_FileNames[4U].c_str());
		ImGui::Text("File Z- : %s", m_FileNames[5U].c_str());

		const float aw = ImGui::GetContentRegionAvail().x;
		DrawTextures(ct::ivec2((int32_t)aw, (int32_t)(aw * 3.0f / 4.0f)));
	}

	return false;
}

void CubeMapModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

}

void CubeMapModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	ImVec2 max = ImVec2((float)vMaxSize.x, (float)vMaxSize.y);
	ImVec2 min = max * 0.5f;

	if (ImGuiFileDialog::Instance()->Display(unique_OpenPictureFileDialog_id,
		ImGuiWindowFlags_NoCollapse, min, max))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			m_SelectedFilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			m_SelectedFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();

			LoadTextures(m_SelectedFilePathName);
		}

		ImGuiFileDialog::Instance()->Close();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE CUBE SLOT OUTPUT ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* CubeMapModule::GetTextureCube(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;

	if (m_TextureCubePtr)
	{
		return &m_TextureCubePtr->m_DescriptorImageInfo;
	}

	return m_VulkanCorePtr->getEmptyTextureCubeDescriptorImageInfo();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVTE //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void CubeMapModule::LoadTextures(const std::string& vFilePathName)
{
	namespace fs = std::filesystem;

	if (fs::exists(vFilePathName))
	{
		bool is_ok = false;
		m_FilePathNames = {};
		
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			// the last number of first file must be 0
			// the last number of last file must be 5

			if (ps.name.size() > 1)
			{
				std::string name = ps.name.substr(0U, ps.name.size() - 1U);
				uint32_t num = ct::uvariant(ps.name.substr(ps.name.size() - 1U)).GetU();

				if (num >= 0U && num <= 5U)
				{
					is_ok = true;
					for (uint32_t idx = 0U; idx < 6U; ++idx)
					{
						m_FilePathNames[idx] = ct::toStr("%s/%s%u.%s", ps.path.c_str(), name.c_str(), idx, ps.ext.c_str());
						if (!fs::exists(m_FilePathNames[idx]))
						{
							is_ok = false;
							m_FilePathNames = {};
							break;
						}
					}
				}
			}
		}

		if (is_ok)
		{
			m_TextureCubePtr = TextureCube::CreateFromFiles(m_VulkanCorePtr, m_FilePathNames);
			if (m_TextureCubePtr)
			{
				for (uint32_t idx = 0U; idx < 6U; ++idx)
				{
					m_Texture2Ds[idx] = Texture2D::CreateFromFile(
						m_VulkanCorePtr, 
						m_FilePathNames[idx],
						MAX_THUMBNAIL_HEIGHT);
					if (m_Texture2Ds[idx])
					{
						auto ps = FileHelper::Instance()->ParsePathFileName(m_FilePathNames[idx]);
						if (ps.isOk)
						{
							m_FileNames[idx] = ps.name;
						}

						auto imguiRendererPtr = m_VulkanCorePtr->GetVulkanImGuiRenderer().getValidShared();
						if (imguiRendererPtr)
						{
							m_ImGuiTextures[idx].SetDescriptor(
								imguiRendererPtr, 
								&m_Texture2Ds[idx]->m_DescriptorImageInfo, 
								m_Texture2Ds[idx]->m_Ratio);
						}
					}
				}

				auto parentNodePtr = GetParentNode().getValidShared();
				if (parentNodePtr)
				{
					parentNodePtr->SendFrontNotification(TextureUpdateDone);
				}
			}
		}
	}	
}

void CubeMapModule::ClearTextures()
{
	for (auto& tex : m_ImGuiTextures)
	{
		tex.ClearDescriptor();
	}

	m_TextureCubePtr.reset();

	auto parentNodePtr = GetParentNode().getValidShared();
	if (parentNodePtr)
	{
		parentNodePtr->SendFrontNotification(TextureUpdateDone);
	}
}

void CubeMapModule::DrawTextures(const ct::ivec2& vMaxSize, const float& vRounding)
{
	if (m_TextureCubePtr)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;

		static const uint32_t _x_pos = 0U;
		static const uint32_t _x_neg = 1U;
		static const uint32_t _y_pos = 2U;
		static const uint32_t _y_neg = 3U;
		static const uint32_t _z_pos = 4U;
		static const uint32_t _z_neg = 5U;
		static int32_t faces[3U][4U] =
		{
			{ -1,		_y_pos,		-1,			-1},
			{_x_neg,	_z_pos,		_x_pos,		_z_neg},
			{-1,		_y_neg,		-1,			-1}
		};
		static int32_t roundedCorners[3U][4U] = {
			{0,								ImDrawFlags_RoundCornersTop,	0,	0},
			{ImDrawFlags_RoundCornersLeft,	0,								0,	ImDrawFlags_RoundCornersRight},
			{0,								ImDrawFlags_RoundCornersBottom,	0,	0}};

		const ImVec2 size = ImVec2((float)vMaxSize.x / 4.0f, (float)vMaxSize.y / 3.0f);
		const ImVec2 spos = window->DC.CursorPos;
		const ImRect bb(spos, spos + ImVec2((float)vMaxSize.x, (float)vMaxSize.y));
		ImGui::ItemSize(bb);
		if (!ImGui::ItemAdd(bb, 0))
			return;

		const auto borders_color = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		for (size_t row = 0U; row < 3U; ++row)
		{
			for (size_t col = 0U; col < 4U; ++col)
			{
				const auto& idx = faces[row][col];
				if (idx > -1)
				{
					if (m_ImGuiTextures[idx].canDisplayPreview)
					{
						const ImVec2 pos = ImVec2(spos.x + size.x * col, spos.y + size.y * row);
						if (vRounding > 0.0f)
						{
							window->DrawList->AddImageRounded((ImTextureID)&m_ImGuiTextures[idx].descriptor, pos, pos + size,
								ImVec2(0, 0), ImVec2(1, 1), borders_color, vRounding, roundedCorners[row][col]);
						}
						else
						{
							window->DrawList->AddImage((ImTextureID)&m_ImGuiTextures[idx].descriptor, pos, pos + size);
						}
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string CubeMapModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += vOffset + "<cubemap_module>\n";

	str += vOffset + "\t<file_path_name>" + m_SelectedFilePathName + "</file_path_name>\n";
	str += vOffset + "\t<file_path>" + m_SelectedFilePath + "</file_path>\n";

	str += vOffset + "</cubemap_module>\n";

	return str;
}

bool CubeMapModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "cubemap_module")
	{
		if (strName == "file_path")
			m_SelectedFilePath = strValue;
		else if (strName == "file_path_name")
		{
			m_SelectedFilePathName = strValue;
			LoadTextures(m_SelectedFilePathName);
		}
	}

	return true;
}
