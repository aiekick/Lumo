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

#include "ModelToAccelStructNode.h"
#include <Headers/RTXCommon.h>

#include <LumoBackend/Interfaces/ModelOutputInterface.h>
#include <LumoBackend/Graph/Slots/NodeSlotModelInput.h>
#include <Slots/NodeSlotAccelStructureOutput.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

std::shared_ptr<ModelToAccelStructNode> ModelToAccelStructNode::Create(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<ModelToAccelStructNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

ModelToAccelStructNode::ModelToAccelStructNode() : BaseNode()
{
	m_NodeTypeString = "RTX_MODEL_TO_ACCELERATION_STRUCTURE";
}

ModelToAccelStructNode::~ModelToAccelStructNode()
{
	Unit();
}

bool ModelToAccelStructNode::Init(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Rtx BVH Builder";

	AddInput(NodeSlotModelInput::Create("Model"), true, true);
	AddOutput(NodeSlotAccelStructureOutput::Create("Output"), true, true);

	bool res = false;

	m_SceneAccelStructurePtr = SceneAccelStructure::Create(vVulkanCorePtr);
	if (m_SceneAccelStructurePtr)
	{
		res = true;
	}

	return res;
}

void ModelToAccelStructNode::Unit()
{
	m_SceneAccelStructurePtr.reset();
}

void ModelToAccelStructNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void ModelToAccelStructNode::SetModel(SceneModelWeak vSceneModel)
{
	m_SceneModel = vSceneModel;

	if (m_SceneAccelStructurePtr)
	{
		m_SceneAccelStructurePtr->BuildForModel(m_SceneModel);
		if (m_SceneAccelStructurePtr->IsOk())
		{
			SendFrontNotification(AccelStructureUpdateDone);
		}
	}
}

SceneAccelStructureWeak ModelToAccelStructNode::GetAccelStruct()
{
	return m_SceneAccelStructurePtr;
}

vk::WriteDescriptorSetAccelerationStructureKHR* ModelToAccelStructNode::GetTLASInfo()
{
	if (m_SceneAccelStructurePtr && 
		m_SceneAccelStructurePtr->IsOk())
	{
		return m_SceneAccelStructurePtr->GetTLASInfo();
	}

	return nullptr;
}

vk::DescriptorBufferInfo* ModelToAccelStructNode::GetBufferAddressInfo()
{
	if (m_SceneAccelStructurePtr &&
		m_SceneAccelStructurePtr->IsOk())
	{
		return m_SceneAccelStructurePtr->GetBufferAddressInfo();
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string ModelToAccelStructNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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
			(uint32_t)nodeID.Get());

		for (auto slot : m_Inputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}
			
		for (auto slot : m_Outputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool ModelToAccelStructNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	return true;
}