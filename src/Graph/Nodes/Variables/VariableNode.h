/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <Graph/Graph.h>
#include <Graph/Base/BaseNode.h>
#include <Interfaces/VariableOutputInterface.h>

class VariableModule;
class VariableNode : 
	public BaseNode,
	public VariableOutputInterface
{
public:
	static std::shared_ptr<VariableNode> Create(vkApi::VulkanCore* vVulkanCore, const NodeTypeEnum& vNodeType);

private:
	std::shared_ptr<VariableModule> m_VariableModulePtr = nullptr;

public:
	VariableNode(const NodeTypeEnum& vNodeType);
	~VariableNode() override;
	bool Init(vkApi::VulkanCore* vVulkanCore) override;
	bool Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void DisplayInfosOnTopOfTheNode(BaseNodeStateStruct* vCanvasState) override;
	void Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak()) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
	void DrawOutputWidget(BaseNodeStateStruct* vCanvasState, NodeSlotWeak vSlot) override;
	SceneVariableWeak GetVariable() override;
};