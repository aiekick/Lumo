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

#include "MeshEmitterNode.h"
#include <Modules/Emitters/MeshEmitterModule.h>
#include <LumoBackend/Graph/Slots/NodeSlotModelInput.h>
#include <Slots/NodeSlotParticlesOutput.h>

std::shared_ptr<MeshEmitterNode> MeshEmitterNode::Create(GaiApi::VulkanCorePtr vVulkanCorePtr) {
    auto res = std::make_shared<MeshEmitterNode>();
    res->m_This = res;
    if (!res->Init(vVulkanCorePtr)) {
        res.reset();
    }
    return res;
}

MeshEmitterNode::MeshEmitterNode() : BaseNode() {
    ZoneScoped;

    m_NodeTypeString = "PARTICLES_MESH_EMITTER";
}

MeshEmitterNode::~MeshEmitterNode() {
    ZoneScoped;

    Unit();
}

bool MeshEmitterNode::Init(GaiApi::VulkanCorePtr vVulkanCorePtr) {
    ZoneScoped;

    name = "Mesh Emitter";

    AddInput(NodeSlotModelInput::Create("Model"), true, true);
    AddOutput(NodeSlotParticlesOutput::Create("Particles"), true, true);

    m_MeshEmitterModulePtr = MeshEmitterModule::Create(vVulkanCorePtr, m_This);
    if (m_MeshEmitterModulePtr) {
        return true;
    }

    return false;
}

bool MeshEmitterNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    ZoneScoped;

    BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

    if (m_MeshEmitterModulePtr) {
        return m_MeshEmitterModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
    }
    return false;
}

bool MeshEmitterNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_MeshEmitterModulePtr) {
        return m_MeshEmitterModulePtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
    }

    return false;
}

bool MeshEmitterNode::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_MeshEmitterModulePtr) {
        return m_MeshEmitterModulePtr->DrawOverlays(vCurrentFrame, vRect, vContextPtr, vUserDatas);
    }
    return false;
}

bool MeshEmitterNode::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_MeshEmitterModulePtr) {
        return m_MeshEmitterModulePtr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
    }
    return false;
}

void MeshEmitterNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) {
    ZoneScoped;

    if (vBaseNodeState && vBaseNodeState->debug_mode) {
        auto drawList = nd::GetNodeBackgroundDrawList(nodeID);
        if (drawList) {
            char debugBuffer[255] = "\0";
            snprintf(debugBuffer, 254, "Used(%s)\nCell(%i, %i)" /*\nPos(%.1f, %.1f)\nSize(%.1f, %.1f)*/, (used ? "true" : "false"), cell.x, cell.y /*, pos.x, pos.y, size.x, size.y*/);
            ImVec2 txtSize = ImGui::CalcTextSize(debugBuffer);
            drawList->AddText(pos - ImVec2(0, txtSize.y), ImGui::GetColorU32(ImGuiCol_Text), debugBuffer);
        }
    }
}

void MeshEmitterNode::SetModel(SceneModelWeak vSceneModel) {
    ZoneScoped;

    if (m_MeshEmitterModulePtr) {
        m_MeshEmitterModulePtr->SetModel(vSceneModel);
    }
}

SceneParticlesWeak MeshEmitterNode::GetParticles() {
    ZoneScoped;

    if (m_MeshEmitterModulePtr) {
        return m_MeshEmitterModulePtr->GetParticles();
    }

    return SceneParticlesWeak();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string MeshEmitterNode::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string res;

    if (!m_ChildNodes.empty()) {
        res += BaseNode::getXml(vOffset, vUserDatas);
    } else {
        res += vOffset + ct::toStr("<node name=\"%s\" type=\"%s\" pos=\"%s\" id=\"%u\">\n", name.c_str(), m_NodeTypeString.c_str(), ct::fvec2(pos.x, pos.y).string().c_str(), (uint32_t)nodeID.Get());

        for (auto slot : m_Inputs) {
            res += slot.second->getXml(vOffset + "\t", vUserDatas);
        }

        for (auto slot : m_Outputs) {
            res += slot.second->getXml(vOffset + "\t", vUserDatas);
        }

        if (m_MeshEmitterModulePtr) {
            res += m_MeshEmitterModulePtr->getXml(vOffset + "\t", vUserDatas);
        }

        res += vOffset + "</node>\n";
    }

    return res;
}

bool MeshEmitterNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (m_MeshEmitterModulePtr) {
        m_MeshEmitterModulePtr->setFromXml(vElem, vParent, vUserDatas);
    }

    return true;
}

void MeshEmitterNode::UpdateShaders(const std::set<std::string>& vFiles) {
    if (m_MeshEmitterModulePtr) {
        m_MeshEmitterModulePtr->UpdateShaders(vFiles);
    }
}
