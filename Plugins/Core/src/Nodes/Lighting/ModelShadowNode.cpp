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

#include "ModelShadowNode.h"
#include <Modules/Lighting/ModelShadowModule.h>
#include <Interfaces/LightGroupOutputInterface.h>
#include <Interfaces/TextureGroupOutputInterface.h>
#include <Graph/Slots/NodeSlotLightGroupInput.h>
#include <Graph/Slots/NodeSlotTextureInput.h>
#include <Graph/Slots/NodeSlotTextureGroupInput.h>
#include <Graph/Slots/NodeSlotTextureOutput.h>

std::shared_ptr<ModelShadowNode> ModelShadowNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<ModelShadowNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

ModelShadowNode::ModelShadowNode() : BaseNode()
{
	m_NodeTypeString = "MODEL_SHADOW";
}

ModelShadowNode::~ModelShadowNode()
{
	Unit();
}

bool ModelShadowNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Model Shadow";

	AddInput(NodeSlotLightGroupInput::Create("Lights"), true, false);
	AddInput(NodeSlotTextureInput::Create("Position", 0U), true, false);
	AddInput(NodeSlotTextureInput::Create("Normal", 1U), true, false);
	AddInput(NodeSlotTextureGroupInput::Create("Shadow Maps"), true, false);
	AddOutput(NodeSlotTextureOutput::Create("Output", 0U), true, true);

	bool res = false;
	m_ModelShadowModulePtr = ModelShadowModule::Create(vVulkanCorePtr);
	if (m_ModelShadowModulePtr)
	{
		res = true;
	}

	return res;
}

void ModelShadowNode::Unit()
{
	m_ModelShadowModulePtr.reset();
}

bool ModelShadowNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureGroupInputDescriptorImageInfos(m_Inputs);

	if (m_ModelShadowModulePtr)
	{
		return m_ModelShadowModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}
	return false;
}

bool ModelShadowNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_ModelShadowModulePtr)
	{
		return m_ModelShadowModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void ModelShadowNode::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_ModelShadowModulePtr)
	{
		m_ModelShadowModulePtr->DrawOverlays(vCurrentFrame, vRect, vContext);
	}
}

void ModelShadowNode::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	if (m_ModelShadowModulePtr)
	{
		m_ModelShadowModulePtr->SetLightGroup(vSceneLightGroup);
	}
}

void ModelShadowNode::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	if (m_ModelShadowModulePtr)
	{
		m_ModelShadowModulePtr->SetTexture(vBinding, vImageInfo, vTextureSize);
	}
}

void ModelShadowNode::SetTextures(const uint32_t& vBinding, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes)
{
	if (m_ModelShadowModulePtr)
	{
		m_ModelShadowModulePtr->SetTextures(vBinding, vImageInfos, vOutSizes);
	}
}

vk::DescriptorImageInfo* ModelShadowNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_ModelShadowModulePtr)
	{
		return m_ModelShadowModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

void ModelShadowNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_ModelShadowModulePtr)
	{
		m_ModelShadowModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string ModelShadowNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_ModelShadowModulePtr)
		{
			res += m_ModelShadowModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool ModelShadowNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_ModelShadowModulePtr)
	{
		m_ModelShadowModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void ModelShadowNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_ModelShadowModulePtr)
	{
		return m_ModelShadowModulePtr->UpdateShaders(vFiles);
	}
}
