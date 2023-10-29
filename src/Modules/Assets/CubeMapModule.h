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


#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>



#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/TextureCube.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/TextureCubeOutputInterface.h>

class CubeMapModule :
	public conf::ConfigAbstract,
	public NodeInterface,
	public TextureCubeOutputInterface,
	public GuiInterface
{
public:
	static std::shared_ptr<CubeMapModule> Create(GaiApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode);

private:
	std::weak_ptr<CubeMapModule> m_This;
	GaiApi::VulkanCorePtr m_VulkanCorePtr = nullptr;
	std::string unique_OpenPictureFileDialog_id;
	TextureCubePtr m_TextureCubePtr = nullptr;
	std::string m_SelectedFilePathName; // to save
	std::string m_SelectedFilePath; // to save
	std::array<Texture2DPtr, 6U> m_Texture2Ds;
	std::array<ImGuiTexture, 6U> m_ImGuiTextures;
	std::array<std::string, 6U> m_FilePathNames;
	std::array<std::string, 6U> m_FileNames;

public:
	CubeMapModule(GaiApi::VulkanCorePtr vVulkanCorePtr);
	~CubeMapModule();

	bool Init();
	void Unit();

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;

	// Interfaces Getters
	vk::DescriptorImageInfo* GetTextureCube(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

	void DrawTextures(const ct::ivec2& vMaxSize, const float& vRounding = 0.0f);

private:
	void LoadTextures(const std::string& vFilePathName);
	void ClearTextures();
};
