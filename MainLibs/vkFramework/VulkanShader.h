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

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vkFramework/vkFramework.h>
#include <glslang/glslang/MachineIndependent/localintermediate.h>

#include <unordered_map>
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <array>

/*
todo : to Refactoa and Convert for use of Vulkan.hpp
*/

class VulkanShader
{
public:
	static VulkanShaderPtr Create();

public:
	typedef std::function<void(std::string, std::string, std::string)> ShaderMessagingFunction;
	typedef std::function<void(glslang::TIntermediate*)> TraverserFunction;

public: // errors
	std::unordered_map<EShLanguage, std::vector<std::string>> m_Error;
	std::unordered_map<EShLanguage, std::vector<std::string>> m_Warnings;

public:
	const std::vector<unsigned int> CompileGLSLFile(
		const std::string& filename,
		ShaderMessagingFunction vMessagingFunction = nullptr,
		std::string* vShaderCode = nullptr,
		std::unordered_map<std::string, bool>* vUsedUniforms = nullptr);
	const std::vector<unsigned int> CompileGLSLString(
		const std::string& vCode,
		std::string vShaderSuffix,
		const std::string& vOriginalFileName,
		ShaderMessagingFunction vMessagingFunction = nullptr,
		std::string* vShaderCode = nullptr,
		std::unordered_map<std::string, bool>* vUsedUniforms = nullptr);
	void ParseGLSLString(
		const std::string& vCode,
		std::string vShaderSuffix,
		const std::string& vOriginalFileName,
		std::string vEntryPoint,
		ShaderMessagingFunction vMessagingFunction,
		TraverserFunction vTraverser);
	VkShaderModule CreateShaderModule(VkDevice vLogicalDevice, std::vector<unsigned int> vSPIRVCode);
	void DestroyShaderModule(VkDevice vLogicalDevice, VkShaderModule vVkShaderModule);
	std::unordered_map<std::string, bool> CollectUniformInfosFromIR(const glslang::TIntermediate& intermediate);

public:
	bool Init();
	void Unit();

public:
	VulkanShader(); // Prevent construction
	VulkanShader(const VulkanShader&) {}; // Prevent construction by copying
	VulkanShader& operator =(const VulkanShader&) { return *this; }; // Prevent assignment
	~VulkanShader(); // Prevent unwanted destruction

};
