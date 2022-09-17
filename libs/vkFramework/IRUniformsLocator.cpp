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

#include "IRUniformsLocator.h"

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

TIRUniformsLocator::TIRUniformsLocator()
{
	usedUniforms.clear();
}

bool TIRUniformsLocator::visitBinary(glslang::TVisit /* visit */, glslang::TIntermBinary* vNode)
{
	ZoneScoped;

	using namespace glslang;

	if (vNode->getOp() == EOpIndexDirectStruct)
	{
		bool reference = vNode->getLeft()->getType().isReference();
		const TTypeList* members = reference ? vNode->getLeft()->getType().getReferentType()->getStruct() : vNode->getLeft()->getType().getStruct();
		auto tName = (*members)[vNode->getRight()->getAsConstantUnion()->getConstArray()[0].getIConst()].type->getFieldName();
		std::string name = tName.c_str(); // corresponding structure name

		auto symbol = vNode->getLeft()->getAsSymbolNode();
		if (symbol)
		{
			auto qualifier = symbol->getQualifier();
			if (qualifier.storage == glslang::TStorageQualifier::EvqUniform)
			{
				std::string newName = symbol->getName().c_str(); // "anon@0"
				usedUniforms[name] = true;
			}
		}
	}

	return true;
}

void TIRUniformsLocator::visitSymbol(glslang::TIntermSymbol* vNode)
{
	ZoneScoped;

	using namespace glslang;

	// on va recuper les noms de samplers
	if (vNode->getBasicType() == TBasicType::EbtSampler)
	{
		auto qualifier = vNode->getQualifier();
		if (qualifier.storage == glslang::TStorageQualifier::EvqUniform)
		{
			std::string name = vNode->getName().c_str();
			usedUniforms[name] = true;
		}
	}
}