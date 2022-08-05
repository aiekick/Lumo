// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include <SceneGraph/SceneAccelStructure.h>
#include <vkFramework/VulkanCommandBuffer.h>
#include <glm/gtc/type_ptr.hpp>
using namespace vkApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

SceneAccelStructurePtr SceneAccelStructure::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	if (vVulkanCorePtr && vVulkanCorePtr->getDevice())
	{
		auto res = std::make_shared<SceneAccelStructure>(vVulkanCorePtr);
		res->m_This = res;
		return res;
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////
//// PUBLIC : BUILD / CLEAR //////////////////////////////////
//////////////////////////////////////////////////////////////

SceneAccelStructure::SceneAccelStructure(vkApi::VulkanCorePtr vVulkanCorePtr)
	: m_VulkanCorePtr(vVulkanCorePtr), m_Device(vVulkanCorePtr->getDevice())
{

}

SceneAccelStructure::~SceneAccelStructure()
{
	Clear();
}

bool SceneAccelStructure::BuildForModel(SceneModelWeak vSceneModelWeak)
{
	Clear();

	m_ModelAdressesBufferInfo = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0U, VK_WHOLE_SIZE };

	auto modelPtr = vSceneModelWeak.getValidShared();
	if (modelPtr &&
		!modelPtr->empty())
	{
		std::vector<SceneMesh::SceneMeshBuffers> modelBufferAddresses;

		m_SuccessfullyBuilt = true;

		for (auto meshPtr : *modelPtr)
		{
			if (meshPtr)
			{
				SceneMesh::SceneMeshBuffers buffer;
				buffer.vertices_address = meshPtr->GetVerticesDeviceAddress();
				buffer.indices_address = meshPtr->GetIndiceDeviceAddress();
				modelBufferAddresses.push_back(buffer);

				m_SuccessfullyBuilt &= CreateBottomLevelAccelerationStructureForMesh(meshPtr);
			}
		}

		if (m_SuccessfullyBuilt && 
			!m_AccelStructure_Bottom_Ptrs.empty())
		{
			vk::BufferUsageFlags bufferUsageFlags = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;

			auto sizeInBytes = modelPtr->size() * sizeof(SceneMesh::SceneMeshBuffers);
			m_ModelAdressesPtr = VulkanRessource::createStorageBufferObject(
				m_VulkanCorePtr, sizeInBytes,
				bufferUsageFlags, VMA_MEMORY_USAGE_CPU_TO_GPU);
			if (m_ModelAdressesPtr)
			{
				VulkanRessource::upload(m_VulkanCorePtr, m_ModelAdressesPtr, modelBufferAddresses.data(), sizeInBytes);

				m_ModelAdressesBufferInfo.buffer = m_ModelAdressesPtr->buffer;
				m_ModelAdressesBufferInfo.offset = 0U;
				m_ModelAdressesBufferInfo.range = sizeInBytes;

				vk::DescriptorBufferInfo m_DescriptorBufferInfo_Vert;
				glm::mat4 m_model_pos = glm::mat4(1.0f);

				// we could create in few time a sepcila input for create instance of some model (transformed)
				// but for the moment only one
				std::vector<vk::AccelerationStructureInstanceKHR> blas_instances;
				blas_instances.emplace_back(CreateBlasInstance(0, m_model_pos));

				m_SuccessfullyBuilt &= CreateTopLevelAccelerationStructure(blas_instances);

				return m_SuccessfullyBuilt;
			}
		}
	}

	return false;
}

void SceneAccelStructure::Clear()
{
	DestroyBottomLevelAccelerationStructureForMesh();
	DestroyTopLevelAccelerationStructure();
	m_SuccessfullyBuilt = false;
}

vk::WriteDescriptorSetAccelerationStructureKHR* SceneAccelStructure::GetTLASInfo()
{
	if (IsOk())
	{
		return &m_AccelStructureTopDescriptorInfo;
	}

	return nullptr;
}

vk::DescriptorBufferInfo* SceneAccelStructure::GetBufferAddressInfo()
{
	if (IsOk())
	{
		return &m_ModelAdressesBufferInfo;
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////
//// PRIVATE : BUILD /////////////////////////////////////////
//////////////////////////////////////////////////////////////

// will convert model in accel struct
bool SceneAccelStructure::CreateBottomLevelAccelerationStructureForMesh(const SceneMeshWeak& vMesh)
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
		VulkanRessource::upload(m_VulkanCorePtr, transformMatrixBufferPtr, &transform_matrix, sizeof(transform_matrix));

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

		return (accelStructure_Bottom_Ptr != nullptr);
	}

	return false;
}

void SceneAccelStructure::DestroyBottomLevelAccelerationStructureForMesh()
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

bool SceneAccelStructure::CreateTopLevelAccelerationStructure(const std::vector<vk::AccelerationStructureInstanceKHR>& vBlasInstances)
{
	auto blasInstances = vBlasInstances;

	auto instancesBufferPtr = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr,
		sizeof(vk::AccelerationStructureInstanceKHR) * blasInstances.size(),
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		VMA_MEMORY_USAGE_CPU_TO_GPU);
	VulkanRessource::upload(m_VulkanCorePtr, instancesBufferPtr,
		blasInstances.data(), sizeof(vk::AccelerationStructureInstanceKHR) * blasInstances.size());

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

	const auto primitive_count = static_cast<uint32_t>(blasInstances.size());

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

	return (m_AccelStructure_Top_Ptr != nullptr);
}

void SceneAccelStructure::DestroyTopLevelAccelerationStructure()
{
	m_Device.waitIdle();
	if (m_AccelStructure_Top_Ptr)
	{
		m_Device.destroyAccelerationStructureKHR(m_AccelStructure_Top_Ptr->handle);
	}
	m_AccelStructure_Top_Ptr.reset();
}

vk::AccelerationStructureInstanceKHR SceneAccelStructure::CreateBlasInstance(const uint32_t& blas_id, glm::mat4& mat)
{
	vk::TransformMatrixKHR transform_matrix;
	glm::mat3x4          rtxT = glm::transpose(mat);
	memcpy(&transform_matrix, glm::value_ptr(rtxT), sizeof(vk::TransformMatrixKHR));

	auto blasPtr = m_AccelStructure_Bottom_Ptrs[(size_t)blas_id];
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