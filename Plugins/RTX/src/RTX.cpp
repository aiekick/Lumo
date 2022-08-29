// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
#include "RTX.h"
#include <Headers/RTXBuild.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/BaseNode.h>
#include <Systems/CommonSystem.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanWindow.h>
#include <Graph/Base/NodeSlot.h>

#include <Nodes/Renderers/RtxPbrRendererNode.h>
#include <Nodes/Lighting/RtxModelShadowNode.h>
#include <Nodes/Builders/ModelToAccelStructNode.h>

#ifndef USE_PLUGIN_STATIC_LINKING
// needed for plugin creating / destroying
extern "C" // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec (dllexport)
#else
#define PLUGIN_PREFIX
#endif

	PLUGIN_PREFIX RTX* allocator()
	{
		return new RTX();
	}

	PLUGIN_PREFIX void deleter(RTX* ptr)
	{
		delete ptr;
	}
}
#endif // USE_PLUGIN_STATIC_LINKING

RTX::RTX()
{
#ifdef _MSC_VER
	// active memory leak detector
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void RTX::ActionAfterInit()
{
	NodeSlot::sGetSlotColors()->AddSlotColor("RTX_ACCEL_STRUCTURE", ImVec4(0.8f, 0.5f, 0.8f, 1.0f));
}

uint32_t RTX::GetVersionMajor() const
{
	return RTX_MinorNumber;
}

uint32_t RTX::GetVersionMinor() const
{
	return RTX_MajorNumber;
}

uint32_t RTX::GetVersionBuild() const
{
	return RTX_BuildNumber;
}

std::string RTX::GetName() const
{
	return "RTX";
}

std::string RTX::GetVersion() const
{
	return RTX_BuildId;
}

std::string RTX::GetDescription() const
{
	return "Ray Tracing (RTX) plugin";
}

std::vector<std::string> RTX::GetNodes() const
{
	return
	{
		"RTX_PBR_RENDERER",
		"RTX_MODEL_SHADOW",
		"RTX_MODEL_TO_ACCELERATION_STRUCTURE"
	};
}

std::vector<LibraryEntry> RTX::GetLibrary() const
{
	std::vector<LibraryEntry> res;

	res.push_back(AddLibraryEntry("RTX/3D", "AccelStruct Builder", "RTX_MODEL_TO_ACCELERATION_STRUCTURE"));
	res.push_back(AddLibraryEntry("RTX/3D", "Model Shadw", "RTX_MODEL_SHADOW"));
	res.push_back(AddLibraryEntry("RTX/3D", "PBR Renderer", "RTX_PBR_RENDERER"));

	return res;
}

BaseNodePtr RTX::CreatePluginNode(const std::string& vPluginNodeName)
{
	auto vkCorePtr = m_VulkanCoreWeak.getValidShared();

	if (vPluginNodeName == "RTX_MODEL_SHADOW")
		return RtxModelShadowNode::Create(vkCorePtr);
	else if (vPluginNodeName == "RTX_MODEL_TO_ACCELERATION_STRUCTURE")
		return ModelToAccelStructNode::Create(vkCorePtr);
	else if (vPluginNodeName == "RTX_PBR_RENDERER")
		return RtxPbrRendererNode::Create(vkCorePtr);

	return nullptr;
}

std::vector<PluginPane> RTX::GetPanes() const
{
	std::vector<PluginPane> res;

	return res;
}

int RTX::ResetImGuiID(const int& vWidgetId)
{
	auto ids = ImGui::CustomStyle::Instance()->pushId;
	ImGui::CustomStyle::Instance()->pushId = vWidgetId;
	return ids;
}
