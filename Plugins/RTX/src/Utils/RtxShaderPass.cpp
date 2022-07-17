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

#include "QuadShaderPass.h"

#include <vkFramework/VulkanRessource.h>

QuadShaderPass::QuadShaderPass(
	vkApi::VulkanCorePtr vVulkanCorePtr,
	const MeshShaderPassType& vMeshShaderPassType)
	: MeshShaderPass<VertexStruct::P2_T2>(
		vVulkanCorePtr,
		vMeshShaderPassType)
{

}

QuadShaderPass::QuadShaderPass(
	vkApi::VulkanCorePtr vVulkanCorePtr,
	const MeshShaderPassType& vMeshShaderPassType,
	vk::CommandPool* vCommandPool,
	vk::DescriptorPool* vDescriptorPool)
	: MeshShaderPass<VertexStruct::P2_T2>(
		vVulkanCorePtr,
		vMeshShaderPassType,
		vCommandPool,
		vDescriptorPool)
{

}

bool QuadShaderPass::BuildModel()
{
	ZoneScoped;

	m_Vertices.m_Array = {
		VertexStruct::P2_T2(ct::fvec2(-1.0f, -1.0f), ct::fvec2(0.0f, 0.0f)),
		VertexStruct::P2_T2(ct::fvec2(1.0f, -1.0f), ct::fvec2(1.0f, 0.0f)),
		VertexStruct::P2_T2(ct::fvec2(1.0f, 1.0f), ct::fvec2(1.0f, 1.0f)),
		VertexStruct::P2_T2(ct::fvec2(-1.0f, 1.0f), ct::fvec2(0.0f, 1.0f)),
	};
	m_Indices.m_Array = {
		0U, 1U, 2U, 0U, 2U, 3U
	};

	m_Vertices.m_Buffer = vkApi::VulkanRessource::createVertexBufferObject(m_VulkanCorePtr, m_Vertices.m_Array);
	m_Vertices.m_Count = (uint32_t)m_Vertices.m_Array.size();
	m_Vertices.m_BufferInfo.buffer = m_Vertices.m_Buffer->buffer;
	m_Vertices.m_BufferInfo.range = m_Vertices.m_Count * sizeof(VertexStruct::P3_N3_TA3_BTA3_T2_C4);
	m_Vertices.m_BufferInfo.offset = 0;

	m_Indices.m_Buffer = vkApi::VulkanRessource::createIndexBufferObject(m_VulkanCorePtr, m_Indices.m_Array);
	m_Indices.m_Count = (uint32_t)m_Indices.m_Array.size();
	m_Indices.m_BufferInfo.buffer = m_Indices.m_Buffer->buffer;
	m_Indices.m_BufferInfo.range = m_Indices.m_Count * sizeof(uint32_t);
	m_Indices.m_BufferInfo.offset = 0;

	return true;
}

void QuadShaderPass::DestroyModel(const bool& vReleaseDatas)
{
	ZoneScoped;

	m_VulkanCorePtr->getDevice().waitIdle();

	m_Vertices.m_Buffer.reset();
	m_Indices.m_BufferInfo = vk::DescriptorBufferInfo{};

	m_Indices.m_Buffer.reset();
	m_Indices.m_BufferInfo = vk::DescriptorBufferInfo{};
}

std::string QuadShaderPass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "QuadShaderPass_Vertex";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 vertPosition;
layout(location = 1) in vec2 vertUv;

layout(location = 0) out vec2 uv_map;

void main() 
{
	uv_map = vertUv;
	gl_Position = vec4(vertPosition, 0.0, 1.0);
}
)";
}

std::string QuadShaderPass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "QuadShaderPass_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 uv_map;

void main() 
{
	fragColor = vec4(uv_map, 0.0, 1.0);
}
)";
}

// will convert model in accel struct
bool ShaderPass::CreateBottomLevelAccelerationStructureForMesh(SceneMeshWeak vMesh)
{
	auto meshPtr = vMesh.getValidShared();
	if (meshPtr)
	{
		auto buffer_usage_flags =
			vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
			vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;

		/*
		vk::TransformMatrixKHR transform_matrix =
			std::array<std::array<float, 4>, 3>
		{
			1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f
		};
		std::unique_ptr<vkb::core::Buffer> transform_matrix_buffer = std::make_unique<vkb::core::Buffer>(get_device(), sizeof(transform_matrix), buffer_usage_flags, VMA_MEMORY_USAGE_CPU_TO_GPU);
		transform_matrix_buffer->update(&transform_matrix, sizeof(transform_matrix));
		*/

		vk::BufferDeviceAddressInfoKHR vertex_buffer_device_address_info{};
		vertex_buffer_device_address_info.buffer = meshPtr->GetVerticesBuffer();
		auto vertex_address = m_Device.getBufferAddressKHR(&vertex_buffer_device_address_info);

		vk::BufferDeviceAddressInfoKHR index_buffer_device_address_info{};
		index_buffer_device_address_info.buffer = meshPtr->GetIndicesBuffer();
		auto index_address = m_Device.getBufferAddressKHR(&index_buffer_device_address_info);

		vk::AccelerationStructureGeometryTrianglesDataKHR triangles;
		triangles.vertexFormat = vk::Format::eR32G32B32Sfloat;
		triangles.vertexData = vertex_address;
		triangles.maxVertex = meshPtr->GetVerticesCount();
		triangles.vertexStride = sizeof(VertexStruct::P3_N3_TA3_BTA3_T2_C4);
		triangles.indexType = vk::IndexType::eUint32;
		triangles.indexData = index_address;
		//triangles.transformData = transform_matrix_device_address;

		// The bottom level acceleration structure contains one set of triangles as the input geometry
		vk::AccelerationStructureGeometryKHR accelStructureGeometry;
		accelStructureGeometry.geometryType = vk::GeometryTypeKHR::eTriangles;
		accelStructureGeometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
		accelStructureGeometry.geometry.triangles = triangles;

		// Get the size requirements for buffers involved in the acceleration structure build process
		vk::AccelerationStructureBuildGeometryInfoKHR accelStructureBuildGeometryInfo;
		accelStructureBuildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
		accelStructureBuildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
		accelStructureBuildGeometryInfo.geometryCount = 1;
		accelStructureBuildGeometryInfo.pGeometries = &accelStructureGeometry;

		const uint32_t triangle_count = meshPtr->GetIndicesCount() / 3U;

		auto accelStructureBuildSizeInfo = m_Device.getAccelerationStructureBuildSizesKHR(
			vk::AccelerationStructureBuildTypeKHR::eDevice,
			accelStructureBuildGeometryInfo, triangle_count);

		// Create a buffer to hold the acceleration structure
		auto accelStructure_Bottom_Ptr = VulkanRessource::createAccelStructureBufferObject(m_VulkanCorePtr,
			accelStructureBuildSizeInfo.accelerationStructureSize,
			VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);

		// Create the acceleration structure
		vk::AccelerationStructureCreateInfoKHR accelStructureCreateInfo;
		accelStructureCreateInfo.buffer = accelStructure_Bottom_Ptr->buffer;
		accelStructureCreateInfo.size = accelStructureBuildSizeInfo.accelerationStructureSize;
		accelStructureCreateInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
		accelStructure_Bottom_Ptr->handle = m_Device.createAccelerationStructureKHR(accelStructureCreateInfo);

		// The actual build process starts here

		// Create a scratch buffer as a temporary storage for the acceleration structure build
		auto scratchBufferPtr = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr,
			accelStructureBuildSizeInfo.accelerationStructureSize,
			vk::BufferUsageFlagBits::eShaderDeviceAddressKHR | vk::BufferUsageFlagBits::eStorageBuffer,
			VMA_MEMORY_USAGE_CPU_TO_GPU);

		vk::BufferDeviceAddressInfoKHR scratchBufferDeviceAddressInfo{};
		scratchBufferDeviceAddressInfo.buffer = scratchBufferPtr->buffer;
		auto scratchBufferAddress = m_Device.getBufferAddressKHR(&scratchBufferDeviceAddressInfo);

		vk::AccelerationStructureBuildGeometryInfoKHR accelBuildGeometryInfo;
		accelBuildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
		accelBuildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
		accelBuildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
		accelBuildGeometryInfo.dstAccelerationStructure = accelStructure_Bottom_Ptr->handle;
		accelBuildGeometryInfo.geometryCount = 1;
		accelBuildGeometryInfo.pGeometries = &accelStructureGeometry;
		accelBuildGeometryInfo.scratchData.deviceAddress = scratchBufferAddress;

		vk::AccelerationStructureBuildRangeInfoKHR accelStructureBuildRangeInfo;
		accelStructureBuildRangeInfo.primitiveCount = triangle_count;
		accelStructureBuildRangeInfo.primitiveOffset = 0;
		accelStructureBuildRangeInfo.firstVertex = 0;
		accelStructureBuildRangeInfo.transformOffset = 0;
		std::vector<vk::AccelerationStructureBuildRangeInfoKHR*> accelStructureBuildRangeInfos =
		{ &accelStructureBuildRangeInfo };

		// Build the acceleration structure on the device via a one-time command buffer submission
		auto cmd = VulkanCommandBuffer::beginSingleTimeCommands(m_VulkanCorePtr, true);
		cmd.buildAccelerationStructuresKHR(1, &accelBuildGeometryInfo, accelStructureBuildRangeInfos.data());
		VulkanCommandBuffer::flushSingleTimeCommands(m_VulkanCorePtr, cmd, true);

		//delete_scratch_buffer(scratch_buffer);
		scratchBufferPtr.reset();

		m_AccelStructure_Bottom_Ptrs.push_back(accelStructure_Bottom_Ptr);

		return true;
	}

	return false;
}

void ShaderPass::DestroyBottomLevelAccelerationStructureForMesh()
{
	m_Device.waitIdle();

	for (auto& accelPtr : m_AccelStructure_Bottom_Ptrs)
	{
		if (accelPtr)
			m_Device.destroyAccelerationStructureKHR(accelPtr->handle);
	}
	m_AccelStructure_Bottom_Ptrs.clear();
}

bool ShaderPass::CreateTopLevelAccelerationStructure(std::vector<vk::AccelerationStructureInstanceKHR>& vBlasInstances)
{
	auto instancesBufferPtr = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr,
		sizeof(vk::AccelerationStructureInstanceKHR) * vBlasInstances.size(),
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR,
		VMA_MEMORY_USAGE_CPU_TO_GPU);
	VulkanRessource::upload(m_VulkanCorePtr, *instancesBufferPtr,
		vBlasInstances.data(), sizeof(vk::AccelerationStructureInstanceKHR) * vBlasInstances.size());

	vk::BufferDeviceAddressInfoKHR instancesBufferDeviceAddressInfo{};
	instancesBufferDeviceAddressInfo.buffer = instancesBufferPtr->buffer;
	auto instancesBufferAddress = m_Device.getBufferAddressKHR(&instancesBufferDeviceAddressInfo);

	vk::DeviceOrHostAddressConstKHR instance_data_device_address{};
	instance_data_device_address.deviceAddress = instancesBufferAddress;

	// The top level acceleration structure contains (bottom level) instance as the input geometry
	vk::AccelerationStructureGeometryKHR accelStructureGeometry;
	accelStructureGeometry.geometryType = vk::GeometryTypeKHR::eInstances;
	accelStructureGeometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
	accelStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
	accelStructureGeometry.geometry.instances.data = instance_data_device_address;

	// Get the size requirements for buffers involved in the acceleration structure build process
	vk::AccelerationStructureBuildGeometryInfoKHR accelStructureBuildGeometryInfo;
	accelStructureBuildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	accelStructureBuildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	accelStructureBuildGeometryInfo.geometryCount = 1;
	accelStructureBuildGeometryInfo.pGeometries = &accelStructureGeometry;

	const auto primitive_count = static_cast<uint32_t>(vBlasInstances.size());

	auto accelStructureBuildSizeInfo = m_Device.getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice,
		accelStructureBuildGeometryInfo, primitive_count);

	// Create a buffer to hold the acceleration structure
	m_AccelStructure_Top_Ptr = VulkanRessource::createAccelStructureBufferObject(m_VulkanCorePtr,
		accelStructureBuildSizeInfo.accelerationStructureSize,
		VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);

	// Create the acceleration structure
	vk::AccelerationStructureCreateInfoKHR accelStructureCreateInfo;
	accelStructureCreateInfo.buffer = m_AccelStructure_Top_Ptr->buffer;
	accelStructureCreateInfo.size = accelStructureBuildSizeInfo.accelerationStructureSize;
	accelStructureCreateInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	m_AccelStructure_Top_Ptr->handle = m_Device.createAccelerationStructureKHR(accelStructureCreateInfo);

	// The actual build process starts here

	// Create a scratch buffer as a temporary storage for the acceleration structure build
	auto scratchBufferPtr = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr,
		accelStructureBuildSizeInfo.accelerationStructureSize,
		vk::BufferUsageFlagBits::eShaderDeviceAddressKHR | vk::BufferUsageFlagBits::eStorageBuffer,
		VMA_MEMORY_USAGE_CPU_TO_GPU);

	vk::BufferDeviceAddressInfoKHR scratchBufferDeviceAddressInfo{};
	scratchBufferDeviceAddressInfo.buffer = scratchBufferPtr->buffer;
	auto scratchBufferAddress = m_Device.getBufferAddressKHR(&scratchBufferDeviceAddressInfo);

	vk::AccelerationStructureBuildGeometryInfoKHR accelBuildGeometryInfo;
	accelBuildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	accelBuildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	accelBuildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
	accelBuildGeometryInfo.dstAccelerationStructure = m_AccelStructure_Top_Ptr->handle;
	accelBuildGeometryInfo.geometryCount = 1;
	accelBuildGeometryInfo.pGeometries = &accelStructureGeometry;
	accelBuildGeometryInfo.scratchData.deviceAddress = scratchBufferAddress;

	vk::AccelerationStructureBuildRangeInfoKHR accelStructureBuildRangeInfo;
	accelStructureBuildRangeInfo.primitiveCount = primitive_count;
	accelStructureBuildRangeInfo.primitiveOffset = 0;
	accelStructureBuildRangeInfo.firstVertex = 0;
	accelStructureBuildRangeInfo.transformOffset = 0;
	std::vector<vk::AccelerationStructureBuildRangeInfoKHR*> accelStructureBuildRangeInfos =
	{ &accelStructureBuildRangeInfo };

	// Build the acceleration structure on the device via a one-time command buffer submission
	auto cmd = VulkanCommandBuffer::beginSingleTimeCommands(m_VulkanCorePtr, true);
	cmd.buildAccelerationStructuresKHR(1, &accelBuildGeometryInfo, accelStructureBuildRangeInfos.data());
	VulkanCommandBuffer::flushSingleTimeCommands(m_VulkanCorePtr, cmd, true);

	//delete_scratch_buffer(scratch_buffer);
	scratchBufferPtr.reset();

	return true;
}

void ShaderPass::DestroyTopLevelAccelerationStructure()
{
	m_Device.waitIdle();
	m_Device.destroyAccelerationStructureKHR(m_AccelStructure_Top_Ptr->handle);
	m_AccelStructure_Top_Ptr.reset();
}

vk::AccelerationStructureInstanceKHR ShaderPass::CreateBlasInstance(const uint32_t& blas_id, glm::mat4& mat)
{
	VkTransformMatrixKHR transform_matrix;
	glm::mat3x4          rtxT = glm::transpose(mat);
	memcpy(&transform_matrix, glm::value_ptr(rtxT), sizeof(VkTransformMatrixKHR));

	auto blasPtr = m_AccelStructure_Bottom_Ptrs[blas_id];
	if (blasPtr)
	{
		// Get the bottom acceleration structure's handle, which will be used during the top level acceleration build
		vk::AccelerationStructureDeviceAddressInfoKHR accelDeviceAddressInfo;
		accelDeviceAddressInfo.accelerationStructure = blasPtr->handle;
		auto device_address = m_Device.getAccelerationStructureAddressKHR(accelDeviceAddressInfo);

		vk::AccelerationStructureInstanceKHR blas_instance{};
		blas_instance.transform = transform_matrix;
		blas_instance.instanceCustomIndex = blas_id;
		blas_instance.mask = 0xFF;
		blas_instance.instanceShaderBindingTableRecordOffset = 0;
		blas_instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		blas_instance.accelerationStructureReference = device_address;

		return blas_instance;
	}

	return vk::AccelerationStructureInstanceKHR{};
}

bool ShaderPass::CreateRtxPipeline()
{
	// Slot for binding top level acceleration structures to the ray generation shader
	VkDescriptorSetLayoutBinding acceleration_structure_layout_binding{};
	acceleration_structure_layout_binding.binding = 0;
	acceleration_structure_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	acceleration_structure_layout_binding.descriptorCount = 1;
	acceleration_structure_layout_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

	VkDescriptorSetLayoutBinding result_image_layout_binding{};
	result_image_layout_binding.binding = 1;
	result_image_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	result_image_layout_binding.descriptorCount = 1;
	result_image_layout_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	VkDescriptorSetLayoutBinding uniform_buffer_binding{};
	uniform_buffer_binding.binding = 2;
	uniform_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniform_buffer_binding.descriptorCount = 1;
	uniform_buffer_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	// Scene description
	VkDescriptorSetLayoutBinding scene_buffer_binding{};
	scene_buffer_binding.binding = 3;
	scene_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	scene_buffer_binding.descriptorCount = 1;
	scene_buffer_binding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

	std::vector<VkDescriptorSetLayoutBinding> bindings = {
		acceleration_structure_layout_binding,
		result_image_layout_binding,
		uniform_buffer_binding,
		scene_buffer_binding,
	};

	VkDescriptorSetLayoutCreateInfo layout_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
	layout_info.pBindings = bindings.data();
	VK_CHECK(vkCreateDescriptorSetLayout(get_device().get_handle(), &layout_info, nullptr, &descriptor_set_layout));

	VkPipelineLayoutCreateInfo pipeline_layout_create_info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipeline_layout_create_info.setLayoutCount = 1;
	pipeline_layout_create_info.pSetLayouts = &descriptor_set_layout;

	VK_CHECK(vkCreatePipelineLayout(get_device().get_handle(), &pipeline_layout_create_info, nullptr, &pipeline_layout));

	// Ray tracing shaders + buffer reference require SPIR-V 1.5, so we need to set the appropriate target environment for the glslang compiler
	vkb::GLSLCompiler::set_target_environment(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);

	/*
		Setup ray tracing shader groups
		Each shader group points at the corresponding shader in the pipeline
	*/
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

	// Ray generation group
	{
		shader_stages.push_back(load_shader("ray_tracing_reflection/raygen.rgen", VK_SHADER_STAGE_RAYGEN_BIT_KHR));
		VkRayTracingShaderGroupCreateInfoKHR raygen_group_ci{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
		raygen_group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		raygen_group_ci.generalShader = static_cast<uint32_t>(shader_stages.size()) - 1;
		raygen_group_ci.closestHitShader = VK_SHADER_UNUSED_KHR;
		raygen_group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
		raygen_group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups.push_back(raygen_group_ci);
	}

	// Ray miss group
	{
		shader_stages.push_back(load_shader("ray_tracing_reflection/miss.rmiss", VK_SHADER_STAGE_MISS_BIT_KHR));
		VkRayTracingShaderGroupCreateInfoKHR miss_group_ci{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
		miss_group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		miss_group_ci.generalShader = static_cast<uint32_t>(shader_stages.size()) - 1;
		miss_group_ci.closestHitShader = VK_SHADER_UNUSED_KHR;
		miss_group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
		miss_group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups.push_back(miss_group_ci);
	}

	// Ray miss (shadow) group
	{
		shader_stages.push_back(load_shader("ray_tracing_reflection/missShadow.rmiss", VK_SHADER_STAGE_MISS_BIT_KHR));
		VkRayTracingShaderGroupCreateInfoKHR miss_group_ci{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
		miss_group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		miss_group_ci.generalShader = static_cast<uint32_t>(shader_stages.size()) - 1;
		miss_group_ci.closestHitShader = VK_SHADER_UNUSED_KHR;
		miss_group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
		miss_group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups.push_back(miss_group_ci);
	}

	// Ray closest hit group
	{
		shader_stages.push_back(load_shader("ray_tracing_reflection/closesthit.rchit", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));
		VkRayTracingShaderGroupCreateInfoKHR closes_hit_group_ci{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
		closes_hit_group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		closes_hit_group_ci.generalShader = VK_SHADER_UNUSED_KHR;
		closes_hit_group_ci.closestHitShader = static_cast<uint32_t>(shader_stages.size()) - 1;
		closes_hit_group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
		closes_hit_group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups.push_back(closes_hit_group_ci);
	}

	/*
		Create the ray tracing pipeline
	*/
	VkRayTracingPipelineCreateInfoKHR raytracing_pipeline_create_info{ VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR };
	raytracing_pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
	raytracing_pipeline_create_info.pStages = shader_stages.data();
	raytracing_pipeline_create_info.groupCount = static_cast<uint32_t>(shader_groups.size());
	raytracing_pipeline_create_info.pGroups = shader_groups.data();
	raytracing_pipeline_create_info.maxPipelineRayRecursionDepth = 2;
	raytracing_pipeline_create_info.layout = pipeline_layout;
	VK_CHECK(vkCreateRayTracingPipelinesKHR(get_device().get_handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &raytracing_pipeline_create_info, nullptr, &pipeline));

	return false;
}

void ShaderPass::DestroyRtxPipeline()
{
	vkDestroyPipeline(get_device().get_handle(), pipeline, nullptr);
	vkDestroyPipelineLayout(get_device().get_handle(), pipeline_layout, nullptr);
	vkDestroyDescriptorSetLayout(get_device().get_handle(), descriptor_set_layout, nullptr);
	vkDestroyImageView(get_device().get_handle(), storage_image.view, nullptr);
	vkDestroyImage(get_device().get_handle(), storage_image.image, nullptr);
	vkFreeMemory(get_device().get_handle(), storage_image.memory, nullptr);
	delete_acceleration_structure(top_level_acceleration_structure);
	for (auto& b : bottom_level_acceleration_structure)
	{
		delete_acceleration_structure(b);
	}

	for (auto& obj : obj_models)
	{
		obj.vertex_buffer.reset();
		obj.index_buffer.reset();
		obj.mat_color_buffer.reset();
		obj.mat_index_buffer.reset();
	}

	ubo.reset();
}

bool ShaderPass::CreateShaderBindingTable()
{
	// Index position of the groups in the generated ray tracing pipeline
	// To be generic, this should be pass in parameters
	std::vector<uint32_t> rgen_index{ 0 };
	std::vector<uint32_t> miss_index{ 1, 2 };
	std::vector<uint32_t> hit_index{ 3 };

	const uint32_t handle_size = ray_tracing_pipeline_properties.shaderGroupHandleSize;
	const uint32_t handle_alignment = ray_tracing_pipeline_properties.shaderGroupHandleAlignment;
	const uint32_t handle_size_aligned = aligned_size(handle_size, handle_alignment);

	const VkBufferUsageFlags sbt_buffer_usage_flags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	const VmaMemoryUsage     sbt_memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	// Create binding table buffers for each shader type
	raygen_shader_binding_table = std::make_unique<vkb::core::Buffer>(get_device(), handle_size_aligned * rgen_index.size(), sbt_buffer_usage_flags, sbt_memory_usage, 0);
	miss_shader_binding_table = std::make_unique<vkb::core::Buffer>(get_device(), handle_size_aligned * miss_index.size(), sbt_buffer_usage_flags, sbt_memory_usage, 0);
	hit_shader_binding_table = std::make_unique<vkb::core::Buffer>(get_device(), handle_size_aligned * hit_index.size(), sbt_buffer_usage_flags, sbt_memory_usage, 0);

	// Copy the pipeline's shader handles into a host buffer
	const auto           group_count = static_cast<uint32_t>(rgen_index.size() + miss_index.size() + hit_index.size());
	const auto           sbt_size = group_count * handle_size_aligned;
	std::vector<uint8_t> shader_handle_storage(sbt_size);
	VK_CHECK(vkGetRayTracingShaderGroupHandlesKHR(get_device().get_handle(), pipeline, 0, group_count, sbt_size, shader_handle_storage.data()));

	// Write the handles in the SBT buffer
	auto copyHandles = [&](auto& buffer, std::vector<uint32_t>& indices, uint32_t stride) {
		auto* pBuffer = static_cast<uint8_t*>(buffer->map());
		for (uint32_t index = 0; index < static_cast<uint32_t>(indices.size()); index++)
		{
			auto* pStart = pBuffer;
			// Copy the handle
			memcpy(pBuffer, shader_handle_storage.data() + (indices[index] * handle_size), handle_size);
			pBuffer = pStart + stride;        // Jumping to next group
		}
		buffer->unmap();
	};

	copyHandles(raygen_shader_binding_table, rgen_index, handle_size_aligned);
	copyHandles(miss_shader_binding_table, miss_index, handle_size_aligned);
	copyHandles(hit_shader_binding_table, hit_index, handle_size_aligned);

	return false;
}

void ShaderPass::DestroyShaderBindingTable()
{

}