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

#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <glslang/glslang/MachineIndependent/localintermediate.h>

#include <unordered_map>
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <array>

class VulkanShader
{
public:
	typedef std::function<void(std::string, std::string, std::string)> ShaderMessagingFunction;
	typedef std::function<void(glslang::TIntermediate*)> TraverserFunction;

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

public: // errors
	std::unordered_map<EShLanguage, std::vector<std::string>> m_Error;
	std::unordered_map<EShLanguage, std::vector<std::string>> m_Warnings;

public:
	VulkanShader(); // Prevent construction
	VulkanShader(const VulkanShader&) {}; // Prevent construction by copying
	VulkanShader& operator =(const VulkanShader&) { return *this; }; // Prevent assignment
	~VulkanShader(); // Prevent unwanted destruction

public:
	void Init();
	void Unit();
};
