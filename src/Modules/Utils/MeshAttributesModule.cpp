/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "MeshAttributesModule.h"

#include <functional>

#include <Gui/MainFrame.h>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>

#include <Generic/FrameBuffer.h>

#include <ImWidgets/ImWidgets.h>

#include <Systems/CommonSystem.h>

#include <Profiler/vkProfiler.hpp>

#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>

#include <Modules/Utils/Pass/MeshAttributesModule_Pass_1.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<MeshAttributesModule> MeshAttributesModule::Create(vkApi::VulkanCore* vVulkanCore)
{
	auto res = std::make_shared<MeshAttributesModule>(vVulkanCore);
	res->m_This = res;
	if (!res->Init())
	{
		res.reset();
	}
	return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

MeshAttributesModule::MeshAttributesModule(vkApi::VulkanCore* vVulkanCore)
	: GenericRenderer(vVulkanCore)
{

}

MeshAttributesModule::~MeshAttributesModule()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool MeshAttributesModule::Init()
{
	ZoneScoped;

	ct::uvec2 map_size = 512;

	m_Loaded = true;

	if (GenericRenderer::InitPixel(map_size))
	{
		m_MeshAttributesModule_Pass_1_Ptr = std::make_shared<MeshAttributesModule_Pass_1>(m_VulkanCore);
		if (m_MeshAttributesModule_Pass_1_Ptr)
		{
			if (m_MeshAttributesModule_Pass_1_Ptr->InitPixel(map_size, 7U, true, true, 0.0f,
				vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1))
			{
				AddGenericPass(m_MeshAttributesModule_Pass_1_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool MeshAttributesModule::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd)
{
	ZoneScoped;

	if (m_LastExecutedFrame != vCurrentFrame)
	{
		GenericRenderer::Render("Mesh Attributes Module", vCmd);

		m_LastExecutedFrame = vCurrentFrame;
	}

	return true;
}

bool MeshAttributesModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Mesh Attributes", -1.0f, true, true, &m_CanWeRender))
		{
			if (m_MeshAttributesModule_Pass_1_Ptr)
			{
				return m_MeshAttributesModule_Pass_1_Ptr->DrawWidgets(vCurrentFrame, vContext);
			}
		}
	}

	return false;
}

void MeshAttributesModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void MeshAttributesModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void MeshAttributesModule::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	GenericRenderer::NeedResize(vNewSize, vCountColorBuffer);
}

void MeshAttributesModule::SetModel(SceneModelWeak vSceneModel)
{
	ZoneScoped;

	if (m_MeshAttributesModule_Pass_1_Ptr)
	{
		m_MeshAttributesModule_Pass_1_Ptr->SetModel(vSceneModel);
	}
}

void MeshAttributesModule::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	ZoneScoped;

	if (m_MeshAttributesModule_Pass_1_Ptr)
	{
		m_MeshAttributesModule_Pass_1_Ptr->SetTexture(vBinding, vImageInfo);
	}
}

vk::DescriptorImageInfo* MeshAttributesModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	ZoneScoped;

	if (m_MeshAttributesModule_Pass_1_Ptr)
	{
		return m_MeshAttributesModule_Pass_1_Ptr->GetDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

void MeshAttributesModule::UpdateShaders(const std::set<std::string>& vFiles)
{
	for (auto passPtr : m_ShaderPass)
	{
		auto passShaUpdPtr = dynamic_pointer_cast<ShaderUpdateInterface>(passPtr);
		if (passShaUpdPtr)
		{
			passShaUpdPtr->UpdateShaders(vFiles);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MeshAttributesModule::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	return str;
}

bool MeshAttributesModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	return true;
}
