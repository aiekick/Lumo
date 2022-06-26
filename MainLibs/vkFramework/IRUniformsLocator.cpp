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