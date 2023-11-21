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

#include "MeshBuffersNode.h"
#include <Graph/Modules/Utils/MeshBuffersModule.h>
#include <LumoBackend/Interfaces/ModelOutputInterface.h>
#include <LumoBackend/Graph/Slots/NodeSlotModelInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureOutput.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

std::shared_ptr<MeshBuffersNode> MeshBuffersNode::Create(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<MeshBuffersNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

MeshBuffersNode::MeshBuffersNode() : BaseNode()
{
	m_NodeTypeString = "MESH_BUFFERS";
}

MeshBuffersNode::~MeshBuffersNode()
{
	Unit();
}

bool MeshBuffersNode::Init(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Mesh Buffers";

	AddInput(NodeSlotModelInput::Create("Model"), true, false);
	AddInput(NodeSlotTextureInput::Create("Mask", 0U), true, false);
	AddOutput(NodeSlotTextureOutput::Create("Position", 0U), true, false);
	AddOutput(NodeSlotTextureOutput::Create("Normal", 1U), true, false);
	AddOutput(NodeSlotTextureOutput::Create("Tangeant", 2U), true, false);
	AddOutput(NodeSlotTextureOutput::Create("BiTangeant", 3U), true, false);
	AddOutput(NodeSlotTextureOutput::Create("UV", 4U), true, false);
	AddOutput(NodeSlotTextureOutput::Create("Color", 5U), true, false);
	AddOutput(NodeSlotTextureOutput::Create("Depth", 6U), true, false);

	bool res = false;

	m_MeshBuffersModulePtr = MeshBuffersModule::Create(vVulkanCorePtr);
	if (m_MeshBuffersModulePtr)
	{
		res = true;
	}

	return res;
}

void MeshBuffersNode::Unit()
{
	m_MeshBuffersModulePtr.reset();
}

bool MeshBuffersNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);

	if (m_MeshBuffersModulePtr)
	{
		return m_MeshBuffersModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}
	return false;
}

bool MeshBuffersNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_MeshBuffersModulePtr)
	{
		return m_MeshBuffersModulePtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
	}

	return false;
}

bool MeshBuffersNode::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool MeshBuffersNode::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_MeshBuffersModulePtr)
	{
        return m_MeshBuffersModulePtr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
	}

	return false;
}

void MeshBuffersNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
{
	if (vBaseNodeState && vBaseNodeState->debug_mode)
	{
		auto drawList = nd::GetNodeBackgroundDrawList(nodeID);
		if (drawList)
		{
			char debugBuffer[255] = "\0";
			snprintf(debugBuffer, 254,
				"Used(%s)\nCell(%i, %i)"/*\nPos(%.1f, %.1f)\nSize(%.1f, %.1f)*/,
				(used ? "true" : "false"), cell.x, cell.y/*, pos.x, pos.y, size.x, size.y*/);
			ImVec2 txtSize = ImGui::CalcTextSize(debugBuffer);
			drawList->AddText(pos - ImVec2(0, txtSize.y), ImGui::GetColorU32(ImGuiCol_Text), debugBuffer);
		}
	}
}

void MeshBuffersNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_MeshBuffersModulePtr)
	{
		m_MeshBuffersModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void MeshBuffersNode::SetModel(SceneModelWeak vSceneModel)
{
	if (m_MeshBuffersModulePtr)
	{
		m_MeshBuffersModulePtr->SetModel(vSceneModel);
	}
}

void MeshBuffersNode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	if (m_MeshBuffersModulePtr)
	{
		m_MeshBuffersModulePtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
}

vk::DescriptorImageInfo* MeshBuffersNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_MeshBuffersModulePtr)
	{
		return m_MeshBuffersModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string MeshBuffersNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string res;

	if (!m_ChildNodes.empty())
	{
		res += BaseNode::getXml(vOffset, vUserDatas);
	}
	else
	{
		res += vOffset + ct::toStr("<node name=\"%s\" type=\"%s\" pos=\"%s\" id=\"%u\">\n",
			name.c_str(),
			m_NodeTypeString.c_str(),
			ct::fvec2(pos.x, pos.y).string().c_str(),
			(uint32_t)GetNodeID());

		for (auto slot : m_Inputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}
			
		for (auto slot : m_Outputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		if (m_MeshBuffersModulePtr)
		{
			res += m_MeshBuffersModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool MeshBuffersNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	BaseNode::setFromXml(vElem, vParent, vUserDatas);

	if (m_MeshBuffersModulePtr)
	{
		m_MeshBuffersModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void MeshBuffersNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_MeshBuffersModulePtr)
	{
		m_MeshBuffersModulePtr->UpdateShaders(vFiles);
	}
}