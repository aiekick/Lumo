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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "PlanetModule_Ground_Mesh_Pass.h"

#include <cinttypes>
#include <functional>
#include <Gui/MainFrame.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanSubmitter.h>
#include <utils/Mesh/VertexStruct.h>
#include <Base/FrameBuffer.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

PlanetModule_Ground_Mesh_Pass::PlanetModule_Ground_Mesh_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: MeshShaderPass<VertexStruct::P3_N3_C4>(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	ZoneScoped;

	SetRenderDocDebugName("Mesh Pass : Planet Ground", MESH_SHADER_PASS_DEBUG_COLOR);

	//m_DontUseShaderFilesOnDisk = true;
}

PlanetModule_Ground_Mesh_Pass::~PlanetModule_Ground_Mesh_Pass()
{
	ZoneScoped;

	Unit();
}

void PlanetModule_Ground_Mesh_Pass::ActionBeforeInit()
{
	ZoneScoped;

	//m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);

	SetDynamicallyChangePrimitiveTopology(true);
	SetPrimitveTopology(vk::PrimitiveTopology::eTriangleList);
	m_PrimitiveTopologiesIndex = (int32_t)vk::PrimitiveTopology::eTriangleList;
	m_LineWidth.x = 0.5f;	// min value
	m_LineWidth.y = 10.0f;	// max value
	m_LineWidth.z = 2.0f;	// default value
	m_LineWidth.w; // value to change

	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}
}

bool PlanetModule_Ground_Mesh_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	bool need_ubos_update = false;
	bool need_model_update = false;

	//need_ubos_update |= DrawResizeWidget();

	if (ImGui::CollapsingHeader("Ground : Mesh Analyze"))
	{
		need_ubos_update |= ImGui::CheckBoxIntDefault("Use debug##ground", &m_UBO_Frag.u_use_debug, 0);
		if (m_UBO_Frag.u_use_debug)
		{
			// because tesselated
			/*if (ImGui::ContrastedComboVectorDefault(0.0f, "Display Mode", &m_PrimitiveTopologiesIndex, m_PrimitiveTopologies, 0))
			{
				ChangeDynamicPrimitiveTopology((vk::PrimitiveTopology)m_PrimitiveTopologiesIndex, true);
			}*/

			need_ubos_update |= ImGui::ContrastedComboVectorDefault(0.0f, "Channel##ground", &m_UBO_Frag.u_show_layer, m_Channels, 0);

			// because tesselated
			/*if (m_PrimitiveTopologiesIndex == 1 ||
				m_PrimitiveTopologiesIndex == 2)
			{
				need_ubos_update |= ImGui::SliderFloatDefaultCompact(0.0f, "Line Thickness", &m_LineWidth.w, m_LineWidth.x, m_LineWidth.y, m_LineWidth.z);
			}
			else if (m_PrimitiveTopologiesIndex == 0)
			{
				need_ubos_update |= ImGui::SliderFloatDefaultCompact(0.0f, "point_size", &m_UBO_Vert.u_point_size, 0.000f, 2.000f, 1.000f, 0.0f, "%.3f");
			}*/
		}

		if (m_PrimitiveTopologiesIndex > 2) // "Triangle List" or "Triangle Strip" or "Triangle Fan"
		{
			need_ubos_update |= ImGui::CheckBoxIntDefault("Show Shaded Wireframe##ground", &m_UBO_Frag.u_show_shaded_wireframe, 0);
		}

		if (!m_Vertices.m_Array.empty() && !m_Indices.m_Array.empty())
		{
			need_ubos_update |= ImGui::CheckBoxBoolDefault("Use Indices Restriction##ground", &m_UseIndiceRestriction, false);
			if (m_UseIndiceRestriction)
			{
				need_ubos_update |= ImGui::SliderUIntDefaultCompact(0.0f, "count indices##ground", &m_RestrictedIndicesCountToDraw, 0, (uint32_t)m_Indices.m_Array.size(), (uint32_t)m_Indices.m_Array.size());
			}
		}

		if (need_model_update)
		{
			CreateCube();
		}
	}

	if (ImGui::CollapsingHeader("Ground : Mesh Control"))
	{
		need_model_update |= ImGui::SliderUIntDefaultCompact(0.0f, "Subdivision Level##ground", &m_SubdivisionLevel, 0U, 5U, 0U);
	}

	if (ImGui::CollapsingHeader("Ground : Mesh Tesselation"))
	{
		need_ubos_update |= ImGui::SliderFloatDefaultCompact(0.0f, "Radius##ground", &m_UBO_Tess_Eval.u_radius, 0.0f, 10.0f, 1.0f);
		need_ubos_update |= ImGui::SliderFloatDefaultCompact(0.0f, "Tesselation level##ground", &m_UBO_Tess_Ctrl.u_tesselation_level, 0.0f, 100.0f, 1.0f);
		need_ubos_update |= ImGui::SliderFloatDefaultCompact(0.0f, "Displacement Factor##ground", &m_UBO_Tess_Eval.u_displace_factor, 0.0f, 2.0f, 1.0f);
		need_ubos_update |= ImGui::SliderFloatDefaultCompact(0.0f, "Normal Precision##ground", &m_UBO_Tess_Eval.u_normal_prec, 0.0f, 0.1f, 0.01f);
	}

	if (need_ubos_update)
	{
		NeedNewUBOUpload();
	}

	if (need_model_update)
	{
		CreateCube();
	}

	return need_ubos_update || need_model_update;
}

void PlanetModule_Ground_Mesh_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
}

void PlanetModule_Ground_Mesh_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void PlanetModule_Ground_Mesh_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBindingPoint < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				if (vTextureSize)
				{
					m_ImageInfosSize[vBindingPoint] = *vTextureSize;

					NeedResizeByHandIfChanged(m_ImageInfosSize[vBindingPoint]);
				}

				m_ImageInfos[vBindingPoint] = *vImageInfo;

				EnableTextureUse(vBindingPoint, 0U, m_UBO_Tess_Eval.u_use_height_map);
				EnableTextureUse(vBindingPoint, 1U, m_UBO_Frag.u_use_normal_map);
				EnableTextureUse(vBindingPoint, 2U, m_UBO_Frag.u_use_color_map);
			}
			else
			{
				m_ImageInfos[vBindingPoint] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();

				DisableTextureUse(vBindingPoint, 0U, m_UBO_Tess_Eval.u_use_height_map);
				DisableTextureUse(vBindingPoint, 1U, m_UBO_Frag.u_use_normal_map);
				DisableTextureUse(vBindingPoint, 2U, m_UBO_Frag.u_use_color_map);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* PlanetModule_Ground_Mesh_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;

	if (m_FrameBufferPtr)
	{
		AutoResizeBuffer(m_FrameBufferPtr.get(), vOutSize);

		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PlanetModule_Ground_Mesh_Pass::WasJustResized()
{
	ZoneScoped;
}

void PlanetModule_Ground_Mesh_Pass::DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	ZoneScoped;

	if (!m_Loaded) return;

	if (vCmdBuffer && m_Vertices.m_Count)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipelines[0].m_Pipeline);
		{
			VKFPScoped(*vCmdBuffer, "Model Renderer", "DrawModel");

			vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
				m_Pipelines[0].m_PipelineLayout, 0,
				m_DescriptorSets[0].m_DescriptorSet, nullptr);
			vk::DeviceSize offsets = 0;
			vCmdBuffer->bindVertexBuffers(0, m_Vertices.m_Buffer->buffer, offsets);

			if (m_Indices.m_Count)
			{
				vCmdBuffer->bindIndexBuffer(m_Indices.m_Buffer->buffer, 0, vk::IndexType::eUint32);
				if (m_UseIndiceRestriction)
				{
					vCmdBuffer->drawIndexed(m_RestrictedIndicesCountToDraw, 1, 0, 0, 0);
				}
				else
				{
					vCmdBuffer->drawIndexed(m_Indices.m_Count, m_CountInstances.w, 0, 0, 0);
				}
			}
			else
			{
				vCmdBuffer->draw(m_Vertices.m_Count, m_CountInstances.w, 0, 0);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PlanetModule_Ground_Mesh_Pass::CreateCube()
{
	static std::vector<ct::fvec3> cube_points = {
		ct::fvec3(-1,1,-1), ct::fvec3(1,1,-1), ct::fvec3(1,1,1), ct::fvec3(-1,1,1),     // top
		ct::fvec3(-1,-1,-1), ct::fvec3(1,-1,-1), ct::fvec3(1,-1,1), ct::fvec3(-1,-1,1)	// bottom
	};

	/*
			   7---------6
			  /|        /|
			 / |       / |
			3---------2  |
			|  4------|--5
			| /       | /
			|/        |/
			0---------1
	*/

	static std::vector<uint32_t> cube_faces = {
		0,1,2,0,2,3,
		1,5,6,1,6,2,
		5,4,7,5,7,6,
		4,0,3,4,3,7,
		3,2,6,3,6,7,
		4,5,1,4,1,0
	};

	assert(cube_points.size() == 8);
	assert(cube_faces.size() == 36);

	m_Indices.m_Array.clear();
	m_Vertices.m_Array.clear();
	m_Vertices.m_Array.reserve(cube_points.size());

	m_TriFaces.clear();
	m_TriFaces.reserve(cube_faces.size() * 3);

	for (size_t point_idx = 0; point_idx < cube_points.size(); ++point_idx)
	{
		const auto& point = cube_points.at(point_idx);
		const ct::fvec3& normal = point.GetNormalized();

		m_Vertices.m_Array.emplace_back(
			point,
			normal);
	}

	for (size_t face_idx = 0; face_idx < cube_faces.size(); face_idx += 3)
	{
		m_TriFaces.emplace_back(
			cube_faces.at(face_idx + 2),
			cube_faces.at(face_idx + 1),
			cube_faces.at(face_idx + 0));
	}

	Subdivide(m_SubdivisionLevel, m_Vertices.m_Array, m_TriFaces);

	BuildMesh();
}

size_t PlanetModule_Ground_Mesh_Pass::GetMiddlePoint_Plane(const size_t& p1, const size_t& p2, std::vector<VertexStruct::P3_N3_C4>& vVertices, CacheDB& vCache)
{
	if (p1 == p2)
	{
		CTOOL_DEBUG_BREAK;
	}

	std::tuple<size_t, size_t> _block(ct::mini(p1, p2), ct::maxi(p1, p2));

	if (vCache.find(_block) != vCache.end())
	{
		return vCache.at(_block);
	}

	// not in cache, calculate it
	const auto& pt1 = vVertices.at(p1);
	const auto& pt2 = vVertices.at(p2);
	const auto& middle_point = (pt1.p + pt2.p) * 0.5f;
	const auto& middle_normal = middle_point.GetNormalized();

	// add vertex makes sure point is on unit sphere
	size_t i = vVertices.size();
	vVertices.emplace_back(middle_point, middle_normal);

	// store it, return index
	vCache[_block] = i;

	return i;
}

void PlanetModule_Ground_Mesh_Pass::CalcNormal(VertexStruct::P3_N3_C4& v0, VertexStruct::P3_N3_C4& v1, VertexStruct::P3_N3_C4& v2)
{
	const ct::fvec3 vec0 = (v2.p - v0.p).GetNormalized();
	const ct::fvec3 vec1 = (v1.p - v0.p).GetNormalized();
	const ct::fvec3 nor = cCross(vec1, vec0).GetNormalized();

	v0.n += nor; v1.n += nor; v2.n += nor;
}

void PlanetModule_Ground_Mesh_Pass::Subdivide(const size_t& vSubdivLevel, std::vector<VertexStruct::P3_N3_C4>& vVertices, std::vector<TriFace>& vFaces)
{
	if (vSubdivLevel > 0)
	{
		CacheDB middlePointIndexCache;

		// refine triangles
		try
		{
			for (int i = 0; i < vSubdivLevel; ++i)
			{
				size_t a = 0, b = 0, c = 0;

				std::vector<TriFace> subFaces;
				std::vector<VertexStruct::P3_N3_C4> subVertices = m_Vertices.m_Array;

				for (auto& face : m_TriFaces)
				{
					a = GetMiddlePoint_Plane(face.v0, face.v1, subVertices, middlePointIndexCache);
					b = GetMiddlePoint_Plane(face.v1, face.v2, subVertices, middlePointIndexCache);
					c = GetMiddlePoint_Plane(face.v2, face.v0, subVertices, middlePointIndexCache);

					subFaces.emplace_back(face.v0, a, c);
					subFaces.emplace_back(face.v1, b, a);
					subFaces.emplace_back(face.v2, c, b);
					subFaces.emplace_back(a, b, c);
				}

				m_TriFaces = subFaces;
				m_Vertices.m_Array = subVertices;
			}
		}
		catch (std::exception& ex)
		{
			LogVarError("error %s", std::string(ex.what()).c_str());
		}
	}
}

void PlanetModule_Ground_Mesh_Pass::BuildMesh()
{
	if (!m_Vertices.m_Array.empty())
	{
		if (!m_TriFaces.empty())
		{
			m_Indices.m_Array.clear();

			m_Indices.m_Array.reserve(m_TriFaces.size() * 3U); // 6 => 1 triangle so 3 indices
			for (auto& f : m_TriFaces)
			{
				m_Indices.m_Array.push_back(VertexStruct::I1(f.v0));
				m_Indices.m_Array.push_back(VertexStruct::I1(f.v1));
				m_Indices.m_Array.push_back(VertexStruct::I1(f.v2));
			}
		}

		for (auto& v : m_Vertices.m_Array)
		{
			// arrange p for better spacing in case of sphere
			// https://mathproofs.blogspot.com/2005/07/mapping-cube-to-sphere.html
			const auto p2 = v.p * v.p;
			v.p.x *= sqrtf(1.0f - (p2.y + p2.z) * 0.5f + (p2.y * p2.z) * 0.33333f);
			v.p.y *= sqrtf(1.0f - (p2.z + p2.x) * 0.5f + (p2.z * p2.x) * 0.33333f);
			v.p.z *= sqrtf(1.0f - (p2.x + p2.y) * 0.5f + (p2.x * p2.y) * 0.33333f);

			// normal of p
			v.n = v.p.GetNormalized(); // compute simple normal in case of sphere

			// normalize of p * radius => cube to sphere
			v.p = v.n * m_UBO_Tess_Eval.u_radius; // transform to sphere
		}

		Build();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PROTECTED /////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PlanetModule_Ground_Mesh_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBO_Vert_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Vert));
	m_UBO_Vert_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Vert_Ptr) {
		m_UBO_Vert_BufferInfos.buffer = m_UBO_Vert_Ptr->buffer;
		m_UBO_Vert_BufferInfos.range = sizeof(UBO_Vert);
		m_UBO_Vert_BufferInfos.offset = 0;
	}

	m_UBO_Tess_Ctrl_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Tess_Ctrl));
	m_UBO_Tess_Ctrl_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Tess_Ctrl_Ptr) {
		m_UBO_Tess_Ctrl_BufferInfos.buffer = m_UBO_Tess_Ctrl_Ptr->buffer;
		m_UBO_Tess_Ctrl_BufferInfos.range = sizeof(UBO_Tess_Ctrl);
		m_UBO_Tess_Ctrl_BufferInfos.offset = 0;
	}

	m_UBO_Tess_Eval_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Tess_Eval));
	m_UBO_Tess_Eval_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Tess_Eval_Ptr) {
		m_UBO_Tess_Eval_BufferInfos.buffer = m_UBO_Tess_Eval_Ptr->buffer;
		m_UBO_Tess_Eval_BufferInfos.range = sizeof(UBO_Tess_Eval);
		m_UBO_Tess_Eval_BufferInfos.offset = 0;
	}

	m_UBO_Frag_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Frag));
	m_UBO_Frag_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Frag_Ptr) {
		m_UBO_Frag_BufferInfos.buffer = m_UBO_Frag_Ptr->buffer;
		m_UBO_Frag_BufferInfos.range = sizeof(UBO_Frag);
		m_UBO_Frag_BufferInfos.offset = 0;
	}

	NeedNewUBOUpload();

	return true;
}

void PlanetModule_Ground_Mesh_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_Vert_Ptr, &m_UBO_Vert, sizeof(UBO_Vert));
	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_Tess_Ctrl_Ptr, &m_UBO_Tess_Ctrl, sizeof(UBO_Tess_Ctrl));
	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_Tess_Eval_Ptr, &m_UBO_Tess_Eval, sizeof(UBO_Tess_Eval));
	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_Frag_Ptr, &m_UBO_Frag, sizeof(UBO_Frag));
}

void PlanetModule_Ground_Mesh_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag_Ptr.reset();
	m_UBO_Frag_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	m_UBO_Tess_Ctrl_Ptr.reset();
	m_UBO_Tess_Ctrl_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	m_UBO_Tess_Eval_Ptr.reset();
	m_UBO_Tess_Eval_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	m_UBO_Vert_Ptr.reset();
	m_UBO_Vert_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}

bool PlanetModule_Ground_Mesh_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;

	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eTessellationEvaluation); // common system UBO
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex); // vertex UBO
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eTessellationControl); // tesselation control UBO
	res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eTessellationEvaluation); // tesselation evaluation UBO
	res &= AddOrSetLayoutDescriptor(4U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment); // fragment UBO
	res &= AddOrSetLayoutDescriptor(5U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eTessellationEvaluation); // height
	res &= AddOrSetLayoutDescriptor(6U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment); // normal
	res &= AddOrSetLayoutDescriptor(7U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment); // color

	return res;
}

bool PlanetModule_Ground_Mesh_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;

	res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo()); // common system UBO
	res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBO_Vert_BufferInfos); // vertex UBO
	res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eUniformBuffer, &m_UBO_Tess_Ctrl_BufferInfos); // tesselation control UBO
	res &= AddOrSetWriteDescriptorBuffer(3U, vk::DescriptorType::eUniformBuffer, &m_UBO_Tess_Eval_BufferInfos); // tesselation evaluation UBO
	res &= AddOrSetWriteDescriptorBuffer(4U, vk::DescriptorType::eUniformBuffer, &m_UBO_Frag_BufferInfos); // fragment UBO
	res &= AddOrSetWriteDescriptorImage(5U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]); // height
	res &= AddOrSetWriteDescriptorImage(6U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[1]); // normal
	res &= AddOrSetWriteDescriptorImage(7U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[2]); // color
	
	return res;
}

std::string PlanetModule_Ground_Mesh_Pass::GetCommonCode()
{
	return u8R"(
vec2 get_uv_from_sphere(vec3 p)
{
	p = normalize(p);

	float lon = atan(p.x, p.z);
	float lat = asin(-p.y);

	const float one_on_pi = 1.0 / 3.14159;
	float u = (lon * one_on_pi + 1.0) * 0.5;
	float v = lat * one_on_pi + 0.5;

	return vec2(u, v);
}
)";
}

std::string PlanetModule_Ground_Mesh_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PlanetModule_Ground_Mesh_Pass_Vertex";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aColor;

layout(location = 0) out vec3 v_position;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec4 v_color;
)"
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
u8R"(

layout(std140, binding = 1) uniform UBO_Vert
{
	float u_point_size;
};

void main() 
{
	gl_PointSize = u_point_size;

	v_position = aPosition;
	v_normal = aNormal;
	v_color = aColor;

	gl_Position = cam * vec4(aPosition, 1.0);
}
)";
}

std::string PlanetModule_Ground_Mesh_Pass::GetTesselationControlShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PlanetModule_Ground_Mesh_Pass_Tesselation_Control";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

// define the number of CPs in the output patch
layout (vertices = 3) out;

// attributes of the input CPs
layout(location = 0) in vec3 v_position[];
layout(location = 1) in vec3 v_normal[];
layout(location = 2) in vec4 v_color[];

layout(location = 0) out vec3 v_position_tess_control[];

layout(std140, binding = 2) uniform UBO_Tess_Ctrl
{
	float u_tesselation_level;
};

void main()
{
    // Set the control points of the output patch
    v_position_tess_control[gl_InvocationID] = v_position[gl_InvocationID];
	
	if (gl_InvocationID == 0)
	{
		// Define / Calculate the tessellation levels
		gl_TessLevelOuter[0] = u_tesselation_level;
		gl_TessLevelOuter[1] = u_tesselation_level;
		gl_TessLevelOuter[2] = u_tesselation_level;
		gl_TessLevelOuter[3] = u_tesselation_level;
		
		gl_TessLevelInner[0] = u_tesselation_level;
		gl_TessLevelInner[1] = u_tesselation_level;
	}
} 
)";
}

std::string PlanetModule_Ground_Mesh_Pass::GetTesselationEvaluationShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PlanetModule_Ground_Mesh_Pass_Tesselation_Evaluation";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(triangles, equal_spacing, ccw) in;

layout(location = 0) in vec3 v_position_tess_control[];

layout(location = 0) out vec3 v_position;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec4 v_color;

)"
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
u8R"(

layout(std140, binding = 3) uniform UBO_Tess_Eval
{
	float u_use_height_map;
	float u_displace_factor;
	float u_radius;
	float u_normal_prec;
};

layout(binding = 5) uniform sampler2D u_tex_height;

)"
+ GetCommonCode() +
u8R"(

vec2 interpolateV2(vec2 v0, vec2 v1, vec2 v2) {
    return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2;
} 

vec3 interpolateV3(vec3 v0, vec3 v1, vec3 v2) {
    return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2;
} 

vec4 interpolateV4(vec4 v0, vec4 v1, vec4 v2) {
    return gl_TessCoord.x * v0 + gl_TessCoord.y * v1 + gl_TessCoord.z * v2;
} 

float displacement(vec3 p)
{
	vec2 uv = get_uv_from_sphere(p);
	return texture(u_tex_height, uv).x * 2.0 - 1.0;
} 

void main()
{
    // Interpolate the attributes of the output vertex using the barycentric coordinates
    v_position = interpolateV3(v_position_tess_control[0], v_position_tess_control[1], v_position_tess_control[2]);
	
	// Displace the vertex along the normal
	v_normal = normalize(v_position);
	float height = displacement(v_position).x;
	v_position = v_normal * (u_radius + height * u_displace_factor);
	v_color = vec4(height, height, height, 1.0);
	gl_Position = cam * vec4(v_position, 1.0);
	
	if (u_normal_prec > 1e-7)
	{
		vec3 eps = vec3(u_normal_prec,0,0);
		float f = displacement(v_position);
		float fx = (f-displacement(v_position - eps)) / u_normal_prec;
		float fy = (f-displacement(v_position - eps.yxz)) / u_normal_prec;
		float fz = (f-displacement(v_position - eps.zyx)) / u_normal_prec;
		v_normal = normalize(v_normal - vec3(fx,fy,fz) * u_displace_factor);
	}
} 
)";
}

std::string PlanetModule_Ground_Mesh_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PlanetModule_Ground_Mesh_Pass_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragPosition;
layout(location = 1) out vec4 fragNormal;
layout(location = 2) out vec4 fragColor;

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec4 v_color;

layout(std140, binding = 4) uniform UBO_Frag
{
	int u_use_debug;
	int u_show_layer;
	int u_show_shaded_wireframe;
	float u_use_normal_map;
	float u_use_color_map;
};

layout(binding = 6) uniform sampler2D u_tex_normal;
layout(binding = 7) uniform sampler2D u_tex_color;

)"
+ GetCommonCode() +
u8R"(

void init_color()
{
	fragPosition = vec4(v_position, 1.0);
	fragNormal = vec4(v_normal * 0.5 + 0.5, 1.0);
	fragColor = v_color;
}

void display_color_map()
{
	if (u_use_color_map > 0.5)
	{	
		vec2 uv = get_uv_from_sphere(v_position);
		fragColor.rgb = texture(u_tex_color, uv).rgb;
	}
}

void display_debug()
{
	if (u_use_debug > 0)
	{
		switch(u_show_layer)
		{
		case 0: // pos
			fragColor.xyz = v_position;
			break;
		case 1: // normal
			fragColor.xyz = v_normal * 0.5 + 0.5;
			break;
		case 2: // vertex color
			fragColor = v_color;
			break;
		}
	}
}

void display_wireframe()
{
	if (u_show_shaded_wireframe == 1 &&
		bitCount(gl_SampleMaskIn[0]) < 2)
	{
		fragColor.rgb = 1.0 - fragColor.rgb;
	}
}

void dislay_only_visible_mesh()
{
	if (dot(fragColor.xyz, fragColor.xyz) > 0.0)
	{
		fragColor.a = 1.0;
	}
	else
	{
	//	discard;
	}
}

void main() 
{	
	init_color();

	// color
	display_color_map();
	
	// mesh debug
	display_debug();
	display_wireframe();

	// gpu fps saver
	dislay_only_visible_mesh();
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string PlanetModule_Ground_Mesh_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += ShaderPass::getXml(vOffset, vUserDatas);

	str += vOffset + "<planet_module_ground_pass>\n";
	str += vOffset + "\t<use_debug>" + ct::toStr(m_UBO_Frag.u_use_debug) + "</use_debug>\n";
	str += vOffset + "\t<display_mode>" + ct::toStr(m_PrimitiveTopologiesIndex) + "</display_mode>\n";
	str += vOffset + "\t<line_thickness>" + ct::toStr(m_LineWidth.w) + "</line_thickness>\n";
	str += vOffset + "\t<point_size>" + ct::toStr(m_UBO_Vert.u_point_size) + "</point_size>\n";
	str += vOffset + "\t<show_layer>" + ct::toStr(m_UBO_Frag.u_show_layer) + "</show_layer>\n";
	str += vOffset + "\t<use_indices_restriction>" + (m_UseIndiceRestriction ? "true" : "false") + "</use_indices_restriction>\n";
	str += vOffset + "\t<restricted_indices_count_to_draw>" + ct::toStr(m_RestrictedIndicesCountToDraw) + "</restricted_indices_count_to_draw>\n";
	str += vOffset + "\t<radius>" + ct::toStr(m_UBO_Tess_Eval.u_radius) + "</radius>\n";
	str += vOffset + "\t<subdivision_level>" + ct::toStr(m_SubdivisionLevel) + "</subdivision_level>\n";
	str += vOffset + "\t<tesselation_level>" + ct::toStr(m_UBO_Tess_Ctrl.u_tesselation_level) + "</tesselation_level>\n";
	str += vOffset + "\t<displace_factor>" + ct::toStr(m_UBO_Tess_Eval.u_displace_factor) + "</displace_factor>\n";
	str += vOffset + "\t<normal_prec>" + ct::toStr(m_UBO_Tess_Eval.u_normal_prec) + "</normal_prec>\n";
	str += vOffset + "</planet_module_ground_pass>\n";

	return str;
}

bool PlanetModule_Ground_Mesh_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	ZoneScoped;

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	ShaderPass::setFromXml(vElem, vParent, vUserDatas);

	if (strParentName == "planet_module_ground_pass")
	{
		if (strName == "use_debug")
			m_UBO_Frag.u_use_debug = ct::ivariant(strValue).GetI();
		else if (strName == "display_mode")
			m_PrimitiveTopologiesIndex = ct::ivariant(strValue).GetI();
		else if (strName == "line_thickness")
			m_LineWidth.w = ct::fvariant(strValue).GetF();
		else if (strName == "point_size")
			m_UBO_Vert.u_point_size = ct::fvariant(strValue).GetF();
		else if (strName == "show_layer")
			m_UBO_Frag.u_show_layer = ct::ivariant(strValue).GetI();
		else if (strName == "use_indices_restriction")
			m_UseIndiceRestriction = ct::ivariant(strValue).GetB();
		else if (strName == "restricted_indices_count_to_draw")
			m_RestrictedIndicesCountToDraw = ct::ivariant(strValue).GetU();
		else if (strName == "radius")
			m_UBO_Tess_Eval.u_radius = ct::fvariant(strValue).GetF();
		else if (strName == "subdivision_level")
			m_SubdivisionLevel = ct::ivariant(strValue).GetU();
		else if (strName == "tesselation_level")
			m_UBO_Tess_Ctrl.u_tesselation_level = ct::fvariant(strValue).GetF();
		else if (strName == "displace_factor")
			m_UBO_Tess_Eval.u_displace_factor = ct::fvariant(strValue).GetF();
		else if (strName == "normal_prec")
			m_UBO_Tess_Eval.u_normal_prec = ct::fvariant(strValue).GetF();
	}

	return true;
}

void PlanetModule_Ground_Mesh_Pass::AfterNodeXmlLoading()
{
	ZoneScoped;

	ChangeDynamicPrimitiveTopology((vk::PrimitiveTopology)m_PrimitiveTopologiesIndex, true);
	CreateCube(); 
	NeedNewUBOUpload();
}
