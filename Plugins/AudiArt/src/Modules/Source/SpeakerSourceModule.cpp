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

#include "SpeakerSourceModule.h"

#include <cinttypes>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <ImGuiPack.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Buffer/FrameBuffer.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<SpeakerSourceModule> SpeakerSourceModule::Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode) {
    ZoneScoped;

    auto res = std::make_shared<SpeakerSourceModule>(vVulkanCore);
    res->SetParentNode(vParentNode);
    res->m_This = res;
    if (!res->Init()) {
        res.reset();
    }

    return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

SpeakerSourceModule::SpeakerSourceModule(GaiApi::VulkanCoreWeak vVulkanCore) : m_VulkanCore(vVulkanCore) {
    ZoneScoped;
}

SpeakerSourceModule::~SpeakerSourceModule() {
    ZoneScoped;

    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SpeakerSourceModule::Init() {
    ZoneScoped;

    return true;
}

void SpeakerSourceModule::Unit() {
    ZoneScoped;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SpeakerSourceModule::ExecuteAllTime(const uint32_t& /*vCurrentFrame*/, vk::CommandBuffer* /*vCmd*/, BaseNodeState* /*vBaseNodeState*/) {
    ZoneScoped;

    return true;
}

bool SpeakerSourceModule::ExecuteWhenNeeded(const uint32_t& /*vCurrentFrame*/, vk::CommandBuffer* /*vCmd*/, BaseNodeState* /*vBaseNodeState*/) {
    ZoneScoped;

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool SpeakerSourceModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool SpeakerSourceModule::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool SpeakerSourceModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SCENEAUDIART OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneAudiArtWeak SpeakerSourceModule::GetSceneAudiArt(const std::string& vName) {
    ZoneScoped;
    return SceneAudiArtWeak();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SpeakerSourceModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;

    std::string str;

    str += vOffset + "<speaker_source_module>\n";

    str += vOffset + "</speaker_source_module>\n";

    return str;
}

bool SpeakerSourceModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (strParentName == "speaker_source_module") {
    }

    return true;
}

void SpeakerSourceModule::AfterNodeXmlLoading() {
    ZoneScoped;
}
