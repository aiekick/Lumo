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

#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Interfaces/ModelInputInterface.h>
#include <LumoBackend/Interfaces/ShaderUpdateInterface.h>
#include <Interfaces/ParticlesOutputInterface.h>

class MeshEmitterModule;
class MeshEmitterNode : public BaseNode, public ModelInputInterface, public ShaderUpdateInterface, public ParticlesOutputInterface {
public:
    static std::shared_ptr<MeshEmitterNode> Create(GaiApi::VulkanCoreWeak vVulkanCore);

private:
    std::shared_ptr<MeshEmitterModule> m_MeshEmitterModulePtr = nullptr;

public:
    MeshEmitterNode();
    ~MeshEmitterNode() override;
    bool Init(GaiApi::VulkanCoreWeak vVulkanCore) override;
    bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    void DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) override;
    void SetModel(SceneModelWeak vSceneModel = SceneModelWeak()) override;
    SceneParticlesWeak GetParticles() override;
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
    void UpdateShaders(const std::set<std::string>& vFiles) override;
};