#pragma once

#include <Graph/Graph.h>
#include <Graph/Base/BaseNode.h>
#include <vkFramework/VulkanCore.h>

class GeneratorNode;
typedef ct::cWeak<GeneratorNode> GeneratorNodeWeak;
typedef std::shared_ptr<GeneratorNode> GeneratorNodePtr;

class GeneratorNode : public BaseNode
{
public:
	static GeneratorNodePtr Create(vkApi::VulkanCorePtr vVulkanCorePtr);

public:
	GeneratorNode();

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
};