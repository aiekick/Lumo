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

#include "CubeMapNode.h"
#include <Graph/Modules/Assets/Load/CubeMapModule.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureCubeOutput.h>

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

std::shared_ptr<CubeMapNode> CubeMapNode::Create(GaiApi::VulkanCorePtr vVulkanCorePtr) {
    ZoneScoped;

    auto res = std::make_shared<CubeMapNode>();
    res->m_This = res;
    if (!res->Init(vVulkanCorePtr)) {
        res.reset();
    }
    return res;
}

CubeMapNode::CubeMapNode() : BaseNode() {
    ZoneScoped;

    m_NodeTypeString = "CUBE_MAP";
}

CubeMapNode::~CubeMapNode() {
    ZoneScoped;

    Unit();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool CubeMapNode::Init(GaiApi::VulkanCorePtr vVulkanCorePtr) {
    ZoneScoped;

    bool res = false;

    name = "CubeMap";

    auto ptr = NodeSlotTextureCubeOutput::Create("New Slot", 0);
    ptr->showWidget = true;
    AddOutput(ptr, false, true);

    m_CubeMapModulePtr = CubeMapModule::Create(vVulkanCorePtr, m_This);
    if (m_CubeMapModulePtr) {
        res = true;
    }

    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool CubeMapNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    bool res = false;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_CubeMapModulePtr) {
        res = m_CubeMapModulePtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
    }

    return res;
}

bool CubeMapNode::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool CubeMapNode::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    if (m_CubeMapModulePtr) {
        return m_CubeMapModulePtr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW SLOTS WIDGET ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void CubeMapNode::DrawOutputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot) {
    auto slotPtr = vSlot.lock();
    if (slotPtr && slotPtr->showWidget) {
        if (m_CubeMapModulePtr) {
            m_CubeMapModulePtr->DrawTextures(ct::ivec2(100, 75));
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW NODE ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void CubeMapNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) {
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
//// TEXTURE CUBE SLOT OUTPUT ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* CubeMapNode::GetTextureCube(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {
    ZoneScoped;
    if (m_CubeMapModulePtr) {
        return m_CubeMapModulePtr->GetTextureCube(vBindingPoint, vOutSize);
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string CubeMapNode::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;

    std::string res;

    if (!m_ChildNodes.empty()) {
        res += BaseNode::getXml(vOffset, vUserDatas);
    } else {
        res += vOffset + ct::toStr("<node name=\"%s\" type=\"%s\" pos=\"%s\" id=\"%u\">\n", name.c_str(),
                             m_NodeTypeString.c_str(), ct::fvec2(pos.x, pos.y).string().c_str(), (uint32_t)GetNodeID());

        for (auto slot : m_Inputs) {
            res += slot.second->getXml(vOffset + "\t", vUserDatas);
        }

        for (auto slot : m_Outputs) {
            res += slot.second->getXml(vOffset + "\t", vUserDatas);
        }

        if (m_CubeMapModulePtr) {
            res += m_CubeMapModulePtr->getXml(vOffset + "\t", vUserDatas);
        }

        res += vOffset + "</node>\n";
    }

    return res;
}

bool CubeMapNode::setFromXml(
    tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (m_CubeMapModulePtr) {
        m_CubeMapModulePtr->setFromXml(vElem, vParent, vUserDatas);
    }

    // continue recurse child exploring
    return true;
}
