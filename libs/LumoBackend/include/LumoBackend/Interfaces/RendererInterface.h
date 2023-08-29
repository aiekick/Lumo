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

#pragma once
#pragma warning(disable : 4251)

#include <set>
#include <string>
#include <vector>
#include <ctools/cTools.h>
#include <Gaia/gaia.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Resources/VulkanFrameBufferAttachment.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

class CameraInterface;
class LUMO_BACKEND_API RendererInterface
{
public:
	std::string name;
	bool canWeRender = false;
	std::weak_ptr<RendererInterface> m_This;

public:
	virtual void Render(vk::CommandBuffer* vCmdBuffer) = 0;
	virtual std::vector<GaiApi::VulkanFrameBufferAttachment>* GetBufferAttachments(uint32_t* vMaxBuffers) { return nullptr; }
	virtual void UpdateShaders(const std::set<std::string>& vFiles, vk::RenderPass* vRenderPass) {}
};