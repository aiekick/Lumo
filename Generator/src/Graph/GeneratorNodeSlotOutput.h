#pragma once

#include <Graph/Graph.h>
#include <Graph/Base/NodeSlotOutput.h>
#include <ctools/cTools.h>

class GeneratorNodeSlotOutput;
typedef ct::cWeak<GeneratorNodeSlotOutput> GeneratorNodeSlotOutputWeak;
typedef std::shared_ptr<GeneratorNodeSlotOutput> GeneratorNodeSlotOutputPtr;


class GeneratorNodeSlotOutput : public NodeSlotOutput
{
public:
	static GeneratorNodeSlotOutputPtr Create();

public:
	GeneratorNodeSlotOutput();

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};
