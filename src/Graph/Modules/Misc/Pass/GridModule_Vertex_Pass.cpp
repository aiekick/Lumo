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

#include "GridModule_Vertex_Pass.h"

#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
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
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

GridModule_Vertex_Pass::GridModule_Vertex_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : VertexShaderPass(vVulkanCore) {
    SetRenderDocDebugName("Vertex Pass 1 : Grid", VERTEX_SHADER_PASS_DEBUG_COLOR);

    m_DontUseShaderFilesOnDisk = true;
}

GridModule_Vertex_Pass::~GridModule_Vertex_Pass() {
    Unit();
}

//////////////////////////////////////////////////////////////
//// PUBLIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void GridModule_Vertex_Pass::ActionBeforeInit() {
    SetPrimitveTopology(vk::PrimitiveTopology::eLineList);
    m_LineWidth.x = 0.5f;   // min value
    m_LineWidth.y = 10.0f;  // max value
    m_LineWidth.z = 2.0f;   // default value
}

bool GridModule_Vertex_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    ZoneScoped;

    bool change = false;

    change |= ImGui::SliderFloatDefaultCompact(0.0f, "Line Thickness", &m_LineWidth.w, m_LineWidth.x, m_LineWidth.y, m_LineWidth.z);
    change |= ImGui::CheckBoxFloatDefault("Show Grid", &m_UBOVert.showGrid, true);
    if (m_UBOVert.showGrid > 0.5f) {
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "Grid Size", &m_UBOVert.gridSize, 0.0f, 10.0f, 1.0f, 0.1f);
        change |= ImGui::SliderIntDefaultCompact(0.0f, "Grid Count", &m_UBOVert.gridCount, 0, 50, 10, 1);
    }

    change |= ImGui::CheckBoxFloatDefault("Show Axis", &m_UBOVert.showAxis, true);
    if (m_UBOVert.showAxis > 0.5f) {
        change |= ImGui::CheckBoxFloatDefault("Both Sides", &m_UBOVert.bothSides, false);
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "Axis Size", &m_UBOVert.axisSize, 0.0f, 10.0f, 5.0f, 0.1f);
    }

    if (change) {
        NeedNewUBOUpload();
    }

    return change;
}

bool GridModule_Vertex_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool GridModule_Vertex_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

vk::DescriptorImageInfo* GridModule_Vertex_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {
    ZoneScoped;

    if (m_FrameBufferPtr) {
        AutoResizeBuffer(std::dynamic_pointer_cast<OutputSizeInterface>(m_FrameBufferPtr).get(), vOutSize);

        return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool GridModule_Vertex_Pass::CreateUBO() {
    ZoneScoped;

    m_UBOVertPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBOVert), "GridModule_Vertex_Pass");
    if (m_UBOVertPtr) {
        m_DescriptorBufferInfo_Vert.buffer = m_UBOVertPtr->buffer;
        m_DescriptorBufferInfo_Vert.range = sizeof(UBOVert);
        m_DescriptorBufferInfo_Vert.offset = 0;
    }

    NeedNewUBOUpload();

    return true;
}

void GridModule_Vertex_Pass::UploadUBO() {
    ZoneScoped;

    m_CountVertexs = 4 * m_UBOVert.gridCount + 4  // grid
                     + 6                          // axis
        ;
    VulkanRessource::upload(m_VulkanCore, m_UBOVertPtr, &m_UBOVert, sizeof(UBOVert));
}

void GridModule_Vertex_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBOVertPtr.reset();
}

bool GridModule_Vertex_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);
    return res;
}

bool GridModule_Vertex_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());
    res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Vert);
    return res;
}

std::string GridModule_Vertex_Pass::GetVertexShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "GridModule_Vertex_Pass_Vertex";

    return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 vertColor;
)" + CommonSystem::GetBufferObjectStructureHeader(0U) +
           u8R"(
layout(std140, binding = 1) uniform UBOStruct {
	float showGrid;
	float gridSize;
	int gridCount;
	float showAxis;
	float bothSides;
	float axisSize;
};

void main() 
{
	float vertexId = float(gl_VertexIndex);
	
	gl_Position = vec4(0);
	vertColor = vec4(0);
	
	float countGridPoints = 4. * gridCount + 4.;
	if (vertexId < countGridPoints && showGrid > 0.5) // grid
	{
		float index = mod(vertexId, 4.);
		float indexAxis = floor(vertexId / 4.);
		float base = indexAxis * gridSize / gridCount;
		vec2 p = vec2(0);
		if (index < 0.5) p = vec2(base, 0);
		else if (index < 1.5) p = vec2(base,gridSize);
		else if (index < 2.5) p = vec2(0,base);
		else if (index < 3.5) p = vec2(gridSize,base);
		p -= gridSize * 0.5;
		gl_Position = cam * vec4(p.x, 0.0, p.y, 1);
		vertColor = vec4(0.8,0.8,0.8,1.0);
	}
	else if (vertexId < (countGridPoints + 6.) && showAxis > 0.5) // axis
	{
		vertexId -= countGridPoints;
		float index = mod(vertexId, 2.);
		float indexAxis = floor(vertexId / 2.);
		vec3 axis = vec3(0);
		if (indexAxis < 0.5) axis = vec3(1,0,0);
		else if (indexAxis < 1.5) axis = vec3(0,1,0);
		else if (indexAxis < 2.5) axis = vec3(0,0,1);
		vertColor = vec4(axis, 1.0);
		if (index < 0.5)
			if (bothSides > 0.5) axis = -axis;
			else axis = vec3(0);
		axis *= axisSize;
		gl_Position = cam * vec4(axis, 1);
	}
}
)";
}

std::string GridModule_Vertex_Pass::GetFragmentShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "GridModule_Vertex_Pass_Fragment";

    return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec4 vertColor;

void main() 
{
	fragColor = vertColor; 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GridModule_Vertex_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    str += vOffset + "<line_thickness>" + ct::toStr(m_LineWidth.w) + "</line_thickness>\n";
    str += vOffset + "<show_grid>" + (m_UBOVert.showGrid > 0.5f ? "true" : "false") + "</show_grid>\n";
    str += vOffset + "<grid_size>" + ct::toStr(m_UBOVert.gridSize) + "</grid_size>\n";
    str += vOffset + "<grid_count>" + ct::toStr(m_UBOVert.gridCount) + "</grid_count>\n";
    str += vOffset + "<show_axis>" + (m_UBOVert.showAxis > 0.5f ? "true" : "false") + "</show_axis>\n";
    str += vOffset + "<both_sides>" + (m_UBOVert.bothSides > 0.5f ? "true" : "false") + "</both_sides>\n";
    str += vOffset + "<axis_size>" + ct::toStr(m_UBOVert.axisSize) + "</axis_size>\n";

    return str;
}

bool GridModule_Vertex_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "grid_module") {
        if (strName == "line_thickness")
            m_LineWidth.w = ct::fvariant(strValue).GetF();
        else if (strName == "show_grid")
            m_UBOVert.showGrid = (ct::ivariant(strValue).GetB() ? 1.0f : 0.0f);
        else if (strName == "grid_size")
            m_UBOVert.gridSize = ct::fvariant(strValue).GetF();
        else if (strName == "grid_count")
            m_UBOVert.gridCount = ct::ivariant(strValue).GetI();
        else if (strName == "show_axis")
            m_UBOVert.showAxis = (ct::ivariant(strValue).GetB() ? 1.0f : 0.0f);
        else if (strName == "both_sides")
            m_UBOVert.bothSides = (ct::ivariant(strValue).GetB() ? 1.0f : 0.0f);
        else if (strName == "axis_size")
            m_UBOVert.axisSize = ct::fvariant(strValue).GetF();

        NeedNewUBOUpload();
    }

    return true;
}