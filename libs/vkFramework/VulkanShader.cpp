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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "VulkanShader.h"

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GLSL.std.450.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <StandAlone/ResourceLimits.h>
#include <glslang/Include/ShHandle.h>
#include <glslang/OSDependent/osinclude.h>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include "IRUniformsLocator.h"

#include <cstdio>			// printf, fprintf
#include <cstdlib>			// abort
#include <iostream>			// std::cout
#include <stdexcept>		// std::exception
#include <algorithm>		// std::min, std::max
#include <fstream>			// std::ifstream
#include <chrono>			// timer

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

#define VERBOSE_DEBUG

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

VulkanShader::ShaderMessagingFunction s_ShaderMessagingFunction = 0;

VulkanShaderPtr VulkanShader::Create()
{
	auto res = std::make_shared<VulkanShader>();
	if (!res->Init())
	{
		res.reset();
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

VulkanShader::VulkanShader()
{
	ZoneScoped;
}

VulkanShader::~VulkanShader()
{
	ZoneScoped;
}

bool VulkanShader::Init()
{
	ZoneScoped;

	return glslang::InitializeProcess();
}

void VulkanShader::Unit()
{
	ZoneScoped;

	glslang::FinalizeProcess();
}

std::string GetSuffix(const std::string& name)
{
	ZoneScoped;

	const size_t pos = name.rfind('.');
	return (pos == std::string::npos) ? "" : name.substr(name.rfind('.') + 1);
}

EShLanguage GetShaderStage(const std::string& stage)
{
	ZoneScoped;

	if (stage == "vert") {
		return EShLangVertex;
	}
	else if (stage == "ctrl") {
		return EShLangTessControl;
	}
	else if (stage == "eval") {
		return EShLangTessEvaluation;
	}
	else if (stage == "geom") {
		return EShLangGeometry;
	}
	else if (stage == "frag") {
		return EShLangFragment;
	}
	else if (stage == "comp") {
		return EShLangCompute;
	}
	else if (stage == "rgen") {
		return EShLangRayGen;
	}
	else if (stage == "rint") {
		return EShLangIntersect;
	}
	else if (stage == "miss") {
		return EShLangMiss;
	}
	else if (stage == "ahit") {
		return EShLangAnyHit;
	}
	else if (stage == "chit") {
		return EShLangClosestHit;
	}
	else {
		assert(0 && "Unknown shader stage");
		return EShLangCount;
	}
}

std::string GetShaderStageString(const EShLanguage& stage)
{
	ZoneScoped;

	switch (stage)
	{
	case EShLangVertex: return "vert";
	case EShLangTessControl: return "ctrl";
	case EShLangTessEvaluation: return "eval";
	case EShLangGeometry: return "geom";
	case EShLangFragment: return "frag";
	case EShLangCompute: return "comp";
	case EShLangRayGen: return "rgen";
	case EShLangIntersect: return "rint";
	case EShLangMiss: return "miss";
	case EShLangAnyHit: return "ahit";
	case EShLangClosestHit: return "chit";
	}
	return "";
}

std::string GetFullShaderStageString(const EShLanguage& stage)
{
	ZoneScoped;

	switch (stage)
	{
	case EShLangVertex: return "Vertex";
	case EShLangTessControl: return "Tesselation Control";
	case EShLangTessEvaluation: return "Tesselation Evaluation";
	case EShLangGeometry: return "Geometry";
	case EShLangFragment: return "Fragment";
	case EShLangCompute: return "Compute";
	case EShLangRayGen: return "Ray Generation";
	case EShLangIntersect: return "Ray Intersection";
	case EShLangMiss: return "Miss";
	case EShLangAnyHit: return "Any Hit";
	case EShLangClosestHit: return "Closest Hit";
	}
	return "";
}

//TODO: Multithread, manage SpirV that doesn't need recompiling (only recompile when dirty)
const std::vector<unsigned int> VulkanShader::CompileGLSLFile(
	const std::string& filename,
	const ShaderEntryPoint& vEntryPoint,
	ShaderMessagingFunction vMessagingFunction,
	std::string* vShaderCode,
	std::unordered_map<std::string, bool>* vUsedUniforms)
{
	ZoneScoped;

	std::vector<unsigned int> SpirV;

	//Load GLSL into a string
	std::ifstream file(filename);

	if (!file.is_open())
	{
		LogVarDebug("Debug : Failed to load shader %s", filename.c_str());
		return SpirV;
	}

	std::string InputGLSL((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	file.close();

	if (!InputGLSL.empty())
	{
		if (vShaderCode)
			*vShaderCode = InputGLSL;

		return CompileGLSLString(InputGLSL, GetSuffix(filename), filename, vEntryPoint, vMessagingFunction, vShaderCode, vUsedUniforms);
	}

	return SpirV;
}

const std::vector<unsigned int> VulkanShader::CompileGLSLString(
	const std::string& vCode,
	const std::string& vShaderSuffix,
	const std::string& vOriginalFileName,
	const ShaderEntryPoint& vEntryPoint,
	ShaderMessagingFunction vMessagingFunction,
	std::string* vShaderCode,
	std::unordered_map<std::string, bool>* vUsedUniforms)
{
	ZoneScoped;

	//LogVarDebug("Debug : ==== VulkanShader::CompileGLSLString (%s) =====", vShaderSuffix.c_str());

	m_Error.clear();
	m_Warnings.clear();

	std::vector<unsigned int> SpirV;

	std::string InputGLSL = vCode;

	EShLanguage shaderType = GetShaderStage(vShaderSuffix);

	if (!InputGLSL.empty() && shaderType != EShLanguage::EShLangCount)
	{
		glslang::TShader Shader(shaderType);
		
		if (vShaderCode)
			*vShaderCode = InputGLSL;

		//Set up Vulkan/SpirV Environment
		int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100
		//glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;  // would map to, say, Vulkan 1.0
		//glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;    // maps to, say, SPIR-V 1.0

		// RTX SUpport
		glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_2;  // would map to, say, Vulkan 1.0
		glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_4;    // maps to, say, SPIR-V 1.0

		Shader.setEnvInput(glslang::EShSourceGlsl, shaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
		Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
		Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);
		
		const char* InputCString = InputGLSL.c_str();

		Shader.setStrings(&InputCString, 1);

		EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

#ifdef _DEBUG
		messages = (EShMessages)(messages | EShMsgDebugInfo);
#endif
		//EShMessages messages = (EShMessages)(EShMsgDefault);

		const int DefaultVersion = 110; // 110 for desktop, 100 for es

		DirStackFileIncluder Includer;

		std::string PreprocessedGLSL;

		std::string shaderTypeString = GetFullShaderStageString(shaderType);

		if (!Shader.preprocess(&glslang::DefaultTBuiltInResource, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer))
		{
			LogVarDebug("Debug Preprocessing : GLSL stage %s Preprocessing Failed for : %s", vShaderSuffix.c_str(), vOriginalFileName.c_str());
			
			std::string log = Shader.getInfoLog();
			if (!log.empty())
			{
				LogVarDebug("Debug Preprocessing Errors : %s", log.c_str());
				m_Error[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Preprocessing Errors", shaderTypeString, log);
				}
			}
#ifdef VERBOSE_DEBUG
			log = Shader.getInfoDebugLog();
			if (!log.empty())
			{
				LogVarDebug("Debug Preprocessing Errors : %s", log.c_str());
				m_Error[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Preprocessing Errors", shaderTypeString, log);
				}
			}
#endif
			m_Warnings.clear();

			//LogVarDebug("Debug : ==========================================");

			return SpirV;
		}
		else
		{
			m_Error.clear();
			std::string log = Shader.getInfoLog();
			if (!log.empty())
			{
				LogVarDebug("Debug Preprocessing Warnings : %s", log.c_str());
				m_Warnings[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Preprocessing Warnings", shaderTypeString, log);
				}
			}
#ifdef VERBOSE_DEBUG
			log = Shader.getInfoDebugLog();
			if (!log.empty())
			{
				LogVarDebug("Debug Preprocessing Warnings : %s", log.c_str());
				m_Warnings[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Preprocessing Warnings", shaderTypeString, log);
				}
			}
#endif
		}

		const char* PreprocessedCStr = PreprocessedGLSL.c_str();
		Shader.setStrings(&PreprocessedCStr, 1);

		auto entry = vEntryPoint;
		if (entry.empty())
			entry = "main";
		Shader.setEntryPoint(entry.c_str());
		Shader.setSourceEntryPoint("main");

		if (!Shader.parse(&glslang::DefaultTBuiltInResource, 100, false, messages))
		{
			LogVarDebug("Debug Parse (%s) : GLSL stage %s Parse Failed for stage : %s", entry.c_str(), vShaderSuffix.c_str(), vOriginalFileName.c_str());
			
			std::string log = Shader.getInfoLog();
			if (!log.empty())
			{
				LogVarDebug("Debug Parse Errors (%s) : %s", entry.c_str(), log.c_str());
				m_Error[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Parse Parse Errors", shaderTypeString, log);
				}
			}
#ifdef VERBOSE_DEBUG
			log = Shader.getInfoDebugLog();
			if (!log.empty())
			{
				LogVarDebug("Debu Parse Errors (%s) : %s", entry.c_str(), log.c_str());
				m_Error[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Parse Errors", shaderTypeString, log);
				}
			}
#endif
			m_Warnings.clear();

			//LogVarDebug("Debug : ==========================================");

			return SpirV;
		}
		else
		{
			m_Error.clear();
			std::string log = Shader.getInfoLog();
			if (!log.empty())
			{
				LogVarDebug("Debug Parse Warnings (%s) : %s", entry.c_str(), log.c_str());
				m_Warnings[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Parse Warnings", shaderTypeString, log);
				}
			}
#ifdef VERBOSE_DEBUG
			log = Shader.getInfoDebugLog();
			if (!log.empty())
			{
				LogVarDebug("Debug Parse Warnings (%s) : %s", entry.c_str(), log.c_str());
				m_Warnings[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Parse Warnings", shaderTypeString, log);
				}
			}
#endif
		}

		glslang::TProgram Program;
		Program.addShader(&Shader);

		if (!Program.link(messages))
		{
			LogVarDebug("Debug Linking (%s) : GLSL stage %s Linking Failed for : %s", entry.c_str(), vShaderSuffix.c_str(), vOriginalFileName.c_str());

			std::string log = Program.getInfoLog();
			if (!log.empty())
			{
				LogVarDebug("Debug Linking Errors (%s) : %s", entry.c_str(), log.c_str());
				m_Error[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Linking Errors", shaderTypeString, log);
				}
			}
#ifdef VERBOSE_DEBUG
			log = Program.getInfoDebugLog();
			if (!log.empty())
			{
				LogVarDebug("Debug Linking Errors (%s) : %s", entry.c_str(), log.c_str());
				m_Error[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Linking Errors", shaderTypeString, log);
				}
			}
#endif
			m_Warnings.clear();

			//LogVarDebug("Debug : ==========================================");

			return SpirV;
		}
		else
		{
			m_Error.clear();
			std::string log = Shader.getInfoLog();
			if (!log.empty())
			{
				LogVarDebug("Debug Linking Warnings (%s) : %s", entry.c_str(), log.c_str());
				m_Warnings[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Warnings", shaderTypeString, log);
				}
			}
#ifdef VERBOSE_DEBUG
			log = Shader.getInfoDebugLog();
			if (!log.empty())
			{
				LogVarDebug("Debug Linking Warnings (%s) : %s", entry.c_str(), log.c_str());
				m_Warnings[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Linking Warnings", shaderTypeString, log);
				}
			}
#endif
		}

		if (vUsedUniforms)
		{
			auto usedUniforms = CollectUniformInfosFromIR(*Shader.getIntermediate());
			for (auto u : usedUniforms)
			{
				(*vUsedUniforms)[u.first] |= u.second;
			}
		}

		spv::SpvBuildLogger logger;
		glslang::SpvOptions spvOptions;
		spvOptions.optimizeSize = true;
#ifdef _DEBUG
		spvOptions.generateDebugInfo = true;
#else
		spvOptions.stripDebugInfo = true;
#endif

		glslang::GlslangToSpv(*Program.getIntermediate(shaderType), SpirV, &logger, &spvOptions);

		if (logger.getAllMessages().length() > 0)
		{
			CTOOL_DEBUG_BREAK;

			std::string allmsgs = logger.getAllMessages();
			std::cout << allmsgs << std::endl;
		}
	}

	if (SpirV.empty())
	{
		LogVarDebug("Debug : Shader stage %s Spirv generation of %s : NOK !", vShaderSuffix.c_str(), vOriginalFileName.c_str());
	}
	else
	{
		//LogVarDebug("Debug : Shader stage %s Spirv generation of %s : OK !", vShaderSuffix.c_str(), vOriginalFileName.c_str());
	}

	//LogVarDebug("Debug : ==========================================");

	return SpirV;
}

void VulkanShader::ParseGLSLString(
	const std::string& vCode,
	const std::string& vShaderSuffix,
	const std::string& vOriginalFileName,
	const ShaderEntryPoint& vEntryPoint,
	ShaderMessagingFunction vMessagingFunction,
	TraverserFunction vTraverser)
{
	ZoneScoped;

	std::string InputGLSL = vCode;

	EShLanguage shaderType = GetShaderStage(vShaderSuffix);

	if (!InputGLSL.empty() && shaderType != EShLanguage::EShLangCount)
	{
		const char* InputCString = InputGLSL.c_str();

		glslang::TShader Shader(shaderType);
		Shader.setStrings(&InputCString, 1);

		//Set up Vulkan/SpirV Environment
		int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100
		glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;  // would map to, say, Vulkan 1.0
		glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;    // maps to, say, SPIR-V 1.0

		Shader.setEnvInput(glslang::EShSourceGlsl, shaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
		Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
		Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);
		auto entry = vEntryPoint;
		if (entry.empty())
			entry = "main";
		Shader.setEntryPoint(entry.c_str());
		Shader.setSourceEntryPoint("main");

		EShMessages messages = (EShMessages)(EShMsgAST);

		const int DefaultVersion = 110; // 110 for desktop, 100 for es

		DirStackFileIncluder Includer;

		std::string PreprocessedGLSL;

		std::string shaderTypeString = GetFullShaderStageString(shaderType);

		if (!Shader.preprocess(&glslang::DefaultTBuiltInResource, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer))
		{
			LogVarDebug("Debug : GLSL stage %s Preprocessing Failed for : %s", vShaderSuffix.c_str(), vOriginalFileName.c_str());
			LogVarDebug("Debug : %s", Shader.getInfoLog());
			LogVarDebug("Debug : %s", Shader.getInfoDebugLog());

			std::string log = Shader.getInfoLog();
			if (!log.empty())
			{
				m_Error[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Preprocessing Errors", shaderTypeString, log);
				}
			}
#ifdef VERBOSE_DEBUG
			log = Shader.getInfoDebugLog();
			if (!log.empty())
			{
				m_Error[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Preprocessing Errors", shaderTypeString, log);
				}
			}
#endif
			m_Warnings.clear();

			//LogVarDebug("Debug : ==========================================");
		}
		else
		{
			m_Error.clear();
			std::string log = Shader.getInfoLog();
			if (!log.empty())
			{
				m_Warnings[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Preprocessing Warnings", shaderTypeString, log);
				}
			}
#ifdef VERBOSE_DEBUG
			log = Shader.getInfoDebugLog();
			if (!log.empty())
			{
				m_Warnings[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Preprocessing Warnings", shaderTypeString, log);
				}
			}
#endif
		}

		const char* PreprocessedCStr = PreprocessedGLSL.c_str();
		Shader.setStrings(&PreprocessedCStr, 1);

		if (!Shader.parse(&glslang::DefaultTBuiltInResource, 100, false, messages))
		{
			LogVarDebug("Debug : GLSL stage %s Parse Failed for stage : %s", vShaderSuffix.c_str(), vOriginalFileName.c_str());
			LogVarDebug("Debug : %s", Shader.getInfoLog());
			LogVarDebug("Debug : %s", Shader.getInfoDebugLog());

			std::string log = Shader.getInfoLog();
			if (!log.empty())
			{
				m_Error[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Parse Errors", shaderTypeString, log);
				}
			}
#ifdef VERBOSE_DEBUG
			log = Shader.getInfoDebugLog();
			if (!log.empty())
			{
				m_Error[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Parse Errors", shaderTypeString, log);
				}
			}
#endif
			m_Warnings.clear();

			//LogVarDebug("Debug : ==========================================");
		}
		else
		{
			m_Error.clear();
			std::string log = Shader.getInfoLog();
			if (!log.empty())
			{
				m_Warnings[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Parse Warnings", shaderTypeString, log);
				}
			}
#ifdef VERBOSE_DEBUG
			log = Shader.getInfoDebugLog();
			if (!log.empty())
			{
				m_Warnings[shaderType].push_back(log);
				if (vMessagingFunction)
				{
					vMessagingFunction("Parse Warnings", shaderTypeString, log);
				}
			}
#endif
		}

		if (vTraverser)
		{
			vTraverser(Shader.getIntermediate());
		}
	}
}

vk::ShaderModule VulkanShader::CreateShaderModule(vk::Device vLogicalDevice, std::vector<unsigned int> vSPIRVCode)
{
	ZoneScoped;

	vk::ShaderModule shaderModule = vk::ShaderModule{};

	if (!vSPIRVCode.empty())
	{
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.codeSize = vSPIRVCode.size() * sizeof(unsigned int);
		createInfo.pCode = vSPIRVCode.data();

		if (vLogicalDevice.createShaderModule(&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess)
		{
			LogVarDebug("Debug : fail to create shader module !");
			return vk::ShaderModule{};
		}
	}
	else
	{
		LogVarDebug("Debug : SPIRV Code is empty. Fail to create shader module !");
		return vk::ShaderModule{};
	}

	if (shaderModule)
	{
		//LogVarDebug("Debug : shader module compilation : OK !");
	}

	return shaderModule;
}

void VulkanShader::DestroyShaderModule(vk::Device vLogicalDevice, vk::ShaderModule vShaderModule)
{
	ZoneScoped;

	if (vLogicalDevice && vShaderModule)
	{
		vLogicalDevice.destroyShaderModule(vShaderModule, nullptr);
	}
}

std::unordered_map<std::string, bool> VulkanShader::CollectUniformInfosFromIR(const glslang::TIntermediate& intermediate)
{
	ZoneScoped;

	std::unordered_map<std::string, bool> res;

	TIntermNode* root = intermediate.getTreeRoot();

	if (root == 0)
		return res;

	TIRUniformsLocator it;
	root->traverse(&it);
	res = it.usedUniforms;

	return res;
}
