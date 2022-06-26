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

#include <glslang/glslang/MachineIndependent/localintermediate.h>
#include <glslang/glslang/Include/InfoSink.h>

#include <unordered_map>
#include <string>

// on va parser le shader
// et tout les uniforms qu'on va rencontrer on va les marquer comme utilisé
// pour le moment on cherche pas à savoir si l'uniforms est vraiment utilisé
// genre si au bout du compte ca valeur ne sert a rien. on verra ca plus tard

class TIRUniformsLocator : public glslang::TIntermTraverser
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
