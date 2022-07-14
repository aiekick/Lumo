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

#include "LightGroupModule.h"
#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/BaseNode.h>
#include <Systems/CommonSystem.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <Systems/GizmoSystem.h>
#include <FontIcons/CustomFont.h>
#include <cinttypes>

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

LightGroupModulePtr LightGroupModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	auto res = std::make_shared<LightGroupModule>(vVulkanCorePtr);
	res->m_This = res;
	res->SetParentNode(vParentNode);
	if (!res->Init())
	{
		res.reset();
	}
	return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

LightGroupModule::LightGroupModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: m_VulkanCorePtr(vVulkanCorePtr)
{
	
}

LightGroupModule::~LightGroupModule()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool LightGroupModule::Init()
{
	m_SceneLightGroupPtr = SceneLightGroup::Create(m_VulkanCorePtr);

	return (m_SceneLightGroupPtr != nullptr);
}

void LightGroupModule::Unit()
{
	m_SceneLightGroupPtr.reset();
}

bool LightGroupModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd)
{
	if (m_LastExecutedFrame != vCurrentFrame)
	{
		uint32_t idx = 0U;
		for (auto lightPtr : *m_SceneLightGroupPtr)
		{
			if (lightPtr && lightPtr->wasChanged)
			{
				lightPtr->NeedUpdateCamera();
				m_SceneLightGroupPtr->GetSBO430().SetVar(ct::toStr("lightDatas_%u", idx), lightPtr->lightDatas); 
				
				lightPtr->wasChanged = false;
			}

			++idx;
		}

		m_SceneLightGroupPtr->UploadBufferObjectIfDirty(m_VulkanCorePtr);

		m_LastExecutedFrame = vCurrentFrame;
	}

	return false;
}

bool LightGroupModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_SceneLightGroupPtr)
	{
		bool oneChangedLightGroupAtLeast = false;

		if (m_SceneLightGroupPtr->CanAddLight())
		{
			if (ImGui::ContrastedButton("Add LightGroup"))
			{
				m_SceneLightGroupPtr->Add();
				m_SceneLightGroupPtr->UploadBufferObjectIfDirty(m_VulkanCorePtr);
				auto parentNodePtr = GetParentNode().getValidShared();
				if (parentNodePtr)
				{
					parentNodePtr->Notify(NotifyEvent::LightGroupUpdateDone);
				}
			}

			ImGui::SameLine();

			ImGui::Text("Maximum 8 lights");
		}

		uint32_t idx = 0U;
		for (auto lightPtr : *m_SceneLightGroupPtr)
		{
			if (lightPtr)
			{
				std::string label = ct::toStr("%s##%" PRIxPTR "", 
					lightPtr->name.c_str(), (uintptr_t)lightPtr.get());

				bool expanded = false;
				if (m_SceneLightGroupPtr->CanRemoveLight())
				{
					bool delete_button = false;
					expanded = ImGui::CollapsingHeader_Button(
						label.c_str(), -1.0f, false,
						ICON_NDP_CANCEL, true, &delete_button);
					if (delete_button)
					{
						m_SceneLightGroupPtr->erase(idx);
						m_SceneLightGroupPtr->UploadBufferObjectIfDirty(m_VulkanCorePtr);
						auto parentNodePtr = GetParentNode().getValidShared();
						if (parentNodePtr)
						{
							parentNodePtr->Notify(NotifyEvent::LightGroupUpdateDone);
						}
						break;
					}
				}
				else
				{
					expanded = ImGui::CollapsingHeader(label.c_str());
				}
				
				if (expanded)
				{
					ImGui::Header("Lights");

					lightPtr->wasChanged |= ImGui::CheckBoxFloatDefault("Active", &lightPtr->lightDatas.lightActive, true);

					auto lightTypeIndex = (int32_t)lightPtr->lightDatas.lightType;
					if (ImGui::ContrastedComboVectorDefault(0.0f, "Type", &lightTypeIndex, 
						{ "NONE","POINT", "DIRECTIONNAL", "SPOT", "AREA" }, (int32_t)LightTypeEnum::POINT))
					{
						lightPtr->lightDatas.lightType = lightTypeIndex;
						lightPtr->wasChanged = true;
					}
					if (ImGui::ColorEdit4Default(0.0f, "Color", &lightPtr->lightDatas.lightColor.x, &m_DefaultLightGroupColor.x))
					{
						lightPtr->AdaptIconColor();
						lightPtr->wasChanged = true;
					}
					lightPtr->wasChanged |= ImGui::SliderFloatDefaultCompact(0.0f, "Intensity", &lightPtr->lightDatas.lightIntensity, 0.0f, 1.0f, 1.0f);

					if (lightTypeIndex == 2U) // orthographic
					{
						ImGui::Header("Orthographic");

						lightPtr->wasChanged |= ImGui::SliderFloatDefaultCompact(0.0f, "Width/Height", &lightPtr->lightDatas.orthoSideSize, 0.0f, 1000.0f, 30.0f);
						lightPtr->wasChanged |= ImGui::SliderFloatDefaultCompact(0.0f, "Rear", &lightPtr->lightDatas.orthoRearSize, 0.0f, 1000.0f, 1000.0f);
						lightPtr->wasChanged |= ImGui::SliderFloatDefaultCompact(0.0f, "Deep", &lightPtr->lightDatas.orthoDeepSize, 0.0f, 1000.0f, 1000.0f);
					}
					else if (lightTypeIndex == 3U) // perspective
					{
						ImGui::Header("Perpective");

						lightPtr->wasChanged |= ImGui::SliderFloatDefaultCompact(0.0f, "Perspective Angle", &lightPtr->lightDatas.perspectiveAngle, 0.0f, 180.0f, 45.0f);
						lightPtr->wasChanged |= ImGui::SliderFloatDefaultCompact(0.0f, "Deep", &lightPtr->lightDatas.orthoDeepSize, 0.0f, 1000.0f, 1000.0f);
					}
					
					ImGui::Header("Gizmo");

					lightPtr->wasChanged |= ImGui::CheckBoxBoolDefault("Show Icon", &lightPtr->showIcon, true);
					lightPtr->wasChanged |= ImGui::CheckBoxBoolDefault("Show Text", &lightPtr->showText, true);

					lightPtr->wasChanged |= GizmoSystem::Instance()->DrawGizmoTransformDialog(lightPtr);

					ImGui::Header("Gizmo Matrix");

					float* floatArr = lightPtr->GetGizmoFloatPtr();
					ImGui::Text("0 : %.2f %.2f %.2f %.2f\n1 : %.2f %.2f %.2f %.2f\n2 : %.2f %.2f %.2f %.2f\n3 : %.2f %.2f %.2f %.2f",
						floatArr[0], floatArr[1], floatArr[2], floatArr[3], floatArr[4], floatArr[5], floatArr[6], floatArr[7],
						floatArr[8], floatArr[9], floatArr[10], floatArr[11], floatArr[12], floatArr[13], floatArr[14], floatArr[15]);

					if (lightPtr->wasChanged)
					{
						oneChangedLightGroupAtLeast = true;

						m_SceneLightGroupPtr->GetSBO430().SetVar(ct::toStr("lightDatas_%u", idx), lightPtr->lightDatas);
					}
				}
			}
			
			++idx;
		}

		if (oneChangedLightGroupAtLeast)
		{
			m_SceneLightGroupPtr->UploadBufferObjectIfDirty(m_VulkanCorePtr);

			auto parentNodePtr = GetParentNode().getValidShared();
			if (parentNodePtr)
			{
				parentNodePtr->Notify(NotifyEvent::LightGroupUpdateDone);
			}
		}
	}

	return false;
}

void LightGroupModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_SceneLightGroupPtr)
	{
		for (auto lightPtr : *m_SceneLightGroupPtr)
		{
			if (GizmoSystem::Instance()->DrawTooltips(lightPtr, vRect))
			{
				auto parentNodePtr = GetParentNode().getValidShared();
				if (parentNodePtr)
				{
					parentNodePtr->Notify(NotifyEvent::LightGroupUpdateDone);
				}
			}
		}
	}
}

void LightGroupModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& /*vMaxSize*/, ImGuiContext* vContext)
{
	assert(vContext);

}

SceneLightGroupWeak LightGroupModule::GetLightGroup()
{
	return m_SceneLightGroupPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string LightGroupModule::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;
	str += vOffset + "<light_group>\n";

	if (m_SceneLightGroupPtr)
	{
		std::vector<float> matrix; matrix.resize(16);
		
		for (auto lightPtr : *m_SceneLightGroupPtr)
		{
			memcpy(matrix.data(), lightPtr->GetGizmoFloatPtr(), matrix.size() * sizeof(float));

			str += vOffset + "\t<light>\n";
			str += vOffset + "\t\t<transform>" + ct::fvariant(matrix).GetS() + "</transform>\n";
			str += vOffset + "\t\t<active>" + (lightPtr->lightDatas.lightActive ? "true" : "false") + "</active>\n";
			str += vOffset + "\t\t<color>" + lightPtr->lightDatas.lightColor.string() + "</color>\n";
			str += vOffset + "\t\t<perspective_angle>" + ct::toStr(lightPtr->lightDatas.perspectiveAngle) + "</perspective_angle>\n";
			str += vOffset + "\t\t<intensity>" + ct::toStr(lightPtr->lightDatas.lightIntensity) + "</intensity>\n";
			str += vOffset + "\t\t<type>" + GetStringFromLightTypeEnum((LightTypeEnum)lightPtr->lightDatas.lightType) + "</type>\n";
			str += vOffset + "\t\t<name>" + lightPtr->name + "</name>\n";
			str += vOffset + "\t\t<show_icon>" + (lightPtr->showIcon ? "true" : "false") + "</show_icon>\n";
			str += vOffset + "\t\t<show_text>" + (lightPtr->showText ? "true" : "false") + "</show_text>\n";
			str += vOffset + "\t</light>\n";
		}
	}
	
	str += vOffset + "</light_group>\n";
	return str;
}

bool LightGroupModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
{
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "light_group")
	{
		if (strName == "light")
		{
			if (!m_FirstXmlLightGroup)
			{
				m_SceneLightGroupPtr->Add();
			}
			else
			{
				m_FirstXmlLightGroup = false;
			}
		}
	}

	if (strParentName == "light")
	{
		if (!m_SceneLightGroupPtr->empty())
		{
			auto lastPtr = m_SceneLightGroupPtr->Get(m_SceneLightGroupPtr->size() - 1U).getValidShared();
			if (lastPtr)
			{
				if (strName == "transform")
				{
					std::vector<float> matrix = ct::fvariant(strValue).GetVectorFloat();
					if (matrix.size() <= 16U)
					{
						memcpy(lastPtr->GetGizmoFloatPtr(), matrix.data(), matrix.size() * sizeof(float));
					}
				}
				else if (strName == "active")
					lastPtr->lightDatas.lightActive = ct::ivariant(strValue).GetB();
				else if (strName == "color")
				{
					lastPtr->lightDatas.lightColor = ct::fvariant(strValue).GetV4();
					lastPtr->AdaptIconColor();
				}
				else if (strName == "perspective_angle")
					lastPtr->lightDatas.perspectiveAngle = ct::fvariant(strValue).GetF();
				else if (strName == "intensity")
					lastPtr->lightDatas.lightIntensity = ct::fvariant(strValue).GetF();
				else if (strName == "type")
					lastPtr->lightDatas.lightType = (int)GetLightTypeEnumFromString(strValue);
				else if (strName == "name")
					lastPtr->name = strValue;
				else if (strName == "show_icon")
					lastPtr->showIcon = ct::ivariant(strValue).GetB();
				else if (strName == "show_text")
					lastPtr->showText = ct::ivariant(strValue).GetB();
			}
		}
	}

	return true;
}
