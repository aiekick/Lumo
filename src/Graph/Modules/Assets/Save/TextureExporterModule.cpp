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

#include "TextureExporterModule.h"

#include <cinttypes>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <ImWidgets.h>
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

std::shared_ptr<TextureExporterModule> TextureExporterModule::Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode) {
    ZoneScoped;

    auto res = std::make_shared<TextureExporterModule>(vVulkanCore);
    res->SetParentNode(vParentNode);
    res->m_This = res;
    if (!res->Init()) {
        res.reset();
    }

    // to implement
    CTOOL_DEBUG_BREAK;

    return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

TextureExporterModule::TextureExporterModule(GaiApi::VulkanCoreWeak vVulkanCore) : m_VulkanCore(vVulkanCore) {
    ZoneScoped;
}

TextureExporterModule::~TextureExporterModule() {
    ZoneScoped;

    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool TextureExporterModule::Init() {
    ZoneScoped;

    return true;
}

void TextureExporterModule::Unit() {
    ZoneScoped;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool TextureExporterModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool TextureExporterModule::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool TextureExporterModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void TextureExporterModule::SetTexture(
    const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    ZoneScoped;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string TextureExporterModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;

    std::string str;

    str += vOffset + "<texture_exporter_module>\n";

    str += vOffset + "</texture_exporter_module>\n";

    return str;
}

bool TextureExporterModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (strParentName == "texture_exporter_module") {
    }

    return true;
}

void TextureExporterModule::AfterNodeXmlLoading() {
    ZoneScoped;
}
