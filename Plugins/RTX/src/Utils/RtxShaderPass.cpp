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

#include "RtxShaderPass.h"

#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanCommandBuffer.h>
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan.hpp>
#include <map>

using namespace vkApi;

RtxShaderPass::RtxShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr, GenericType::RTX)
{

}

RtxShaderPass::RtxShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool)
	: ShaderPass(vVulkanCorePtr, GenericType::RTX, vCommandPool, vDescriptorPool)
{
	
}

void RtxShaderPass::ActionBeforeInit()
{
	auto devPtr = m_VulkanCorePtr->getFrameworkDevice().getValidShared();
	if (devPtr)
	{
		m_RayTracingPipelineProperties = devPtr->m_RayTracingDeviceProperties;
	}
}

uint32_t RtxShaderPass::GetAlignedSize(uint32_t value, uint32_t alignment)
{
	return (value + alignment - 1) & ~(alignment - 1);
}

void RtxShaderPass::TraceRays(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (m_Pipeline && m_PipelineLayout)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_Pipeline);
		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_PipelineLayout, 0, m_DescriptorSet, nullptr);
		vCmdBuffer->traceRaysKHR(
			m_RayGenShaderSbtEntry,
			m_MissShaderSbtEntry,
			m_HitShaderSbtEntry,
			m_CallableShaderSbtEntry,
			m_DispatchSize.x,
			m_DispatchSize.y,
			1);
	}
}

bool RtxShaderPass::BuildModel()
{
	ZoneScoped;

	return true;
}

void RtxShaderPass::DestroyModel(const bool& vReleaseDatas)
{
	ZoneScoped;

	m_VulkanCorePtr->getDevice().waitIdle();
}


// will convert model in accel struct
bool RtxShaderPass::CreateBottomLevelAccelerationStructureForMesh(SceneMeshWeak vMesh)
{
	auto meshPtr = vMesh.getValidShared();
	if (meshPtr)
	{
		auto buffer_usage_flags =
			vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
			vk::BufferUsageFlagBits::eShaderDeviceAddress;

		vk::TransformMatrixKHR transform_matrix =
			std::array<std::array<float, 4>, 3>
		{
			1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f
		};

		auto transformMatrixBufferPtr = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr, sizeof(transform_matrix),
			vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			VMA_MEMORY_USAGE_CPU_TO_GPU);
		VulkanRessource::upload(m_VulkanCorePtr, *transformMatrixBufferPtr, &transform_matrix, sizeof(transform_matrix));

		vk::AccelerationStructureGeometryTrianglesDataKHR triangles;
		triangles.vertexFormat = vk::Format::eR32G32B32Sfloat;
		triangles.vertexData = meshPtr->GetVerticesDeviceAddress();
		triangles.maxVertex = meshPtr->GetVerticesCount();
		triangles.vertexStride = sizeof(VertexStruct::P3_N3_TA3_BTA3_T2_C4);
		triangles.indexType = vk::IndexType::eUint32;
		triangles.indexData = meshPtr->GetIndiceDeviceAddress();
		triangles.transformData = transformMatrixBufferPtr->device_address;

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
			vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
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

void RtxShaderPass::DestroyBottomLevelAccelerationStructureForMesh()
{
	m_Device.waitIdle();

	for (auto& accelPtr : m_AccelStructure_Bottom_Ptrs)
	{
		if (accelPtr)
		{
			m_Device.destroyAccelerationStructureKHR(accelPtr->handle);
		}
	}

	m_AccelStructure_Bottom_Ptrs.clear();
}

bool RtxShaderPass::CreateTopLevelAccelerationStructure(std::vector<vk::AccelerationStructureInstanceKHR>& vBlasInstances)
{
	auto instancesBufferPtr = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr,
		sizeof(vk::AccelerationStructureInstanceKHR) * vBlasInstances.size(),
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		VMA_MEMORY_USAGE_CPU_TO_GPU);
	VulkanRessource::upload(m_VulkanCorePtr, *instancesBufferPtr,
		vBlasInstances.data(), sizeof(vk::AccelerationStructureInstanceKHR) * vBlasInstances.size());

	vk::DeviceOrHostAddressConstKHR instance_data_device_address{};
	instance_data_device_address.deviceAddress = instancesBufferPtr->device_address;

	// The top level acceleration structure contains (bottom level) instance as the input geometry
	vk::AccelerationStructureGeometryKHR accelStructureGeometry;
	accelStructureGeometry.geometryType = vk::GeometryTypeKHR::eInstances;
	accelStructureGeometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
	accelStructureGeometry.geometry.instances = vk::AccelerationStructureGeometryInstancesDataKHR{};
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

	// for the writeDescriptorSets
	m_AccelStructureTopDescriptorInfo = vk::WriteDescriptorSetAccelerationStructureKHR{};
	m_AccelStructureTopDescriptorInfo.accelerationStructureCount = 1;
	m_AccelStructureTopDescriptorInfo.pAccelerationStructures = &m_AccelStructure_Top_Ptr->handle;

	// The actual build process starts here

	// Create a scratch buffer as a temporary storage for the acceleration structure build
	auto scratchBufferPtr = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr,
		accelStructureBuildSizeInfo.accelerationStructureSize,
		vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
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

void RtxShaderPass::DestroyTopLevelAccelerationStructure()
{
	m_Device.waitIdle();
	if (m_AccelStructure_Top_Ptr)
	{
		m_Device.destroyAccelerationStructureKHR(m_AccelStructure_Top_Ptr->handle);
	}
	m_AccelStructure_Top_Ptr.reset();
}

vk::AccelerationStructureInstanceKHR RtxShaderPass::CreateBlasInstance(const uint32_t& blas_id, glm::mat4& mat)
{
	vk::TransformMatrixKHR transform_matrix;
	glm::mat3x4          rtxT = glm::transpose(mat);
	memcpy(&transform_matrix, glm::value_ptr(rtxT), sizeof(vk::TransformMatrixKHR));

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

bool RtxShaderPass::CreateRtxPipeline()
{
	ZoneScoped;

	std::vector<std::pair<ShaderId, vk::ShaderStageFlagBits>> m_ShaderIds =
	{
		std::pair<ShaderId, vk::ShaderStageFlagBits>(ShaderId::eRtxRayGen, vk::ShaderStageFlagBits::eRaygenKHR),
		std::pair<ShaderId, vk::ShaderStageFlagBits>(ShaderId::eRtxRayInt, vk::ShaderStageFlagBits::eIntersectionKHR),
		std::pair<ShaderId, vk::ShaderStageFlagBits>(ShaderId::eRtxRayMiss, vk::ShaderStageFlagBits::eMissKHR),
		std::pair<ShaderId, vk::ShaderStageFlagBits>(ShaderId::eRtxRayAnyHit, vk::ShaderStageFlagBits::eAnyHitKHR),
		std::pair<ShaderId, vk::ShaderStageFlagBits>(ShaderId::eRtxRayClosestHit, vk::ShaderStageFlagBits::eClosestHitKHR)
	};

	for (const auto& shaderId : m_ShaderIds)
	{
		if (m_ShaderCodes[shaderId.first].m_Used &&
			m_ShaderCodes[shaderId.first].m_SPIRV.empty())
			return false;
	}

	std::vector<vk::PushConstantRange> push_constants;
	if (m_Internal_PushConstants.size)
	{
		push_constants.push_back(m_Internal_PushConstants);
	}

	m_PipelineLayout =
		m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			1, &m_DescriptorSetLayout,
			(uint32_t)push_constants.size(), push_constants.data()
		));

	m_ShaderCreateInfos.clear();
	m_RayTracingShaderGroups.clear();
	for (const auto& shaderId : m_ShaderIds)
	{
		if (m_ShaderCodes[shaderId.first].m_Used)
		{
			m_ShaderCodes[shaderId.first].m_ShaderModule =
				vkApi::VulkanCore::sVulkanShader->CreateShaderModule(
					(vk::Device)m_Device, m_ShaderCodes[shaderId.first].m_SPIRV);

			if (m_ShaderCodes[shaderId.first].m_ShaderModule)
			{
				auto shaderIndex = static_cast<uint32_t>(m_ShaderCreateInfos.size());

				if (shaderId.first == ShaderId::eRtxRayGen ||
					shaderId.first == ShaderId::eRtxRayMiss)
				{
					vk::RayTracingShaderGroupCreateInfoKHR shaderGroup;
					shaderGroup.type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
					shaderGroup.generalShader = shaderIndex;
					shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
					shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
					shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
					m_RayTracingShaderGroups.push_back(shaderGroup);
				}
				else if (shaderId.first == ShaderId::eRtxRayClosestHit ||
					shaderId.first == ShaderId::eRtxRayAnyHit)
				{
					vk::RayTracingShaderGroupCreateInfoKHR shaderGroup;
					shaderGroup.type = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
					if (shaderId.first == ShaderId::eRtxRayAnyHit)
					{
						shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
						shaderGroup.anyHitShader = shaderIndex;
						shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
						shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
					}
					else //if (shaderId.first == ShaderId::eRtxRayClosestHit)
					{
						shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
						shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
						shaderGroup.closestHitShader = shaderIndex;
						shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
					}
					m_RayTracingShaderGroups.push_back(shaderGroup);
				}
				else if (shaderId.first == ShaderId::eRtxRayInt)
				{
					vk::RayTracingShaderGroupCreateInfoKHR shaderGroup;
					shaderGroup.type = vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup;
					shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
					shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
					shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
					shaderGroup.intersectionShader = shaderIndex;
					m_RayTracingShaderGroups.push_back(shaderGroup);
				}

				m_ShaderCreateInfos.push_back(
					vk::PipelineShaderStageCreateInfo(
						vk::PipelineShaderStageCreateFlags(),
						shaderId.second,
						m_ShaderCodes[shaderId.first].m_ShaderModule, "main"
					)
				);
			}
		}
	}

	vk::RayTracingPipelineCreateInfoKHR rayTracingPipeInfo = vk::RayTracingPipelineCreateInfoKHR();
	rayTracingPipeInfo.stageCount = static_cast<uint32_t>(m_ShaderCreateInfos.size());
	rayTracingPipeInfo.pStages = m_ShaderCreateInfos.data();
	rayTracingPipeInfo.groupCount = static_cast<uint32_t>(m_RayTracingShaderGroups.size());
	rayTracingPipeInfo.pGroups = m_RayTracingShaderGroups.data();
	rayTracingPipeInfo.maxPipelineRayRecursionDepth = 2;
	rayTracingPipeInfo.layout = m_PipelineLayout;

	m_Pipeline = m_Device.createRayTracingPipelineKHR(nullptr, m_PipelineCache, rayTracingPipeInfo).value;

	// destroy modules
	for (const auto& shaderId : m_ShaderIds)
	{
		if (m_ShaderCodes[shaderId.first].m_Used &&
			m_ShaderCodes[shaderId.first].m_ShaderModule)
		{
			vkApi::VulkanCore::sVulkanShader->DestroyShaderModule(
				(vk::Device)m_Device, m_ShaderCodes[shaderId.first].m_ShaderModule);
		}
	}

	if (m_Pipeline && m_PipelineLayout)
	{
		return CreateShaderBindingTable();
	}

	return false;
}

bool RtxShaderPass::CreateShaderBindingTable()
{
	// Index position of the groups in the generated ray tracing pipeline
	// To be generic, this should be pass in parameters
	// todo, faire attention a correler ces ids avec ce qui est fait dans CreateRtxPipeline
	// sion le driver peut crasher le pc apres la commande de rendu traceRaysKHR
	std::vector<uint32_t> rgen_index{ 0 };
	std::vector<uint32_t> miss_index{ 1 };
	std::vector<uint32_t> hit_index{ 2 };

	const uint32_t handle_size = m_RayTracingPipelineProperties.shaderGroupHandleSize;
	const uint32_t handle_alignment = m_RayTracingPipelineProperties.shaderGroupHandleAlignment;
	const uint32_t handle_size_aligned = GetAlignedSize(handle_size, handle_alignment);

	// Create binding table buffers for each shader type
	const auto bufferUsageFlags =
		vk::BufferUsageFlagBits::eShaderBindingTableKHR | 
		vk::BufferUsageFlagBits::eTransferSrc | 
		vk::BufferUsageFlagBits::eShaderDeviceAddress;
	m_RayGenShaderBindingTablePtr = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr, handle_size_aligned * rgen_index.size(),
		bufferUsageFlags, VMA_MEMORY_USAGE_CPU_TO_GPU);
	m_RayMissShaderBindingTablePtr = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr, handle_size_aligned * miss_index.size(),
		bufferUsageFlags, VMA_MEMORY_USAGE_CPU_TO_GPU);
	m_RayHitShaderBindingTablePtr = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr, handle_size_aligned * hit_index.size(),
		bufferUsageFlags, VMA_MEMORY_USAGE_CPU_TO_GPU);

	if (m_RayGenShaderBindingTablePtr && 
		m_RayMissShaderBindingTablePtr && 
		m_RayHitShaderBindingTablePtr)
	{
		// Copy the pipeline's shader handles into a host buffer
		const auto           group_count = static_cast<uint32_t>(rgen_index.size() + miss_index.size() + hit_index.size());
		const auto           sbt_size = group_count * handle_size_aligned;
		std::vector<uint8_t> shader_handle_storage(sbt_size);

		if (VULKAN_HPP_DEFAULT_DISPATCHER.vkGetRayTracingShaderGroupHandlesKHR(
			m_Device, m_Pipeline, 0, group_count, sbt_size, shader_handle_storage.data()) == VkResult::VK_SUCCESS)
		{
			// Write the handles in the SBT buffer
			auto copyHandles = [&](VulkanBufferObjectPtr vBufferPtr, std::vector<uint32_t>& indices, uint32_t stride)
			{
				if (vBufferPtr)
				{
					void* mapped_dst = nullptr;
					VulkanCore::check_error(vmaMapMemory(vkApi::VulkanCore::sAllocator, vBufferPtr->alloc_meta, &mapped_dst));

					auto* pBuffer = static_cast<uint8_t*>(mapped_dst);
					for (uint32_t index = 0; index < static_cast<uint32_t>(indices.size()); index++)
					{
						auto* pStart = pBuffer;
						// Copy the handle
						memcpy(pBuffer, shader_handle_storage.data() + (indices[index] * handle_size), handle_size);
						pBuffer = pStart + stride;        // Jumping to next group
					}

					vmaUnmapMemory(vkApi::VulkanCore::sAllocator, vBufferPtr->alloc_meta);
				}
			};

			copyHandles(m_RayGenShaderBindingTablePtr, rgen_index, handle_size_aligned);
			copyHandles(m_RayMissShaderBindingTablePtr, miss_index, handle_size_aligned);
			copyHandles(m_RayHitShaderBindingTablePtr, hit_index, handle_size_aligned);

			const uint32_t handle_size_aligned = GetAlignedSize(
				m_RayTracingPipelineProperties.shaderGroupHandleSize,
				m_RayTracingPipelineProperties.shaderGroupHandleAlignment);

			m_RayGenShaderSbtEntry = vk::StridedDeviceAddressRegionKHR{};
			m_RayGenShaderSbtEntry.deviceAddress = m_RayGenShaderBindingTablePtr->device_address;
			m_RayGenShaderSbtEntry.stride = handle_size_aligned;
			m_RayGenShaderSbtEntry.size = handle_size_aligned; // * the count of shaders ion this category

			m_MissShaderSbtEntry = vk::StridedDeviceAddressRegionKHR{};
			m_MissShaderSbtEntry.deviceAddress = m_RayMissShaderBindingTablePtr->device_address;
			m_MissShaderSbtEntry.stride = handle_size_aligned;
			m_MissShaderSbtEntry.size = handle_size_aligned; // * the count of shaders ion this category

			m_HitShaderSbtEntry = vk::StridedDeviceAddressRegionKHR{};
			m_HitShaderSbtEntry.deviceAddress = m_RayHitShaderBindingTablePtr->device_address;
			m_HitShaderSbtEntry.stride = handle_size_aligned;
			m_HitShaderSbtEntry.size = handle_size_aligned; // * the count of shaders ion this category

			m_CallableShaderSbtEntry = vk::StridedDeviceAddressRegionKHR{};
			//m_CallableShaderSbtEntry.deviceAddress = ?? ->device_address;
			//m_CallableShaderSbtEntry.stride = handle_size_aligned;
			//m_CallableShaderSbtEntry.size = handle_size_aligned; // * the count of shaders ion this category

			return true;
		}
	}	

	return false;
}

void RtxShaderPass::DestroyShaderBindingTable()
{

}