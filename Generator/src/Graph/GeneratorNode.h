#pragma once

#include <string>
#include <memory>
#include <vector>

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

#include <ctools/cTools.h>

class GeneratorNode;
typedef ct::cWeak<GeneratorNode> GeneratorNodeWeak;
typedef std::shared_ptr<GeneratorNode> GeneratorNodePtr;

#include <Graph/Graph.h>
#include <Graph/Base/BaseNode.h>
#include <vkFramework/VulkanCore.h>
#include <Project/ProjectFile.h>

class ProjectFile;
class GeneratorNode : public BaseNode
{
private:
	BaseTypes m_BaseTypes;

public:
	std::string m_ClassName = "NewClass";
	std::string m_Category = "None";
	std::string m_NodeCreationName = "NEW_NODE"; // node name maj ex : 2D_SIMULATION_GRAY_SCOTT
	std::string m_NodeDisplayName = "New Node"; // node name maj ex : Gray Scott
	bool m_GenerateAModule = false;
	bool m_GenerateAPass = false;
	std::string m_RendererType;

public:
	static GeneratorNodePtr Create(vkApi::VulkanCorePtr vVulkanCorePtr);

public:
	GeneratorNode();

public:
	void GenerateNodeClasses(const std::string& vPath, const ProjectFile* vDatas);
	void GenerateModules(const std::string& vPath, const ProjectFile* vDatas);
	void GeneratePasses(const std::string& vPath, const ProjectFile* vDatas);

private:
	std::string GetLicenceHeader();
	std::string GetPVSStudioHeader();

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
};