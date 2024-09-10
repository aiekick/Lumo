#pragma once

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

#include <LumoBackend/Interfaces/PluginInterface.h>

#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Core/vk_mem_alloc.h>

class RTX : public PluginInterface {
public:
    RTX();
    bool AuthorizeLoading() override;
    void ActionAfterInit() override;
    uint32_t GetVersionMajor() const override;
    uint32_t GetVersionMinor() const override;
    uint32_t GetVersionBuild() const override;
    std::string GetName() const override;
    std::string GetVersion() const override;
    std::string GetDescription() const override;
    std::vector<std::string> GetNodes() const override;
    std::vector<LibraryEntry> GetLibrary() const override;
    BaseNodePtr CreatePluginNode(const std::string& vPluginNodeName) override;
    std::vector<PluginPaneConfig> GetPanes() const override;
    int ResetImGuiID(const int& vWidgetId) override;
};