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

#pragma once

#include <Graph/Graph.h>
#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <uTypes/uTypes.h>
#include "NodeStamp.h"
#include <imgui/imgui.h>
#include <string>
#include <array>
#include <vector>
#include <memory>
#include <ctools/ConfigAbstract.h>
#include <imgui_node_editor/NodeEditor/Include/imgui_node_editor.h>
#include <Interfaces/NotifyInterface.h>

namespace nd = ax::NodeEditor;

enum class NodeSlotTypeEnum : uint8_t
{
	NONE = 0,
	MESH,
	MESH_GROUP,
	LIGHT_GROUP,
	ENVIRONMENT,
	MERGED,
	TEXTURE_2D,
	TEXTURE_2D_GROUP,
	MIXED,
	TYPE_BOOLEAN,
	TYPE_UINT,
	TYPE_INT,
	TYPE_FLOAT,
	DEPTH,
	Count
};

inline static std::string GetStringFromNodeSlotTypeEnum(const NodeSlotTypeEnum& vNodeSlotTypeEnum)
{
	static std::array<std::string, (uint32_t)NodeSlotTypeEnum::Count> NodeSlotTypeString = {
		"NONE",
		"MESH",
		"MESH_GROUP",
		"LIGHT_GROUP",
		"ENVIRONMENT",
		"MERGED",
		"TEXTURE_2D",
		"TEXTURE_2D_GROUP",
		"MIXED",
		"TYPE_BOOLEAN",
		"TYPE_UINT",
		"TYPE_INT",
		"TYPE_FLOAT",
		"DEPTH",
	};
	if (vNodeSlotTypeEnum != NodeSlotTypeEnum::Count)
		return NodeSlotTypeString[(int)vNodeSlotTypeEnum];
	LogVarDebug("Error, one NodeSlotTypeEnum have no corresponding string, return \"None\"");
	return "NONE";
}

inline static NodeSlotTypeEnum GetNodeSlotTypeEnumFromString(const std::string& vNodeSlotTypeString)
{
	if (vNodeSlotTypeString == "NONE") return NodeSlotTypeEnum::NONE;
	else if (vNodeSlotTypeString == "MESH") return NodeSlotTypeEnum::MESH;
	else if (vNodeSlotTypeString == "MESH_GROUP") return NodeSlotTypeEnum::MESH;
	else if (vNodeSlotTypeString == "LIGHT_GROUP") return NodeSlotTypeEnum::LIGHT_GROUP;
	else if (vNodeSlotTypeString == "ENVIRONMENT") return NodeSlotTypeEnum::ENVIRONMENT;
	else if (vNodeSlotTypeString == "MERGED") return NodeSlotTypeEnum::MERGED;
	else if (vNodeSlotTypeString == "TEXTURE_2D") return NodeSlotTypeEnum::TEXTURE_2D;
	else if (vNodeSlotTypeString == "TEXTURE_2D_GROUP") return NodeSlotTypeEnum::TEXTURE_2D_GROUP;
	else if (vNodeSlotTypeString == "MIXED") return NodeSlotTypeEnum::MIXED;
	else if (vNodeSlotTypeString == "TYPE_BOOLEAN") return NodeSlotTypeEnum::TYPE_BOOLEAN;
	else if (vNodeSlotTypeString == "TYPE_UINT") return NodeSlotTypeEnum::TYPE_UINT;
	else if (vNodeSlotTypeString == "TYPE_INT") return NodeSlotTypeEnum::TYPE_INT;
	else if (vNodeSlotTypeString == "TYPE_FLOAT") return NodeSlotTypeEnum::TYPE_FLOAT;
	else if (vNodeSlotTypeString == "DEPTH") return NodeSlotTypeEnum::DEPTH;
	return NodeSlotTypeEnum::NONE;
}

enum class NodeSlotPlaceEnum : uint8_t
{
	NONE = 0,
	INPUT,
	OUTPUT,
	Count
};

inline static std::string GetStringFromNodeSlotPlaceEnum(const NodeSlotPlaceEnum& vNodeSlotPlaceEnum)
{
	static std::array<std::string, (uint32_t)NodeSlotPlaceEnum::Count> NodeSlotPlaceString = {
		"NONE",
		"INPUT",
		"OUTPUT",
	};
	if (vNodeSlotPlaceEnum != NodeSlotPlaceEnum::Count)
		return NodeSlotPlaceString[(int)vNodeSlotPlaceEnum];
	LogVarDebug("Error, one NodeSlotvNodeSlotPlaceEnumEnum have no corresponding string, return \"None\"");
	return "NONE";
}

inline static NodeSlotPlaceEnum GetNodeSlotPlaceEnumFromString(const std::string& vNodeSlotPlaceString)
{
	if (vNodeSlotPlaceString == "NONE") return NodeSlotPlaceEnum::NONE;
	else if (vNodeSlotPlaceString == "INPUT") return NodeSlotPlaceEnum::INPUT;
	else if (vNodeSlotPlaceString == "OUTPUT") return NodeSlotPlaceEnum::OUTPUT;
	return NodeSlotPlaceEnum::NONE;
}

struct ImRect;
class BaseNode;
class UniformWidgetBase;
struct BaseNodeStateStruct;
class NodeSlot : 
	public conf::ConfigAbstract,
	public NotifyInterface
{
public:
	static size_t GetNewSlotId();
	static ImVec4 GetSlotColorAccordingToType(const NodeSlotTypeEnum& vNodeSlotType);
	static NodeSlotPtr Create(NodeSlot vSlot);

public:
	NodeSlotWeak m_This;

public:
	NodeSlotTypeEnum slotType = NodeSlotTypeEnum::NONE;
	NodeSlotPlaceEnum slotPlace = NodeSlotPlaceEnum::INPUT;

public:
	nd::PinId pinID = 0;
	std::string name;
	uint32_t descriptorBinding = 0u;
	uint32_t variableIndex = 0u;
	std::string help;
	NodeStamp stamp; // style vec3(vec3,float), pour le linking
	ImVec4 color = ImVec4(0.8f, 0.8f, 0.0f, 1.0f);
	bool colorIsSet = false;
	uType::uTypeEnum type = uType::uTypeEnum::U_VOID;
	std::vector<NodeSlotWeak> linkedSlots;
	BaseNodeWeak parentNode;
	ct::cWeak<UniformWidgetBase> uniform;
	uint32_t fboAttachmentId = 0;
	uint32_t channelId = 0; // by ex is this slot is the b channel of a vec4, channelId will be 2
	bool idAlreadySetbyXml = false; // know if id was already set by xml
	bool acceptManyInputs = false; // one input can be connected to more than one output

	std::string originalFuncName;
	std::string funcCode; // code de la function si c'est un slot de type FUNCTION

	bool IsGenericType()
	{
		return 
			type == uType::uTypeEnum::U_TYPE || 
			type == uType::uTypeEnum::U_VEC || 
			type == uType::uTypeEnum::U_MAT;
	}
	bool IsGenericStampType()
	{
		return 
			stamp.typeStamp == "type" || 
			stamp.typeStamp == "vec" ||
			stamp.typeStamp == "mat";
	}

public:
	ImVec2 pos;
	bool highLighted = false;
	bool connected = false; // connect� a une autre node
	bool hideName = false; // n'affiche pas le nom du slot
	bool showWidget = false;
	bool hidden = false; // si true, il ne sera pas visible mais toujours present
	bool virtualUniform = false; // virtual si l'uniform n'est pas utilisé

public:
	explicit NodeSlot();
	explicit NodeSlot(std::string vName);
	explicit NodeSlot(std::string vName, NodeSlotTypeEnum vType);
	explicit NodeSlot(std::string vName, NodeSlotTypeEnum vType, bool vHideName);
	explicit NodeSlot(std::string vName, NodeSlotTypeEnum vType, bool vHideName, bool vShowWidget);
	~NodeSlot();

	void Init();
	void Unit();

	std::string GetFullStamp();

	void NotifyConnectionChangeToParent(bool vConnected); // va contacter le parent pour lui dire que ce solt est connecté a un autre
	bool CanWeConnectToSlot(NodeSlotWeak vSlot);

	// splitter
	std::vector<NodeSlotWeak> InjectTypeInSlot(uType::uTypeEnum vType);

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

	void DrawContent(BaseNodeStateStruct *vCanvasState);
	void DrawSlot(BaseNodeStateStruct *vCanvasState, ImVec2 vSlotSize, ImVec2 vSlotOffset = ImVec2(0,0));
	
	bool IsAnInput();
	bool IsAnOutput();

	void Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak()) override;

	void DrawDebugInfos();

private:
	void DrawInputWidget(BaseNodeStateStruct *vCanvasState);
	void DrawOutputWidget(BaseNodeStateStruct *vCanvasState);
	void DrawSlotText(BaseNodeStateStruct *vCanvasState);

	void DrawNodeSlot(ImDrawList *vDrawList, ImVec2 vCenter, float vSlotRadius, bool vConnected, ImU32 vColor, ImU32 vInnerColor);
};