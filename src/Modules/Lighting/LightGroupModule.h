/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <array>
#include <string>
#include <vector>
#include <memory>
#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <glm/glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ctools/ConfigAbstract.h>
#include <Interfaces/GuiInterface.h>
#include <Interfaces/NodeInterface.h>
#include <Interfaces/GizmoInterface.h>
#include <Interfaces/LightOutputInterface.h>
#include <Interfaces/BufferObjectInterface.h>

class LightGroupModule;
typedef std::shared_ptr<LightGroupModule> LightGroupModulePtr;
typedef ct::cWeak<LightGroupModule> LightGroupModuleWeak;

namespace vkApi { class VulkanCore; }
class LightGroupModule :
	public conf::ConfigAbstract,
	public NodeInterface,
	public GuiInterface,
	public LightOutputInterface,
	public BufferObjectInterface
{
public:
	static LightGroupModulePtr Create(vkApi::VulkanCore* vVulkanCore, BaseNodeWeak vParentNode);
	static std::string GetBufferObjectStructureHeader(const uint32_t& vBinding);

private:
	LightGroupModuleWeak m_This;
	SceneLightGroupPtr m_SceneLightGroupPtr = nullptr;
	vkApi::VulkanCore* m_VulkanCore = nullptr;

private: // imgui
	ct::fvec4 m_DefaultLightColor = 1.0f;

public:
	LightGroupModule(vkApi::VulkanCore* vVulkanCore);
	~LightGroupModule();

	bool Init();
	void Unit();

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;

	SceneLightGroupWeak GetLightGroup();

	///////////////////////////////////////////////////////
	//// BUFFER OBJECTS ///////////////////////////////////
	///////////////////////////////////////////////////////

	void UploadBufferObjectIfDirty(vkApi::VulkanCore* vVulkanCore) override;
	bool CreateBufferObject(vkApi::VulkanCore* vVulkanCore) override;
	void DestroyBufferObject() override;
	vk::DescriptorBufferInfo* GetBufferInfo() override;

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};