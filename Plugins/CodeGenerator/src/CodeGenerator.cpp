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
#include "CodeGenerator.h"
#include <Headers/CodeGeneratorBuild.h>
#include <ctools/FileHelper.h>
#include <ImGuiPack.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <LumoBackend/Graph/Base/NodeSlot.h>

#include <Nodes/Renderers/CodeGeneratorPbrRendererNode.h>
#include <Nodes/Lighting/CodeGeneratorModelShadowNode.h>
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

	PLUGIN_PREFIX CodeGenerator* allocator()
	{
		return new CodeGenerator();
	}

	PLUGIN_PREFIX void deleter(CodeGenerator* ptr)
	{
		delete ptr;
	}
}
#endif // USE_PLUGIN_STATIC_LINKING

CodeGenerator::CodeGenerator()
{
#ifdef _MSC_VER
	// active memory leak detector
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

bool CodeGenerator::AuthorizeLoading()
{
	auto vkCorePtr = m_VulkanCoreWeak.lock();
	return (vkCorePtr && vkCorePtr->GetSupportedFeatures().is_CodeGenerator_Supported); // autohrisation to load if CodeGenerator Feature available
}

void CodeGenerator::ActionAfterInit()
{
	NodeSlot::sGetSlotColors()->AddSlotColor("CodeGenerator_ACCEL_STRUCTURE", ImVec4(0.8f, 0.5f, 0.8f, 1.0f));
}

uint32_t CodeGenerator::GetVersionMajor() const
{
	return CodeGenerator_MinorNumber;
}

uint32_t CodeGenerator::GetVersionMinor() const
{
	return CodeGenerator_MajorNumber;
}

uint32_t CodeGenerator::GetVersionBuild() const
{
	return CodeGenerator_BuildNumber;
}

std::string CodeGenerator::GetName() const
{
	return "CodeGenerator";
}

std::string CodeGenerator::GetVersion() const
{
	return CodeGenerator_BuildId;
}

std::string CodeGenerator::GetDescription() const
{
	auto vkCorePtr = m_VulkanCoreWeak.lock();
	if (!vkCorePtr->GetSupportedFeatures().is_CodeGenerator_Supported)
	{
		return "Err : the CodeGenerator Features are not availables";
	}
	return "Ray Tracing (CodeGenerator) plugin";

}

std::vector<std::string> CodeGenerator::GetNodes() const
{
	return
	{
		"CodeGenerator_PBR_RENDERER",
		"CodeGenerator_MODEL_SHADOW",
		"CodeGenerator_MODEL_TO_ACCELERATION_STRUCTURE"
	};
}

std::vector<LibraryEntry> CodeGenerator::GetLibrary() const
{
	std::vector<LibraryEntry> res;

	res.push_back(AddLibraryEntry("CodeGenerator/3D", "AccelStruct Builder", "CodeGenerator_MODEL_TO_ACCELERATION_STRUCTURE"));
	res.push_back(AddLibraryEntry("CodeGenerator/3D", "Model Shadw", "CodeGenerator_MODEL_SHADOW"));
	res.push_back(AddLibraryEntry("CodeGenerator/3D", "PBR Renderer", "CodeGenerator_PBR_RENDERER"));

	return res;
}

BaseNodePtr CodeGenerator::CreatePluginNode(const std::string& vPluginNodeName)
{
	auto vkCorePtr = m_VulkanCoreWeak.lock();

	if (vPluginNodeName == "CodeGenerator_MODEL_SHADOW")
		return CodeGeneratorModelShadowNode::Create(vkCorePtr);
	else if (vPluginNodeName == "CodeGenerator_MODEL_TO_ACCELERATION_STRUCTURE")
		return ModelToAccelStructNode::Create(vkCorePtr);
	else if (vPluginNodeName == "CodeGenerator_PBR_RENDERER")
		return CodeGeneratorPbrRendererNode::Create(vkCorePtr);

	return nullptr;
}

std::vector<PluginPane> CodeGenerator::GetPanes() const
{
	std::vector<PluginPane> res;

	return res;
}

int CodeGenerator::ResetImGuiID(const int& vWidgetId) {
    auto ids = ImGui::GetPUSHID();
    ImGui::SetPUSHID(vWidgetId);
    return ids;
}
