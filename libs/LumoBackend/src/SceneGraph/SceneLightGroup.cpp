// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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

#include <LumoBackend/SceneGraph/SceneLightGroup.h>

///////////////////////////////////////////////////////
//// STATIC ///////////////////////////////////////////
///////////////////////////////////////////////////////

SceneLightGroupPtr SceneLightGroup::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res = std::make_shared<SceneLightGroup>();
    res->m_This = res;
    if (!res->Init(vVulkanCore)) {
        res.reset();
    }
    return res;
}

///////////////////////////////////////////////////////
//// STATIC : SBO//////////////////////////////////////
///////////////////////////////////////////////////////

std::string SceneLightGroup::GetBufferObjectStructureHeader(const uint32_t& vBindingPoint) {
    return ct::toStr(
        u8R"(%s

layout(std430, binding = %u) readonly buffer SBO_LightGroup
{
	uint lightsCount;
	LightDatas lightDatas[];
};
)",
        SceneLight::GetStructureHeader().c_str(), vBindingPoint);
}

// will create a empty sbo for default sbo when no slot are connected
VulkanBufferObjectPtr SceneLightGroup::CreateEmptyBuffer(GaiApi::VulkanCoreWeak vVulkanCore) {
    ZoneScoped;

    auto size_in_bytes = sizeof(uint32_t) + sizeof(SceneLight::lightDatas);
    // gpu only since no udpate will be done
    return GaiApi::VulkanRessource::createStorageBufferObject(
        vVulkanCore, size_in_bytes, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY, "SceneLightGroup");
}

///////////////////////////////////////////////////////
//// PUBLIC : CTOR / DTOR /////////////////////////////
///////////////////////////////////////////////////////

SceneLightGroup::SceneLightGroup() {
}

SceneLightGroup::~SceneLightGroup() {
    Unit();
}

///////////////////////////////////////////////////////
//// PUBLIC : INIT / UNIT /////////////////////////////
///////////////////////////////////////////////////////

bool SceneLightGroup::Init(GaiApi::VulkanCoreWeak vVulkanCore) {
    m_VulkanCore = vVulkanCore;

    m_SBO430.RegisterVar("LightsCount", m_LightsCount);

    if (empty()) {
        Add();
    }

    return CreateBufferObject(m_VulkanCore);
}

void SceneLightGroup::Unit() {
    DestroyBufferObject();
}

///////////////////////////////////////////////////////
//// PUBLIC ///////////////////////////////////////////
///////////////////////////////////////////////////////

std::vector<SceneLightPtr>::iterator SceneLightGroup::begin() {
    ZoneScoped;

    return m_Lights.begin();
}

std::vector<SceneLightPtr>::iterator SceneLightGroup::end() {
    ZoneScoped;

    return m_Lights.end();
}

size_t SceneLightGroup::size() {
    ZoneScoped;

    return m_Lights.size();
}

void SceneLightGroup::clear() {
    ZoneScoped;

    m_Lights.clear();
}

bool SceneLightGroup::empty() {
    ZoneScoped;

    return m_Lights.empty();
}

SceneLightWeak SceneLightGroup::Add() {
    ZoneScoped;

    uint32_t idx = (uint32_t)m_Lights.size();

    if (idx <= sMaxLightCount) {
        auto lightPtr = SceneLight::Create();
        if (lightPtr) {
            lightPtr->gizmo_name = ct::toStr("Light %u", idx);

            m_SBO430.RegisterVar(ct::toStr("lightDatas_%u", idx), lightPtr->lightDatas);

            m_Lights.push_back(lightPtr);

            m_LightsCount = (uint32_t)m_Lights.size();
            m_SBO430.SetVar("LightsCount", m_LightsCount);

            return Get(idx);
        }
    }

    return SceneLightWeak();
}

void SceneLightGroup::erase(uint32_t vIndex) {
    ZoneScoped;

    if ((size_t)vIndex < m_Lights.size()) {
        m_Lights.erase(m_Lights.begin() + vIndex);

        m_SBO430.Clear();

        m_LightsCount = 0U;
        m_SBO430.RegisterVar("LightsCount", m_LightsCount);

        uint32_t idx = 0U;
        for (auto lightPtr : m_Lights) {
            if (lightPtr) {
                m_SBO430.RegisterVar(ct::toStr("lightDatas_%u", idx++), lightPtr->lightDatas);
            }
        }

        m_LightsCount = (uint32_t)m_Lights.size();
        m_SBO430.SetVar("LightsCount", m_LightsCount);
    }
}

SceneLightWeak SceneLightGroup::Get(const size_t& vIndex) {
    ZoneScoped;

    if (m_Lights.size() > (size_t)vIndex) {
        return m_Lights[(size_t)vIndex];
    }

    return SceneLightWeak();
}

bool SceneLightGroup::CanAddLight() const {
    return (m_Lights.size() <= sMaxLightCount);
}

bool SceneLightGroup::CanRemoveLight() const {
    return (m_Lights.size() > 1U);
}

///////////////////////////////////////////////////////
//// SBO //////////////////////////////////////////////
///////////////////////////////////////////////////////

bool SceneLightGroup::IsOk() {
    return false;
}

void SceneLightGroup::UploadBufferObjectIfDirty(GaiApi::VulkanCoreWeak vVulkanCore) {
    ZoneScoped;

    m_SBO430.Upload(vVulkanCore, true);
}

bool SceneLightGroup::CreateBufferObject(GaiApi::VulkanCoreWeak vVulkanCore) {
    ZoneScoped;

    if (!m_Lights.empty()) {
        auto corePtr = vVulkanCore.lock();
        assert(corePtr != nullptr);

        corePtr->getDevice().waitIdle();

        return m_SBO430.CreateSBO(vVulkanCore, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU);
    }

    return false;
}

void SceneLightGroup::DestroyBufferObject() {
    ZoneScoped;

    m_SBO430.DestroySBO();
}

vk::DescriptorBufferInfo* SceneLightGroup::GetBufferInfo() {
    if (m_SBO430.IsOk()) {
        return &m_SBO430.descriptorBufferInfo;
    }
    return nullptr;
}
