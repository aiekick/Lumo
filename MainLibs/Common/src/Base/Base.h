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

#include <memory>
#include <ctools/cTools.h>

class FrameBuffer;
typedef std::shared_ptr<FrameBuffer> FrameBufferPtr;
typedef ct::cWeak<FrameBuffer> FrameBufferWeak;

class ComputeBuffer;
typedef std::shared_ptr<ComputeBuffer> ComputeBufferPtr;
typedef ct::cWeak<ComputeBuffer> ComputeBufferWeak;

class ShaderPass;
typedef std::shared_ptr<ShaderPass> ShaderPassPtr;
typedef ct::cWeak<ShaderPass> ShaderPassWeak;

// colors for RenderDoc
#define QUAD_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.6f, 0.8f, 0.9f, 0.5f)
#define MESH_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.7f, 0.9f, 0.6f, 0.5f)
#define VERTEX_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.7f, 0.4f, 0.6f, 0.5f)
#define COMPUTE_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.7f, 0.6f, 0.9f, 0.5f)

#define GENERIC_RENDERER_DEBUG_COLOR ct::fvec4(0.8f, 0.8f, 0.5f, 0.5f)

#define IMGUI_RENDERER_DEBUG_COLOR ct::fvec4(0.9f, 0.6f, 0.6f, 0.5f)
