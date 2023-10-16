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

#include "NodeSlotSceneAudiArtOutput.h"

#include <utility>
#include <SceneGraph/SceneAudiArt.h>
#include <LumoBackend/Graph/Base/BaseNode.h>

static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotSceneAudiArtOutputPtr NodeSlotSceneAudiArtOutput::Create(NodeSlotSceneAudiArtOutput vSlot)
{
	auto res = std::make_shared<NodeSlotSceneAudiArtOutput>(vSlot);
	res->m_This = res;
	return res;
}

NodeSlotSceneAudiArtOutputPtr NodeSlotSceneAudiArtOutput::Create(const std::string& vName)
{
	auto res = std::make_shared<NodeSlotSceneAudiArtOutput>(vName);
	res->m_This = res;
	return res;
}

NodeSlotSceneAudiArtOutputPtr NodeSlotSceneAudiArtOutput::Create(const std::string& vName, const bool& vHideName)
{
	auto res = std::make_shared<NodeSlotSceneAudiArtOutput>(vName, vHideName);
	res->m_This = res;
	return res;
}

NodeSlotSceneAudiArtOutputPtr NodeSlotSceneAudiArtOutput::Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
{
	auto res = std::make_shared<NodeSlotSceneAudiArtOutput>(vName, vHideName, vShowWidget);
	res->m_This = res;
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// PUBIC CLASS /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotSceneAudiArtOutput::NodeSlotSceneAudiArtOutput()
	: NodeSlotOutput("", "SCENEAUDIART")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotSceneAudiArtOutput::NodeSlotSceneAudiArtOutput(const std::string& vName)
	: NodeSlotOutput(vName, "SCENEAUDIART")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotSceneAudiArtOutput::NodeSlotSceneAudiArtOutput(const std::string& vName, const bool& vHideName)
	: NodeSlotOutput(vName, "SCENEAUDIART", vHideName)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotSceneAudiArtOutput::NodeSlotSceneAudiArtOutput(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
	: NodeSlotOutput(vName, "SCENEAUDIART", vHideName, vShowWidget)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotSceneAudiArtOutput::~NodeSlotSceneAudiArtOutput() = default;

void NodeSlotSceneAudiArtOutput::Init()
{
	
}

void NodeSlotSceneAudiArtOutput::Unit()
{
	// ici pas besoin du assert sur le m_This 
	// car NodeSlotSceneAudiArtOutput peut etre instancié à l'ancienne en copie local donc sans shared_ptr
	// donc pour gagner du temps on va checker le this, si expiré on va pas plus loins
	if (!m_This.expired())
	{
		if (!parentNode.expired())
		{
			auto parentNodePtr = parentNode.lock();
			if (parentNodePtr)
			{
				auto graph = parentNodePtr->GetParentNode();
				if (!graph.expired())
				{
					auto graphPtr = graph.lock();
					if (graphPtr)
					{
						graphPtr->BreakAllLinksConnectedToSlot(m_This);
					}
				}
			}
		}
	}
}

void NodeSlotSceneAudiArtOutput::SendFrontNotification(const NotifyEvent& vEvent)
{
	if (vEvent == SceneAudiArtUpdateDone)
	{
		SendNotification(slotType, vEvent);
	}
}

void NodeSlotSceneAudiArtOutput::DrawDebugInfos()
{
	ImGui::Text("--------------------");
	ImGui::Text("Slot SceneAudiArt", name.c_str());
	ImGui::Text(IsAnInput() ? "Input" : "Output");
	ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
