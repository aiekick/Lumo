#pragma once

#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Graph/Base/NodeSlotOutput.h>
#include <LumoBackend/Graph/GeneratorCommon.h>
#include <ctools/cTools.h>

class GeneratorNodeSlotOutput;
typedef ct::cWeak<GeneratorNodeSlotOutput> GeneratorNodeSlotOutputWeak;
typedef std::shared_ptr<GeneratorNodeSlotOutput> GeneratorNodeSlotOutputPtr;

class GeneratorNodeSlotOutput : public NodeSlotOutput, public GeneratorNodeSlotDatas
{
public:
	static GeneratorNodeSlotOutputPtr Create();
	static GeneratorNodeSlotOutputPtr Create(const std::string& vName);

public:
	explicit GeneratorNodeSlotOutput();
	explicit GeneratorNodeSlotOutput(const std::string& vName);

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};
