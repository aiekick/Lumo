#pragma once

#include <vector>
#include <string>

enum BaseTypeEnum
{
	BASE_TYPE_None = 0,
	BASE_TYPE_LightGroup,
	BASE_TYPE_Model,
	BASE_TYPE_StorageBuffer,
	BASE_TYPE_TexelBuffer,
	BASE_TYPE_Texture,
	BASE_TYPE_TextureGroup,
	BASE_TYPE_Variable,
	BASE_TYPE_Custom
};

class BaseTypes
{
public:
	std::vector<std::string> m_TypeArray =
	{
		"None",
		"LightGroup",
		"Model",
		"StorageBuffer",
		"TexelBuffer",
		"Texture",
		"TextureGroup",
		"Variable",
		"Custom"
	};
};

#include <Graph/Graph.h>
#include <ctools/cTools.h>
#include <Editor/SlotEditor.h>
#include <ctools/ConfigAbstract.h>
#include <vkFramework/VulkanCore.h>
#include <Graph/Base/NodeSlotInput.h>
#include <Graph/Base/NodeSlotOutput.h>
#include <Generator/NodeGenerator.h>
#include <Project/ProjectFile.h>

class NodeGenerator
{
private:
	BaseTypes m_BaseTypes;

public:
	void GenerateNodeClasses(const std::string& vPath, const ProjectFile& vDatas);
	void GenerateModules(const std::string& vPath, const ProjectFile& vDatas);
	void GeneratePasses(const std::string& vPath, const ProjectFile& vDatas);

private:
	std::string GetLicenceHeader();
	std::string GetPVSStudioHeader();
};