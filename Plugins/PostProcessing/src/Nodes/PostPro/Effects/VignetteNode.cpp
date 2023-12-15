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

#include "VignetteNode.h"
#include <Modules/PostPro/Effects/VignetteModule.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexture2DInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexture2DOutput.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<VignetteNode> VignetteNode::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    ZoneScoped;

    auto res = std::make_shared<VignetteNode>();
    res->m_This = res;
    if (!res->Init(vVulkanCore)) {
        res.reset();
    }
    return res;
}

VignetteNode::VignetteNode() : BaseNode() {
    ZoneScoped;

    m_NodeTypeString = "VIGNETTE";
}

VignetteNode::~VignetteNode() {
    ZoneScoped;

    Unit();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool VignetteNode::Init(GaiApi::VulkanCoreWeak vVulkanCore) {
    ZoneScoped;

    bool res = false;

    name = "Vignette";
    AddInput(NodeSlotTexture2DInput::Create("", 0), false, true);

    AddOutput(NodeSlotTexture2DOutput::Create("", 0), false, true);

    m_VignetteModulePtr = VignetteModule::Create(vVulkanCore, m_This);
    if (m_VignetteModulePtr) {
        res = true;
    }

    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TASK EXECUTE ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool VignetteNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    ZoneScoped;

    bool res = false;

    BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

    // for update input texture buffer infos => avoid vk crash
    UpdateTexture2DInputDescriptorImageInfos(m_Inputs);
    if (m_VignetteModulePtr) {
        res = m_VignetteModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
    }

    return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool VignetteNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    bool res = false;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_VignetteModulePtr) {
        res = m_VignetteModulePtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
    }

    return res;
}

bool VignetteNode::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool VignetteNode::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_VignetteModulePtr) {
        return m_VignetteModulePtr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW NODE ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void VignetteNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) {
    ZoneScoped;

    if (vBaseNodeState && vBaseNodeState->debug_mode) {
        auto drawList = nd::GetNodeBackgroundDrawList(nodeID);
        if (drawList) {
            char debugBuffer[255] = "\0";
            snprintf(debugBuffer, 254, "Used[%s]\nCell[%i, %i]", (used ? "true" : "false"), cell.x, cell.y);
            ImVec2 txtSize = ImGui::CalcTextSize(debugBuffer);
            drawList->AddText(pos - ImVec2(0, txtSize.y), ImGui::GetColorU32(ImGuiCol_Text), debugBuffer);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// RESIZE //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void VignetteNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    ZoneScoped;
    if (m_VignetteModulePtr) {
        m_VignetteModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
    }

    // on fait ca apres
    BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void VignetteNode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    ZoneScoped;
    if (m_VignetteModulePtr) {
        m_VignetteModulePtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize, vUserDatas);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* VignetteNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    ZoneScoped;
    if (m_VignetteModulePtr) {
        return m_VignetteModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize, vUserDatas);
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string VignetteNode::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;

    std::string res;

    if (!m_ChildNodes.empty()) {
        res += BaseNode::getXml(vOffset, vUserDatas);
    } else {
        res += vOffset + ct::toStr("<node name=\"%s\" type=\"%s\" pos=\"%s\" id=\"%u\">\n", name.c_str(), m_NodeTypeString.c_str(),
                             ct::fvec2(pos.x, pos.y).string().c_str(), (uint32_t)GetNodeID());

        for (auto slot : m_Inputs) {
            res += slot.second->getXml(vOffset + "\t", vUserDatas);
        }

        for (auto slot : m_Outputs) {
            res += slot.second->getXml(vOffset + "\t", vUserDatas);
        }

        if (m_VignetteModulePtr) {
            res += m_VignetteModulePtr->getXml(vOffset + "\t", vUserDatas);
        }

        res += vOffset + "</node>\n";
    }

    return res;
}

bool VignetteNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (m_VignetteModulePtr) {
        m_VignetteModulePtr->setFromXml(vElem, vParent, vUserDatas);
    }

    // continue recurse child exploring
    return true;
}

void VignetteNode::AfterNodeXmlLoading() {
    if (m_VignetteModulePtr) {
        m_VignetteModulePtr->AfterNodeXmlLoading();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SHADER UPDATE ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void VignetteNode::UpdateShaders(const std::set<std::string>& vFiles) {
    ZoneScoped;
    if (m_VignetteModulePtr) {
        m_VignetteModulePtr->UpdateShaders(vFiles);
    }
}
