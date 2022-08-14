/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

#include <Graph/Graph.h>
#include <Graph/Base/BaseNode.h>
#include <Interfaces/TextureCubeOutputInterface.h>
#include <Interfaces/ShaderUpdateInterface.h>

class CubeMapModule;
class CubeMapNode :
	public BaseNode,
	public TextureCubeOutputInterface,
	public ShaderUpdateInterface
{
public:
	static std::shared_ptr<CubeMapNode> Create(vkApi::VulkanCorePtr vVulkanCorePtr);

private:
	std::shared_ptr<CubeMapModule> m_CubeMapModulePtr = nullptr;

public:
	CubeMapNode();
	~CubeMapNode() override;

	// Init / Unit
	bool Init(vkApi::VulkanCorePtr vVulkanCorePtr) override;

	// Draw Widgets
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) override;

	// Interfaces Getters
	vk::DescriptorImageInfo* GetTextureCube(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;


	// Input / Ouput slot widgets
	void DrawOutputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot) override;

	// Configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
	
	// Shader Update
	void UpdateShaders(const std::set<std::string>& vFiles) override;
};
