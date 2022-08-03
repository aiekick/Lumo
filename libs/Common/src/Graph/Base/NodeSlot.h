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

class SlotColor
{
private:
	std::map<std::string, ImVec4> m_ColorSlots;

public:
	SlotColor();
	ImVec4 GetSlotColor(const std::string& vNodeSlotType);
	void AddSlotColor(const std::string& vNodeSlotType, const ImVec4& vSlotColor);
};

struct ImRect;
class BaseNode;
class UniformWidgetBase;
struct BaseNodeState;
class NodeSlot :
	public conf::ConfigAbstract
{
public:
	enum class PlaceEnum : uint8_t
	{
		NONE = 0,
		INPUT,
		OUTPUT,
		Count
	};

public:
	static NodeSlotWeak sSlotGraphOutputMouseLeft;
	static NodeSlotWeak sSlotGraphOutputMouseMiddle;
	static NodeSlotWeak sSlotGraphOutputMouseRight;

public:
	static std::string sGetStringFromNodeSlotPlaceEnum(const PlaceEnum& vPlaceEnum);
	static PlaceEnum sGetNodeSlotPlaceEnumFromString(const std::string& vNodeSlotPlaceString);
	static size_t sGetNewSlotId();
	static SlotColor* sGetSlotColors(SlotColor* vCopy = nullptr, bool vForce = false); // static are null when a plugin is loaded
	static NodeSlotPtr Create(NodeSlot vSlot);

public:
	NodeSlotWeak m_This;

public:
	std::string slotType = "NONE";
	NodeSlot::PlaceEnum slotPlace = NodeSlot::PlaceEnum::INPUT;

public:
	nd::PinId pinID = 0;
	std::string name;
	uint32_t index = 0U; // index of the slot for xml
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

private:
	bool m_Selected = false; // will select visually the slot, signify he is the output of the graph

public:
	explicit NodeSlot();
	explicit NodeSlot(std::string vName);
	explicit NodeSlot(std::string vName, std::string vType);
	explicit NodeSlot(std::string vName, std::string vType, bool vHideName);
	explicit NodeSlot(std::string vName, std::string vType, bool vHideName, bool vShowWidget);
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

	void DrawContent(BaseNodeState *vBaseNodeState);
	void DrawSlot(BaseNodeState *vBaseNodeState, ImVec2 vSlotSize, ImVec2 vSlotOffset = ImVec2(2,2));
	
	bool IsAnInput();
	bool IsAnOutput();

	void Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());
	void SendNotification(const std::string& vSlotType, const NotifyEvent& vEvent);

	/// <summary>
	/// When a Connect event is detected (to be herited)
	/// </summary>
	/// <param name="vOtherSlot">the slot to Connect to</param>
	virtual void Connect(NodeSlotWeak vOtherSlot);

	/// <summary>
	/// When a DisConnect event is detected (to be herited)
	/// </summary>
	/// <param name="vOtherSlot">the slot to DisConnect to</param>
	virtual void DisConnect(NodeSlotWeak vOtherSlot);

	/// <summary>
	/// Treat an event (to be herited)
	/// </summary>
	/// <param name="vEvent"></param>
	/// <param name="vEmitterSlot"></param>
	/// <param name="vReceiverSlot"></param>
	virtual void TreatNotification(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());
	
	/// <summary>
	/// Send a event in front (to be herited)
	/// </summary>
	/// <param name="vEvent"></param>
	virtual void SendFrontNotification(const NotifyEvent& vEvent);

	/// <summary>
	/// Send a event in back (to be herited)
	/// </summary>
	/// <param name="vEvent"></param>
	virtual void SendBackNotification(const NotifyEvent& vEvent);

	/// <summary>
	/// called when a slot was double clicked with mouse
	/// </summary>
	/// <param name="vMouseButton"></param>
	virtual void MouseDoubleClickedOnSlot(const ImGuiMouseButton& vMouseButton);

	void DrawDebugInfos();

private:
	void DrawInputWidget(BaseNodeState *vBaseNodeState);
	void DrawOutputWidget(BaseNodeState *vBaseNodeState);
	void DrawSlotText(BaseNodeState *vBaseNodeState);

	void DrawNodeSlot(ImDrawList *vDrawList, ImVec2 vCenter, float vSlotRadius, bool vConnected, ImU32 vColor, ImU32 vInnerColor);
};