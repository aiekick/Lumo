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

#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <Gaia/gaia.h>
#include <ImGuiPack.h>
#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>

#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/TaskInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/ResizerInterface.h>

#include <LumoBackend/Interfaces/VariableInputInterface.h>
#include <LumoBackend/Interfaces/ModelOutputInterface.h>

#include <tinyexpr/tinyexpr.h>

class ParametricCurveModule : public NodeInterface,
                              public conf::ConfigAbstract,
                              public VariableInputInterface<0U>,
                              public ModelOutputInterface,
                              public GuiInterface {
private:
    static constexpr size_t s_EXPR_MAX_LEN = 1024;

public:
    static std::shared_ptr<ParametricCurveModule> Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode);

private:
    std::weak_ptr<ParametricCurveModule> m_This;
    GaiApi::VulkanCoreWeak m_VulkanCore;
    SceneModelPtr m_SceneModelPtr = nullptr;

private:  // curve
    int m_Err_x = 0, m_Err_y = 0, m_Err_z = 0;
    char m_ExprX[s_EXPR_MAX_LEN + 1] = "t - pi * 0.5";
    char m_ExprY[s_EXPR_MAX_LEN + 1] = "0";
    char m_ExprZ[s_EXPR_MAX_LEN + 1] = "sin(t * 5.0) * 0.25";
    double m_Start_T = 0.0;
    double m_End_T = 3.14259;
    double m_Step_T = 0.1;
    std::map<std::string, double> m_VarNameValues;
    std::vector<te_variable> m_Vars;
    char m_VarToAddBuffer[s_EXPR_MAX_LEN + 1] = "";
    bool m_CloseCurve = false;
    ct::dvec3 m_CenterPoint;

public:
    ParametricCurveModule(GaiApi::VulkanCoreWeak vVulkanCore);
    ~ParametricCurveModule();

    bool Init();
    void Unit();

    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, void* vUserDatas) override;

    // Interfaces Setters
    void SetVariable(const uint32_t& vVarIndex, SceneVariableWeak vSceneVariable, void* vUserDatas) override;

    // Interfaces Getters
    SceneModelWeak GetModel() override;

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
    void AfterNodeXmlLoading() override;

private:
    void prDrawWidgets();
    void prUpdateMesh();

    void prAddVar(const std::string& vName, const double& vValue);
    void prDelVar(const std::string& vName);

    bool prDrawInputExpr(
        const char* vLabel, const char* vBufferLabel, char* vBuffer, size_t vBufferSize, const int& vError, const char* vDdefaultValue);
    bool prDrawVars();
};
