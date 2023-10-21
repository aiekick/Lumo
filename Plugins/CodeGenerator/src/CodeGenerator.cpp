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
#include <ImGuiPack.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <LumoBackend/Graph/Base/NodeSlot.h>

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

bool RTX::AuthorizeLoading()
{
	auto vkCorePtr = m_VulkanCoreWeak.lock();
	return (vkCorePtr && vkCorePtr->GetSupportedFeatures().is_RTX_Supported); // autohrisation to load if RTX Feature available
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
	auto vkCorePtr = m_VulkanCoreWeak.lock();
	if (!vkCorePtr->GetSupportedFeatures().is_RTX_Supported)
	{
		return "Err : the RTX Features are not availables";
	}
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
	auto vkCorePtr = m_VulkanCoreWeak.lock();

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

int RTX::ResetImGuiID(const int& vWidgetId) {
    auto ids = ImGui::GetPUSHID();
    ImGui::SetPUSHID(vWidgetId);
    return ids;
}
