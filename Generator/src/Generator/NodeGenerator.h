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

struct GeneratorStruct
{
public:
	std::string m_FilePathNameToLoad;
	std::string m_ClassName = "NewClass";
	std::string m_Category = "None";
	std::string m_NodeCreationName = "NEW_NODE"; // node name maj ex : 2D_SIMULATION_GRAY_SCOTT
	std::string m_NodeDisplayName= "New Node"; // node name maj ex : Gray Scott
	bool m_GenerateAModule = false;
	bool m_GenerateAPass = false;
	std::string m_RendererType;
	BaseNodeWeak m_SelectedNode;
};

class NodeGenerator
{
private:
	BaseTypes m_BaseTypes;

public:
	void GenerateNodeClasses(const std::string& vPath, const GeneratorStruct& vDatas);
	void GenerateModules(const std::string& vPath, const GeneratorStruct& vDatas);
	void GeneratePasses(const std::string& vPath, const GeneratorStruct& vDatas);

private:
	std::string GetLicenceHeader();
	std::string GetPVSStudioHeader();
};