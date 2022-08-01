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

#include "ParticlesSpriteRendererNode.h"
#include <Modules/Renderers/ParticlesSpriteRenderer.h>
#include <Interfaces/TexelBufferOutputInterface.h>

std::shared_ptr<ParticlesSpriteRendererNode> ParticlesSpriteRendererNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<ParticlesSpriteRendererNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

ParticlesSpriteRendererNode::ParticlesSpriteRendererNode() : BaseNode()
{
	m_NodeTypeString = "PARTICLES_SPRITE_RENDERER";
}

ParticlesSpriteRendererNode::~ParticlesSpriteRendererNode()
{
	Unit();
}

bool ParticlesSpriteRendererNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Sprite Renderer";

	NodeSlot slot;
	
	slot.slotType = "PARTICLES";
	slot.name = "Particles";
	AddInput(slot, true, false);

	slot.slotType = "TEXTURE_2D";
	slot.name = "Output";
	AddOutput(slot, true, true);

	bool res = false;

	m_ParticlesSpriteRenderer = ParticlesSpriteRenderer::Create(vVulkanCorePtr);
	if (m_ParticlesSpriteRenderer)
	{
		res = true;
	}

	return res;
}

void ParticlesSpriteRendererNode::Unit()
{
	m_ParticlesSpriteRenderer.reset();
}

bool ParticlesSpriteRendererNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	if (m_ParticlesSpriteRenderer)
	{
		return m_ParticlesSpriteRenderer->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}

	return false;
}

bool ParticlesSpriteRendererNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_ParticlesSpriteRenderer)
	{
		return m_ParticlesSpriteRenderer->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void ParticlesSpriteRendererNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_ParticlesSpriteRenderer)
	{
		m_ParticlesSpriteRenderer->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void ParticlesSpriteRendererNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void ParticlesSpriteRendererNode::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_ParticlesSpriteRenderer)
	{
		m_ParticlesSpriteRenderer->NeedResize(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResize(vNewSize, vCountColorBuffers);
}

void ParticlesSpriteRendererNode::SetTexelBuffer(const uint32_t& vBinding, vk::Buffer* vTexelBuffer, ct::uvec2* vTexelBufferSize)
{
	ZoneScoped;

	if (m_ParticlesSpriteRenderer)
	{
		return m_ParticlesSpriteRenderer->SetTexelBuffer(vBinding, vTexelBuffer, vTexelBufferSize);
	}
}

void ParticlesSpriteRendererNode::SetTexelBufferView(const uint32_t& vBinding, vk::BufferView* vTexelBufferView, ct::uvec2* vTexelBufferSize)
{
	ZoneScoped;

	if (m_ParticlesSpriteRenderer)
	{
		return m_ParticlesSpriteRenderer->SetTexelBufferView(vBinding, vTexelBufferView, vTexelBufferSize);
	}
}

vk::DescriptorImageInfo* ParticlesSpriteRendererNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	ZoneScoped;

	if (m_ParticlesSpriteRenderer)
	{
		return m_ParticlesSpriteRenderer->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return VK_NULL_HANDLE;
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void ParticlesSpriteRendererNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_ParticlesSpriteRenderer)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == "PARTICLES")
			{
				auto otherTextureNodePtr = dynamic_pointer_cast<TexelBufferOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherTextureNodePtr)
				{
					ct::uvec2 texelBufferSize;
					auto bufferPtr = otherTextureNodePtr->GetTexelBuffer(endSlotPtr->descriptorBinding, &texelBufferSize);
					SetTexelBuffer(startSlotPtr->descriptorBinding, bufferPtr, &texelBufferSize);
					auto bufferViewPtr = otherTextureNodePtr->GetTexelBufferView(endSlotPtr->descriptorBinding, &texelBufferSize);
					SetTexelBufferView(startSlotPtr->descriptorBinding, bufferViewPtr, &texelBufferSize);
				}
			}
		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void ParticlesSpriteRendererNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_ParticlesSpriteRenderer)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == "PARTICLES")
			{
				SetTexelBuffer(startSlotPtr->descriptorBinding, nullptr, nullptr);
				SetTexelBufferView(startSlotPtr->descriptorBinding, nullptr, nullptr);
			}
		}
	}
}

void ParticlesSpriteRendererNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::TexelBufferUpdateDone:
	{
		auto emiterSlotPtr = vEmitterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherNodePtr = dynamic_pointer_cast<TexelBufferOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					auto receiverSlotPtr = vReceiverSlot.getValidShared();
					if (receiverSlotPtr)
					{
						ct::uvec2 texelBufferSize;
						auto bufferPtr = otherNodePtr->GetTexelBuffer(emiterSlotPtr->descriptorBinding, &texelBufferSize);
						SetTexelBuffer(receiverSlotPtr->descriptorBinding, bufferPtr, &texelBufferSize);
						auto bufferViewPtr = otherNodePtr->GetTexelBufferView(emiterSlotPtr->descriptorBinding, &texelBufferSize);
						SetTexelBufferView(receiverSlotPtr->descriptorBinding, bufferViewPtr, &texelBufferSize);
					}
				}
			}
		}
		break;
	}
	/*
	case NotifyEvent::TextureUpdateDone:
	{
		auto slots = GetOutputSlotsOfType("TEXTURE_2D");
		for (const auto& slot : slots)
		{
			auto slotPtr = slot.getValidShared();
			if (slotPtr)
			{
				slotPtr->Notify(NotifyEvent::TextureUpdateDone, slot);
			}
		}
		break;
	}
	*/
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string ParticlesSpriteRendererNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_ParticlesSpriteRenderer)
		{
			res += m_ParticlesSpriteRenderer->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool ParticlesSpriteRendererNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_ParticlesSpriteRenderer)
	{
		m_ParticlesSpriteRenderer->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void ParticlesSpriteRendererNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_ParticlesSpriteRenderer)
	{
		m_ParticlesSpriteRenderer->UpdateShaders(vFiles);
	}
}
