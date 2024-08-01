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

#pragma once
#pragma warning(disable : 4251)

#include <memory>
#include <string>
#include <vector>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>
#include <LumoBackend/Interfaces/ProjectInterface.h>
#include <LumoBackend/Graph/Library/LibraryCategory.h>

#include <Gaia/Core/VulkanCore.h>

class PluginInterface;
typedef std::weak_ptr<PluginInterface> PluginInterfaceWeak;
typedef std::shared_ptr<PluginInterface> PluginInterfacePtr;

class LUMO_BACKEND_API PluginPane : public AbstractPane, public ProjectInterface {
public:
    AbstractPaneWeak paneWeak;

private:
    ProjectInterface* m_ProjectInstancePtr = nullptr;

public:
    void SetProjectInstancePtr(ProjectInterface* vProjectInstancePtr) {
        m_ProjectInstancePtr = vProjectInstancePtr;
    }
    bool IsProjectLoaded() const override {
        if (m_ProjectInstancePtr != nullptr) {
            return m_ProjectInstancePtr->IsProjectLoaded();
        }
        return false;
    }
    bool IsProjectNeverSaved() const override {
        if (m_ProjectInstancePtr != nullptr) {
            return m_ProjectInstancePtr->IsProjectNeverSaved();
        }
        return false;
    }
    bool IsThereAnyProjectChanges() const override {
        if (m_ProjectInstancePtr != nullptr) {
            return m_ProjectInstancePtr->IsThereAnyProjectChanges();
        }
        return false;
    }
    void SetProjectChange(bool vChange = true) override {
        if (m_ProjectInstancePtr != nullptr) {
            m_ProjectInstancePtr->SetProjectChange(vChange);
        }
    }
    bool WasJustSaved() override {
        if (m_ProjectInstancePtr != nullptr) {
            return m_ProjectInstancePtr->WasJustSaved();
        }
        return false;
    }
};

class LUMO_BACKEND_API PluginPaneConfig {
public:
    AbstractPaneWeak paneWeak;
    std::string paneName;
    std::string paneCategory;
    float paneRatio = 0.0f;
    PaneDisposal paneDisposal = "CENTRAL";
    bool isPaneOpenedDefault = false;
    bool isPaneFocusedDefault = false;

public:
    PluginPaneConfig(AbstractPaneWeak vPaneWeak,
        std::string vName,
        std::string vPaneCategory,
        PaneDisposal vPaneDisposal,
        float vPaneRatio,
        bool vIsOpenedDefault,
        bool vIsFocusedDefault)
        : paneWeak(vPaneWeak),
          paneName(vName),
          paneCategory(vPaneCategory),
          paneRatio(vPaneRatio),
          paneDisposal(vPaneDisposal),
          isPaneOpenedDefault(vIsOpenedDefault),
          isPaneFocusedDefault(vIsFocusedDefault) {
    }
};

class LUMO_BACKEND_API PluginInterface : public conf::ConfigAbstract {
protected:
    GaiApi::VulkanCoreWeak m_VulkanCoreWeak;
    bool iSinAPlugin = false;

protected:
    LibraryEntry AddLibraryEntry(
        const std::string& vCategoryPath, const std::string& vNodeLabel, const std::string& vNodeType, const ct::fvec4& vColor = 0.0f) const;

public:
    PluginInterface() = default;
    virtual ~PluginInterface();

    virtual bool Init(GaiApi::VulkanCoreWeak vVulkanCoreWeak);
    virtual void Unit();

    // if false, will prevent plugin loading, ex if a feature is not available
    virtual bool AuthorizeLoading();

    virtual void ActionBeforeInit();
    virtual void ActionAfterInit();

    virtual uint32_t GetVersionMajor() const = 0;
    virtual uint32_t GetVersionMinor() const = 0;
    virtual uint32_t GetVersionBuild() const = 0;
    virtual std::string GetName() const = 0;
    virtual std::string GetVersion() const = 0;
    virtual std::string GetDescription() const = 0;
    virtual std::vector<std::string> GetNodes() const = 0;
    virtual std::vector<LibraryEntry> GetLibrary() const = 0;
    virtual BaseNodePtr CreatePluginNode(const std::string& vPluginNodeName) = 0;  // factory for nodes
    virtual std::vector<PluginPaneConfig> GetPanes() const = 0;

    // will reset the ids but will return the id count pre reset
    virtual int ResetImGuiID(const int& vWidgetId) = 0;

    // config
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override {
        std::string str;
        str += vOffset + "<" + GetName() + ">\n";
        str += vOffset + "</" + GetName() + ">\n";
        return str;
    }
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override {
        return false;
    }
};
