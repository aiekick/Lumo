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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <LumoBackend/Graph/Library/LibraryCategory.h>

#include <imgui.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <ctools/FileHelper.h>

#include <stdio.h>
#include <string.h>

#include <functional>

// for directory/files lister
#if defined(__WIN32__) || defined(_WIN32)
#include <dirent/dirent.h>
#define PATH_SEP '\\'
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#include <dirent.h>
#define PATH_SEP '/'
#endif

#define LIBRARY_EFECT_ROOT_PATH "NodeLibrary"

//////////////////////////////////////////////////////////////////////////////////////////////
//// INIT ////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

LibraryCategory::LibraryCategory() = default;

LibraryCategory::~LibraryCategory() = default;

void LibraryCategory::Clear() {
    m_CategoryName.clear();
    m_LibraryItems.clear();
    m_SubCategories.clear();
}

LibraryCategory* LibraryCategory::AddCategory(const std::string& vCategoryName) {
    if (vCategoryName.empty())
        return 0;

    m_SubCategories[vCategoryName].m_CategoryName = vCategoryName;

    return &m_SubCategories[vCategoryName];
}

void LibraryCategory::AddShader(const std::string& vShaderName, const std::string& vShaderPath) {
    if (vShaderName.empty())
        return;
    if (vShaderPath.empty())
        return;

    auto ps = FileHelper::Instance()->ParsePathFileName(vShaderName);
    if (ps.isOk) {
        LibraryEntry entry;

        if (ps.ext == "comp")
            entry.first = "compute";
        if (ps.ext == "frag")
            entry.first = "fragment";
        if (ps.ext == "scen")
            entry.first = "scene";

        if (!entry.first.empty()) {
            entry.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_SHADER;
            entry.second.shaderpath = vShaderPath;
            m_LibraryItems[ps.name] = entry;
        }
    }
}

void LibraryCategory::AddCustom(
    const std::string& vCategoryPath, const std::string& vNodeLabel, const std::string& vNodeType, const ct::fvec4& vColor) {
    if (vNodeLabel.empty()) {
        return;
    }
    if (vCategoryPath.empty()) {
        LibraryEntry entry;
        entry.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_INTERNAL;
        entry.first = "internal";
        entry.second.nodeLabel = vNodeLabel;
        entry.second.nodeType = vNodeType;
        entry.second.color = vColor;
        m_LibraryItems[vNodeLabel] = entry;
    } else {
        auto vec = ct::splitStringToVector(vCategoryPath, '/');
        if (!vec.empty()) {
            auto cat = this;
            for (auto word : vec) {
                if (cat->m_SubCategories.find(word) != cat->m_SubCategories.end()) {
                    cat = &cat->m_SubCategories[word];
                } else {
                    cat = cat->AddCategory(word);
                }
            }

            LibraryEntry entry;
            entry.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_INTERNAL;
            entry.first = "internal";
            entry.second.nodeLabel = vNodeLabel;
            entry.second.nodeType = vNodeType;
            entry.second.color = vColor;
            cat->m_LibraryItems[vNodeLabel] = entry;
        }
    }
}

void LibraryCategory::AddLibraryEntry(const LibraryEntry& vLibraryEntry) {
    if (vLibraryEntry.second.nodeLabel.empty())
        return;

    auto vec = ct::splitStringToVector(vLibraryEntry.second.categoryPath, '/');
    if (!vec.empty()) {
        auto cat = this;
        for (auto word : vec) {
            if (cat->m_SubCategories.find(word) != cat->m_SubCategories.end()) {
                cat = &cat->m_SubCategories[word];
            } else {
                cat = cat->AddCategory(word);
            }
        }

        cat->m_LibraryItems[vLibraryEntry.second.nodeLabel] = vLibraryEntry;
    }
}

LibraryEntry LibraryCategory::ShowMenu(BaseNodeWeak vNodeGraph, BaseNodeState* vBaseNodeState, int vLevel) {
    LibraryEntry entry;

    ImGui::SetNextWindowViewport(ImGui::GetWindowViewport()->ID);

    if (ImGui::BeginMenu(m_CategoryName.c_str())) {
        auto ent = ShowContent(vNodeGraph, vBaseNodeState, vLevel + 1);
        if (!ent.first.empty()) {
            entry = ent;
        }

        ImGui::EndMenu();
    }

    return entry;
}

LibraryEntry LibraryCategory::ShowContent(BaseNodeWeak vNodeGraph, BaseNodeState* vBaseNodeState, int vLevel) {
    LibraryEntry entry;

    if (vBaseNodeState && vLevel == 0) {
        if (!vBaseNodeState->linkFromSlot.expired()) {
            ImGui::SetNextWindowViewport(ImGui::GetWindowViewport()->ID);

            if (ImGui::MenuItem("extract")) {
                auto slotPtr = vBaseNodeState->linkFromSlot.lock();
                if (slotPtr) {
                    entry.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_INTERNAL;
                    entry.first = "internal";
                    entry.second.nodeLabel = "extract";
                    entry.second.nodeType = "extract";
                    entry.second.nodeSlot = *slotPtr;
                }
            }

            ImGui::Separator();
        }
    }

    for (auto& category : m_SubCategories) {
        auto ent = category.second.ShowMenu(vNodeGraph, vBaseNodeState, vLevel + 1);
        if (!ent.first.empty()) {
            entry = ent;
        }
    }

    if (!m_LibraryItems.empty() && !m_SubCategories.empty()) {
        ImGui::Separator();
    }

    ImGui::SetNextWindowViewport(ImGui::GetWindowViewport()->ID);

    for (auto& item : m_LibraryItems) {
        if (ImGui::MenuItem(item.first.c_str())) {
            entry = item.second;

            break;
        }
    }

    return entry;
}
