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

#include <glslang/glslang/MachineIndependent/localintermediate.h>
#include <glslang/glslang/Include/InfoSink.h>
#include <vkFramework/vkFramework.h>

#include <unordered_map>
#include <string>

// on va parser le shader
// et tout les uniforms qu'on va rencontrer on va les marquer comme utilisé
// pour le moment on cherche pas à savoir si l'uniforms est vraiment utilisé
// genre si au bout du compte ca valeur ne sert a rien. on verra ca plus tard

class VKFRAMEWORK_API TIRUniformsLocator : public glslang::TIntermTraverser
{
public:
	std::unordered_map<std::string, bool> usedUniforms;

public:
	TIRUniformsLocator();

	virtual bool visitBinary(glslang::TVisit, glslang::TIntermBinary* vNode);
	virtual void visitSymbol(glslang::TIntermSymbol* vNode);

protected:
	TIRUniformsLocator(TIRUniformsLocator&);
	TIRUniformsLocator& operator=(TIRUniformsLocator&);
};
