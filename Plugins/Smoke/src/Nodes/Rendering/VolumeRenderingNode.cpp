/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "VolumeRenderingNode.h"
#include <Modules/Rendering/VolumeRenderingModule.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexture3DInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexture2DOutput.h>

#ifdef PROFILER_INCLUDE
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<VolumeRenderingNode> VolumeRenderingNode::Create(GaiApi::VulkanCoreWeak vVulkanCore)
{
	ZoneScoped;

	auto res = std::make_shared<VolumeRenderingNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCore))
	{
		res.reset();
	}

	return res;
}

VolumeRenderingNode::VolumeRenderingNode() : BaseNode()
{
	ZoneScoped;

	m_NodeTypeString = "VOLUME_RENDERING";
}

VolumeRenderingNode::~VolumeRenderingNode()
{
	ZoneScoped;

	Unit();
}		

//////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool VolumeRenderingNode::Init(GaiApi::VulkanCoreWeak vVulkanCore)
{
	ZoneScoped;

	bool res = false;

	name = "Volume Rendering";

	AddInput(NodeSlotTexture3DInput::Create("Density", 0), false, false);

	AddOutput(NodeSlotTexture2DOutput::Create("", 0), false, true);

	m_VolumeRenderingModulePtr = VolumeRenderingModule::Create(vVulkanCore, m_This);
	if (m_VolumeRenderingModulePtr)
	{
		res = true;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TASK EXECUTE ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool VolumeRenderingNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	bool res = false;

	BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTexture3DInputDescriptorImageInfos(m_Inputs);
	if (m_VolumeRenderingModulePtr)
	{
		res = m_VolumeRenderingModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}

	return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool VolumeRenderingNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	ZoneScoped;
	bool res = false;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	if (m_VolumeRenderingModulePtr)	{
		res = m_VolumeRenderingModulePtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
	}
	return res;
}

bool VolumeRenderingNode::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	if (m_LastExecutedFrame == vCurrentFrame) {
		return m_VolumeRenderingModulePtr->DrawOverlays(vCurrentFrame, vRect, vContextPtr, vUserDatas);
	}
	return false;
}

bool VolumeRenderingNode::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	if (m_VolumeRenderingModulePtr)	{
		return m_VolumeRenderingModulePtr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW NODE ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void VolumeRenderingNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	if (vBaseNodeState && vBaseNodeState->debug_mode) {
		auto drawList = nd::GetNodeBackgroundDrawList(nodeID);
		if (drawList) {
			char debugBuffer[255] = "\0";
			snprintf(debugBuffer, 254,
				"Used[%s]\nCell[%i, %i]",
				(used ? "true" : "false"), cell.x, cell.y);
			ImVec2 txtSize = ImGui::CalcTextSize(debugBuffer);
			drawList->AddText(pos - ImVec2(0, txtSize.y), ImGui::GetColorU32(ImGuiCol_Text), debugBuffer);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// RESIZE //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void VolumeRenderingNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	if (m_VolumeRenderingModulePtr)	{
		m_VolumeRenderingModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE 3D SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void VolumeRenderingNode::SetTexture3D(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec3* vTextureSize) {	
	ZoneScoped;

	if (m_VolumeRenderingModulePtr)	{
		m_VolumeRenderingModulePtr->SetTexture3D(vBindingPoint, vImageInfo, vTextureSize);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* VolumeRenderingNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {	
	ZoneScoped;

	if (m_VolumeRenderingModulePtr)	{
		return m_VolumeRenderingModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize, vUserDatas);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string VolumeRenderingNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
{	
	ZoneScoped;

	std::string res;

	if (!m_ChildNodes.empty()) {
		res += BaseNode::getXml(vOffset, vUserDatas);
	} else {
		res += vOffset + ct::toStr("<node name=\"%s\" type=\"%s\" pos=\"%s\" id=\"%u\">\n",
			name.c_str(),
			m_NodeTypeString.c_str(),
			ct::fvec2(pos.x, pos.y).string().c_str(),
			(uint32_t)GetNodeID());

		for (auto slot : m_Inputs) {
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		for (auto slot : m_Outputs) {
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		if (m_VolumeRenderingModulePtr)	{
			res += m_VolumeRenderingModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool VolumeRenderingNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{	
	ZoneScoped;

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

	if (m_VolumeRenderingModulePtr)	{
		m_VolumeRenderingModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	// continue recurse child exploring
	return true;
}

void VolumeRenderingNode::AfterNodeXmlLoading()
{
	ZoneScoped;

	if (m_VolumeRenderingModulePtr)	{
		m_VolumeRenderingModulePtr->AfterNodeXmlLoading();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SHADER UPDATE ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void VolumeRenderingNode::UpdateShaders(const std::set<std::string>& vFiles)
{	
	ZoneScoped;

	if (m_VolumeRenderingModulePtr)	{
		m_VolumeRenderingModulePtr->UpdateShaders(vFiles);
	}
}
