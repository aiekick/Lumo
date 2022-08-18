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

#include <imgui/imgui.h>
#include <Graph/Graph.h>
#include <uTypes/uTypes.h>
#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <Graph/Base/NodeSlotInput.h>
#include <Graph/Base/NodeSlotOutput.h>
#include <Graph/Base/NodeLink.h>
#include <ctools/ConfigAbstract.h>
#include <Interfaces/GuiInterface.h>
#include <Interfaces/NodeInterface.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/NotifyInterface.h>
#include <Interfaces/ResizerInterface.h>
#include <vkFramework/vkFramework.h>

#include <imgui_node_editor/NodeEditor/Include/imgui_node_editor.h>
namespace nd = ax::NodeEditor;

#include <imgui/imgui_internal.h>

#include <unordered_map>
#include <map>
#include <utility>
#include <memory>
#include <string>
#include <set>
#include <unordered_set>
#include <array>
#include <functional>

#define MAGIC_NUMBER 4577

/*
POUR EFFACER LES NODES
il faut appeler la fonction DestroyNodesIfAnys() apres le rendu de imgui voir meme apres le swap buffer ou swapchain
*/

struct GraphStyleStruct
{
	// node
	float DEFAULT_WIDTH = 100.0f;
	float WINDOW_PADDING = 10.0f;
	float BACKGROUND_RADIUS = 0.5f;
	ImVec4 HEADER_COLOR = ImVec4(0.05f, 0.05f, 0.95f, 1.0f);
	ImVec4 HOVERED_HEADER_COLOR = ImVec4(0.1f, 0.1f, 0.95f, 1.0f);
	ImVec4 BACKGROUND_COLOR = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
	ImVec4 HOVERED_BACKGROUND_COLOR = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	
	// slot
	float SLOT_RADIUS = 5.0f;
	ImVec4 SLOT_COLOR = ImVec4(0.8f, 0.8f, 0.2f, 1.0f);
	ImVec4 FLOW_SLOT_COLOR = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImVec4 FUNCTION_SLOT_COLOR = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
	
	// grid
	ImVec4 GRID_COLOR = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
	ImVec4 GRID_COLOR_ZERO_X = ImVec4(0.8f, 0.2f, 0.2f, 0.8f);
	ImVec4 GRID_COLOR_ZERO_Y = ImVec4(0.2f, 0.8f, 0.2f, 0.8f);
	float GRID_SPACING = 64.0f;

	// link
	ImVec4 linkDefaultColor = ImVec4(0.4f, 0.6f, 0.3f, 1.0f);
	ImVec4 selectedLinkColor = ImVec4(0.5f, 0.8f, 0.5f, 1.0f);
	ImVec4 extractionLinkColor = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
	ImVec4 selectedExtractionLinkColor = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);

	// graph
	nd::Style graphStyle;
};

struct OldFuncToChangeStruct
{
	std::string parentFunc;
	std::string callName;
	std::string stamp;
	std::string sourceFunc;
	std::string sourceFullFunc;
	ct::uvec2 sourceLoc;
	size_t start = 0;
	size_t end = 0;

	OldFuncToChangeStruct()
	{
		start = 0;
		end = 0;
	}
};

struct Func_Loc_In_Code_Struct
{
	std::string parentFunc;
	std::string stamp;
	std::string sourceFunc;
	std::vector<ct::uvec2> locFunc;
	ct::uvec2 sourceLoc;
	std::string sourceFullFunc;
	std::vector<ct::uvec2> locFullFunc;
	//size_t start;
	//size_t end;
	std::string replacingFuncName;
	std::string replacingFullFunc;

	Func_Loc_In_Code_Struct() = default;
};

enum class LINK_TYPE_Enum : uint8_t
{
	LINK_TYPE_LINE = 0,
	LINK_TYPE_SPLINE,
	LINK_TYPE_STRAIGHT,
	LINK_TYPE_Count
};

struct BaseNodeState
{
	ImGuiContext* m_Context = nullptr;
	nd::EditorContext* m_NodeGraphContext = nullptr;
	uint32_t m_CurrentFrame = 0U;

	GraphStyleStruct graphStyle;

	bool debug_mode = false;

	ImVec2 scrolling;

	int itemPushId = -1; // imgui widget id

	bool hoveredItem = false;
	bool activeItem = false;

	bool is_any_hovered_items = false;
	bool is_any_active_items = false;

	BaseNodeWeak current_hovered_node;
	BaseNodeWeak current_moving_node;
	BaseNodeWeak current_selected_node;

	LINK_TYPE_Enum linkType = LINK_TYPE_Enum::LINK_TYPE_SPLINE;
	bool showLinks = true;
	bool showLinksBehind = true;
	bool showUnUsedNodes = true;
	bool showUniforms = true;
	bool showExtractedFuncs = true;
	bool showFunctionParametersSlots = true;
	bool showFunctionSlots = true;

	NodeSlotWeak linkFromSlot; // quand on prend un lien depuis un slot
	
	BaseNodeWeak node_to_open;
	BaseNodeWeak node_to_select;

	// custom context menu
	bool m_CustomContextMenuRequested = false;
	BaseNodeWeak m_CustomContextMenuNode;
};

class BaseNode : 
	public conf::ConfigAbstract, 
	public GuiInterface,
	public NodeInterface,
	public TaskInterface,
	public ResizerInterface
{
public:
	static BaseNodePtr Create(vkApi::VulkanCorePtr vVulkanCorePtr);

public:
	BaseNodeWeak m_This;
	BaseNodeWeak m_GraphRoot3DNode;
	BaseNodeWeak m_GraphRoot2DNode;

public: // links
	std::unordered_map<uint32_t, std::shared_ptr<NodeLink>> m_Links; // linkId, link // for search query
	std::unordered_map<uint32_t, std::set<uint32_t>> m_LinksDico; // NodeSlot Ptr, linkId // for search query

public: // static
	static uint32_t freeNodeId;
	static uint32_t GetNextNodeId();
	// get shared from weak
	static BaseNodePtr GetSharedFromWeak(const BaseNodeWeak& vNode);

	static std::function<void(const BaseNodeWeak&)> sOpenGraphCallback; // open graph
	static void OpenGraph_Callback(const BaseNodeWeak& vNode);

	static std::function<void(const NodeSlotWeak&, const ImGuiMouseButton&)> sSelectSlotCallback; // select for be the graph output
	static void SelectSlot_Callback(const NodeSlotWeak& vSlot, const ImGuiMouseButton&);

	static std::function<void(const NodeSlotWeak&, const ImGuiMouseButton&)> sSelectForGraphOutputCallback; // select for be the graph output
	static void SelectForGraphOutput_Callback(const NodeSlotWeak& vSlot, const ImGuiMouseButton&);

	static std::function<void(const std::string&)> sOpenCodeCallback; // open code
	static void OpenCode_Callback(const std::string& vCode);

	static std::function<void(const std::string&)> sLogErrorsCallback; // log errors
	static void LogErrors_Callback(const std::string& vErrors);

	static std::function<void(const std::string&)> sLogInfosCallback; // log infos
	static void LogInfos_Callback(const std::string& vInfos);

	static std::function<void(const BaseNodeWeak&)> sSelectCallback; // select node
	static void Select_Callback(const BaseNodeWeak& vNode);

	static std::function<BaseNodeWeak(BaseNodeWeak vNodeGraph, BaseNodeState* vBaseNodeState)> sShowNewNodeMenuCallback; // new node menu
	static void ShowNewNodeMenu_Callback(const BaseNodeWeak& vNodeGraph, BaseNodeState* vBaseNodeState);

	static std::function<bool(BaseNodeWeak, tinyxml2::XMLElement*, tinyxml2::XMLElement*,
		const std::string&, const std::string&, const ct::fvec2&, const size_t&)> sLoadNodeFromXMLCallback; // log infos
	static bool LoadNodeFromXML_Callback(const BaseNodeWeak& vBaseNodeWeak, tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent,
		const std::string& vNodeName, const std::string& vNodeType, const ct::fvec2& vPos, const size_t& vNodeId);

public: // popups
	bool m_CreateNewNode = false;
	ImVec2 m_OpenPopupPosition;
	nd::NodeId m_ContextMenuNodeId = 0;
	nd::PinId m_ContextMenuSlotId = 0;
	nd::LinkId m_ContextMenuLinkId = 0;
	ImRect m_HeaderRect = ImRect(0,0,0,0);

public: // ident 
	// on met ca en prems pour le voir direct dans le debugger quand on est au dessu d'un abstractNode
	// on verra le name en 1er ou second
	std::string name; // nom raccouris du node, genre nom de la fonction
	nd::NodeId nodeID;
	uType::uTypeEnum returnType = uType::uTypeEnum::U_VOID; // type du retour
	std::string uniquePaneId;
	
public:
	BaseNodeState m_BaseNodeState;
	std::string m_NodeTypeString = "NONE";

public:
	// only during laodibg for change slots
	// will save originals slot pin and new slot pin
	// duing loading new slot pin id are created and msut repalce slot pins form the xml
	// but the pin from xml must be saved for create the links from xml
	// so the frist is those from xml and the second is those from new slot pin
	std::unordered_map<NodeSlot::PlaceEnum, std::unordered_map<uint32_t, uint32_t>> m_SlotPinsFromXMLToNew;

public: // used by layout
	ImVec2 pos; // position absolue du node
	ImVec2 size; // taille du node
	bool used = false; // utilis� dans le code ou non
	bool hidden = false; // visibilit� du node
	ct::ivec2 cell = ct::ivec2(-1); // layout x:column, y:row
	bool inserted = false; // pour voir si il y a des doublon dasn des colonnes
	std::string rootFuncName = "main"; // le return du graph, genre "main" ou un return
	bool rootUsed = false; // ce node est le root
	bool graphDisabled = false; // pas possible d'ouvirr ce graph
	bool deletionDisabled = false; // pas possible d'ffacer ce node
	bool changed = false; // need to save

public: // parsing de la fonction pas du grpah complet
	std::string funcNameToParse;
	bool mainFuncParsing = false;

public: // Code Generation
	ct::uvec2 m_CodeBlock; // x:start / y:end in m_Code

public: // glslang and links
	int m_Depth = 0; // glslang
	std::unordered_map<uint32_t, BaseNodePtr> m_ChildNodes; // node du graphn  // for query only
	//std::string m_lastChildNodeName; // le nom du dernier node pour le layout
	
	std::map<uint32_t, NodeSlotInputPtr> m_Inputs; // for display, need order
	std::map<uint32_t, NodeSlotOutputPtr> m_Outputs; // for display, need order

	std::set<uint32_t> m_NodeIdToDelete; // node selected to delete

	ImVec2 m_BaseCopyOffset = ImVec2(0, 0);
	std::vector<nd::NodeId> m_NodesToCopy; // for copy/paste

private:
	bool m_IsCodeDirty = false; // code changed, need regeneration of code of parents

public:
	std::unordered_map<std::string, uint32_t> m_NodeStamps; // bd des signatures (de fonctions, params, return, etc..) for query only

public:
	std::string m_NodeGraphConfigFile;
	ax::NodeEditor::Style m_Style;
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;

public:
	BaseNode();
	virtual ~BaseNode();

public:
    virtual bool Init(vkApi::VulkanCorePtr vVulkanCorePtr);
	virtual bool Init(const BaseNodeWeak& vThis);
	virtual bool Init(const std::string& vCode, const BaseNodeWeak& vThis);
	void InitGraph(const ax::NodeEditor::Style& vStyle = ax::NodeEditor::Style());
	void FinalizeGraphLoading();

	virtual void Unit();
	void UnitGraph();

	void ClearNode();
	void ClearGraph();
	void ClearSlots();

	/// <summary>
	/// execute all time, each frames so
	/// </summary>
	/// <param name="vCurrentFrame"></param>
	/// <param name="vCmd"></param>
	/// <param name="vBaseNodeState"></param>
	/// <returns></returns>
	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;

	/// <summary>
	/// execute all input childs
	/// </summary>
	/// <param name="vCurrentFrame"></param>
	/// <param name="vCmd"></param>
	/// <param name="vBaseNodeState"></param>
	/// <returns></returns>
	bool ExecuteChilds(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr);

	/// <summary>
	/// apply some treatment about event, can be herited by nodes
	/// </summary>
	/// <param name="vEvent"></param>
	/// <param name="vEmitterSlot"></param>
	/// <param name="vReceiverSlot"></param>
	virtual void TreatNotification(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());

	/// <summary>
	/// send notification in front (output to input)
	/// </summary>
	/// <param name="vSlotType"></param>
	/// <param name="vEvent"></param>
	void SendFrontNotification(const NotifyEvent& vEvent);

	/// <summary>
	/// send notification of a type in front (output to input)
	/// </summary>
	/// <param name="vSlotType"></param>
	/// <param name="vEvent"></param>
	void SendFrontNotification(const std::string& vSlotType, const NotifyEvent& vEvent);

	/// <summary>
	/// propagate notification in front (output to input)
	/// </summary>
	/// <param name="vSlotType"></param>
	/// <param name="vEvent"></param>
	void PropagateFrontNotification(const NotifyEvent& vEventt);

	/// <summary>
	/// send notification of a type in back (input to output)
	/// </summary>
	/// <param name="vSlotType"></param>
	/// <param name="vEvent"></param>
	void SendBackNotification(const NotifyEvent& vEvent);

	/// <summary>
	/// send notification of a type in back (input to output)
	/// </summary>
	/// <param name="vSlotType"></param>
	/// <param name="vEvent"></param>
	void SendBackNotification(const std::string& vSlotType, const NotifyEvent& vEvent);

	/// <summary>
	/// propagate notification in front (output to input)
	/// </summary>
	/// <param name="vSlotType"></param>
	/// <param name="vEvent"></param>
	void PropagateBackNotification(const NotifyEvent& vEvent);

	void NeedResizeByHand(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers);
	void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
	ct::fvec2 GetOutputSize() override;

	virtual void UpdateSlots();
	virtual void ClearDescriptors();

	bool DrawGraph(); // dras graph of child
	bool GenerateGraphFromCode(const std::string& vCode);

	void ZoomToContent() const;
	void NavigateToContent() const;
	void ZoomToSelection() const;
	void NavigateToSelection() const;

	[[nodiscard]] ImVec2 GetCanvasOffset() const;
	[[nodiscard]] float GetCanvasScale() const;

	void SetCanvasOffset(const ImVec2& vOffset);
	void SetCanvasScale(const float& vScale);

	void CopySelectedNodes();
	void PasteNodesAtMousePos();
	void DuplicateSelectedNodes(ImVec2 vOffset = ImVec2(0,0));

	// finders
	BaseNodeWeak FindNode(nd::NodeId vId);
	BaseNodeWeak FindNodeByName(std::string vName);
	std::vector<BaseNodeWeak> GetPublicNodes();		// les Public nodes sont les nodes exposé dans le parents, c'est PUBLIC_ puis non du node
	ct::cWeak<NodeLink> FindLink(nd::LinkId vId);
	NodeSlotWeak FindSlot(nd::PinId vId);
	NodeSlotWeak FindNodeSlotByName(BaseNodeWeak vNode, std::string vName);
	NodeSlotWeak FindNodeSlotById(nd::NodeId vNodeId, nd::PinId vSlotId);

	/// <summary>
	/// ca retourner la liste des slots d'un type particulier et d'une place particuliere
	/// </summary>
	std::vector<NodeSlotWeak> GetSlotsOfType(NodeSlot::PlaceEnum vPlace, std::string vType);

	/// <summary>
	/// va retourne la liste des input slot d'un type particulier
	/// </summary>
	std::vector<NodeSlotWeak> GetInputSlotsOfType(std::string vType);

	/// <summary>
	/// va retourne la liste des output slot d'un type particulier
	/// </summary>
	std::vector<NodeSlotWeak> GetOutputSlotsOfType(std::string vType);

	// Add slots
	NodeSlotWeak AddInput(NodeSlotInputPtr vSlotPtr, bool vIncSlotId = false, bool vHideName = true);
	NodeSlotWeak AddOutput(NodeSlotOutputPtr  vSlotPtr, bool vIncSlotId = false, bool vHideName = true);

	// add nodes
	BaseNodeWeak AddChildNode(BaseNodePtr vNode, bool vIncNodeId = false);

	// // get the root node by exploring tree with by out call slots
	BaseNodePtr GetRootNodeByCalls(BaseNodeWeak vNode = BaseNodeWeak());

	// node removal
	void DestroyChildNode(BaseNodeWeak vNode);
	bool DestroyChildNodeByIdIfAllowed(int vNodeID, bool vDestroy);
	void DestroySlotOfAnyMap(NodeSlotWeak vSlot);

	// delete node finally
	void DestroyNodesIfAnys(); // a executer apres le rendu de imgui

	// need to save
	void SetChanged(bool vFlag = true);

	// when the xml laoding of the node is finished
	void AfterNodeXmlLoading() override;

public:
	void DoLayout();

public: // shader gen related stuff
	virtual std::string GetNodeCode(bool vRecursChilds = false);
	virtual void CompilGeneratedCode();
	//virtual void JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot);
	//virtual void JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot);
	bool IsCodeDirty();
	void SetCodeDirty(bool vFlag);
	
private: // graph states / action / drawings
	void DoGraphActions(BaseNodeState *vBaseNodeState);
	void OpenNodeInNewPane(BaseNodeState* vBaseNodeState);
	void SelectNodeforPreview(BaseNodeState* vBaseNodeState);
	void FillState(BaseNodeState *vBaseNodeState);
	void DoCreateLinkOrNode(BaseNodeState *vBaseNodeState);
	void DoDeleteLinkOrNode(BaseNodeState *vBaseNodeState);
	void DoShorcutsOnNode(BaseNodeState *vBaseNodeState);
	void DoPopups(BaseNodeState *vBaseNodeState);
	void DoCheckNodePopup(BaseNodeState *vBaseNodeState);
	void DoCheckSlotPopup(BaseNodeState *vBaseNodeState);
	void DoCheckLinkPopup(BaseNodeState *vBaseNodeState);
	void DoNewNodePopup(BaseNodeState *vBaseNodeState);
	
private: // node manipulation
	void DuplicateNode(uint32_t vNodeId, ImVec2 vOffsetPos);

private: // utils
	std::string GetAvailableNodeStamp(const std::string& vNodeStamp);

public: // Get Links / Slots
	std::vector<ct::cWeak<NodeLink>> GetLinksAssociatedToSlot(NodeSlotWeak vSlot);
	std::vector<NodeSlotWeak> GetSlotsAssociatedToSlot(NodeSlotWeak vSlot);

public: // ADD/DELETE VISUAL LINKS (NO CHANGE BEHIND)
	void Add_VisualLink(NodeSlotWeak vStart, NodeSlotWeak vEnd);
	void Del_VisualLink(uint32_t vLinkId);
	void Break_VisualLinks_ConnectedToSlot(NodeSlotWeak vSlot);
	void Break_VisualLink_ConnectedToSlots(NodeSlotWeak vFrom, NodeSlotWeak vTo);
	
public: // CONNECT / DISCONNECT SLOTS BEHIND
	bool ConnectSlots(NodeSlotWeak vFrom, NodeSlotWeak vTo);
	bool DisConnectSlots(NodeSlotWeak vFrom, NodeSlotWeak vTo);
	bool DisConnectSlot(NodeSlotWeak vSlot);
	virtual void NotifyConnectionChangeOfThisSlot(NodeSlotWeak vSlot, bool vConnected); // ce solt a été connecté/déconnecté

private:
	bool ConnectNodeSlots(NodeSlotWeak vStart, NodeSlotWeak vEnd);
	bool DisConnectNodeSlots(NodeSlotWeak vStart, NodeSlotWeak vEnd);

public: // test if slot connection possible for have rule node
	virtual bool CanWeConnectSlots(NodeSlotWeak vFrom, NodeSlotWeak vTo);

public: // slot splitter
	virtual std::vector<NodeSlotWeak> InjectTypeInSlot(NodeSlotWeak vSlotToSplit, uType::uTypeEnum vType);

public: // ImGui
	void DrawStyleMenu(); // ImGui
	void DrawNodeGraphStyleMenu() const; // imgui_node_editor
	void DrawToolMenu();

public: // gui interface
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext) override;

public: // pane
	virtual bool DrawDebugInfos(BaseNodeState *vBaseNodeState);
	virtual void DrawProperties(BaseNodeState* vBaseNodeState);

public: // draw nodes widgets (can be caleed from external)
	virtual void DrawInputWidget(BaseNodeState *vBaseNodeState, NodeSlotWeak vSlot);
	virtual void DrawOutputWidget(BaseNodeState *vBaseNodeState, NodeSlotWeak vSlot);
	virtual void DrawContextMenuForSlot(BaseNodeState *vBaseNodeState, NodeSlotWeak vSlot);
	virtual void DrawContextMenuForNode(BaseNodeState *vBaseNodeState);
	virtual void DrawCustomContextMenuForNode(BaseNodeState *vBaseNodeState);

protected: // cant bt called from external, must be derived
	virtual void DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState);
	virtual void DrawNode(BaseNodeState *vBaseNodeState);
	virtual bool DrawBegin(BaseNodeState *vBaseNodeState);
	virtual bool DrawHeader(BaseNodeState *vBaseNodeState);
	virtual bool DrawNodeContent(BaseNodeState *vBaseNodeState);
	virtual bool DrawFooter(BaseNodeState *vBaseNodeState);
	virtual bool DrawEnd(BaseNodeState *vBaseNodeState);
	virtual void DrawLinks(BaseNodeState *vBaseNodeState);
	
public: // loading / saving
	typedef std::pair<uint32_t, uint32_t> SlotEntry;
	typedef std::pair<SlotEntry, SlotEntry> LinkEntry;
	std::vector<LinkEntry> m_LinksToBuildAfterLoading;
	SlotEntry m_OutputLeftSlotToSelectAfterLoading;
	SlotEntry m_OutputMiddleSlotToSelectAfterLoading;
	SlotEntry m_OutputRightSlotToSelectAfterLoading;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
};