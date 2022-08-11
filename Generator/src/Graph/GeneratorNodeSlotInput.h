#pragma once

#include <Graph/Graph.h>
#include <Graph/Base/NodeSlotInput.h>
#include <Graph/GeneratorCommon.h>
#include <ctools/cTools.h>

class GeneratorNodeSlotInput;
typedef ct::cWeak<GeneratorNodeSlotInput> GeneratorNodeSlotInputWeak;
typedef std::shared_ptr<GeneratorNodeSlotInput> GeneratorNodeSlotInputPtr;

class GeneratorNodeSlotInput : public NodeSlotInput, public GeneratorNodeSlotDatas
{
public:
	static GeneratorNodeSlotInputPtr Create();

public:
	GeneratorNodeSlotInput();

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};
