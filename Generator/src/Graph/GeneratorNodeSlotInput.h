#pragma once

#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Graph/Base/NodeSlotInput.h>
#include <LumoBackend/Graph/GeneratorCommon.h>
#include <ctools/cTools.h>

class GeneratorNodeSlotInput;
typedef ct::cWeak<GeneratorNodeSlotInput> GeneratorNodeSlotInputWeak;
typedef std::shared_ptr<GeneratorNodeSlotInput> GeneratorNodeSlotInputPtr;

class GeneratorNodeSlotInput : public NodeSlotInput, public GeneratorNodeSlotDatas
{
public:
	static GeneratorNodeSlotInputPtr Create();
	static GeneratorNodeSlotInputPtr Create(const std::string& vName);

public:
	explicit GeneratorNodeSlotInput();
	explicit GeneratorNodeSlotInput(const std::string& vName);

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};
