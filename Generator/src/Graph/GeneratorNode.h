#pragma once

#include <string>
#include <memory>
#include <Graph/Graph.h>
#include <Graph/Base/BaseNode.h>
#include <vkFramework/VulkanCore.h>

class GeneratorNode;
typedef ct::cWeak<GeneratorNode> GeneratorNodeWeak;
typedef std::shared_ptr<GeneratorNode> GeneratorNodePtr;

class GeneratorNode : public BaseNode
{
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
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
};