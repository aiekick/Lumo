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

#include "VertexStruct.h"

namespace VertexStruct
{
	void P3_N3_C4::GetInputState(PipelineVertexInputState& vInputState)
	{
		vInputState.binding.binding = 0;
		vInputState.binding.stride = sizeof(P3_N3_C4);
		vInputState.binding.inputRate = vk::VertexInputRate::eVertex;

		uint32_t offset = 0;

		// P3_N3_C4
		vInputState.attributes.resize(3);

		// vertex pos vec3
		vInputState.attributes[0].binding = 0;
		vInputState.attributes[0].location = 0;
		vInputState.attributes[0].format = vk::Format::eR32G32B32Sfloat;
		vInputState.attributes[0].offset = 0;
		offset += sizeof(ct::fvec3);

		// vertex normal vec3
		vInputState.attributes[1].binding = 0;
		vInputState.attributes[1].location = 1;
		vInputState.attributes[1].format = vk::Format::eR32G32B32Sfloat;
		vInputState.attributes[1].offset = offset;
		offset += sizeof(ct::fvec3);

		// vertex color vec4
		vInputState.attributes[2].binding = 0;
		vInputState.attributes[2].location = 2;
		vInputState.attributes[2].format = vk::Format::eR32G32B32A32Sfloat;
		vInputState.attributes[2].offset = offset;
		offset += sizeof(ct::fvec4);

		vInputState.state = vk::PipelineVertexInputStateCreateInfo(
			vk::PipelineVertexInputStateCreateFlags(),
			1,
			&vInputState.binding,
			static_cast<uint32_t>(vInputState.attributes.size()),
			vInputState.attributes.data()
		);
	}
	P3_N3_C4::P3_N3_C4() {}
	P3_N3_C4::P3_N3_C4(ct::fvec3 vp) { p = vp; }
	P3_N3_C4::P3_N3_C4(ct::fvec3 vp, ct::fvec3 vn) { p = vp; n = vn; }
	P3_N3_C4::P3_N3_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec4 vc) { p = vp; n = vn; c = vc; }

	void P3_N3_C4_D1::GetInputState(PipelineVertexInputState& vInputState)
	{
		vInputState.binding.binding = 0;
		vInputState.binding.stride = sizeof(P3_N3_C4_D1);
		vInputState.binding.inputRate = vk::VertexInputRate::eVertex;

		uint32_t offset = 0;

		// P3_N3_C4_D1
		vInputState.attributes.resize(4);

		// vertex pos vec3
		vInputState.attributes[0].binding = 0;
		vInputState.attributes[0].location = 0;
		vInputState.attributes[0].format = vk::Format::eR32G32B32Sfloat;
		vInputState.attributes[0].offset = 0;
		offset += sizeof(ct::fvec3);

		// vertex normal vec3
		vInputState.attributes[1].binding = 0;
		vInputState.attributes[1].location = 1;
		vInputState.attributes[1].format = vk::Format::eR32G32B32Sfloat;
		vInputState.attributes[1].offset = offset;
		offset += sizeof(ct::fvec3);

		// vertex color vec4
		vInputState.attributes[2].binding = 0;
		vInputState.attributes[2].location = 2;
		vInputState.attributes[2].format = vk::Format::eR32G32B32A32Sfloat;
		vInputState.attributes[2].offset = offset;
		offset += sizeof(ct::fvec4);

		// vertex df
		vInputState.attributes[3].binding = 0;
		vInputState.attributes[3].location = 3;
		vInputState.attributes[3].format = vk::Format::eR32Sfloat;
		vInputState.attributes[3].offset = offset;
		offset += sizeof(float);

		vInputState.state = vk::PipelineVertexInputStateCreateInfo(
			vk::PipelineVertexInputStateCreateFlags(),
			1,
			&vInputState.binding,
			static_cast<uint32_t>(vInputState.attributes.size()),
			vInputState.attributes.data()
		);
	}
	P3_N3_C4_D1::P3_N3_C4_D1() {}
	P3_N3_C4_D1::P3_N3_C4_D1(ct::fvec3 vp) { p = vp; }
	P3_N3_C4_D1::P3_N3_C4_D1(ct::fvec3 vp, ct::fvec3 vn) { p = vp; n = vn; }
	P3_N3_C4_D1::P3_N3_C4_D1(ct::fvec3 vp, ct::fvec3 vn, ct::fvec4 vc) { p = vp; n = vn; c = vc; }
	P3_N3_C4_D1::P3_N3_C4_D1(ct::fvec3 vp, ct::fvec3 vn, ct::fvec4 vc, float vd) { p = vp; n = vn; c = vc; d = vd; }

	void P2_T2::GetInputState(PipelineVertexInputState& vInputState)
	{
		vInputState.binding.binding = 0;
		vInputState.binding.stride = sizeof(P2_T2);
		vInputState.binding.inputRate = vk::VertexInputRate::eVertex;

		uint32_t offset = 0;

		// P2_T2
		vInputState.attributes.resize(2);

		// vertex pos vec2
		vInputState.attributes[0].binding = 0;
		vInputState.attributes[0].location = 0;
		vInputState.attributes[0].format = vk::Format::eR32G32Sfloat;
		vInputState.attributes[0].offset = 0;
		offset += sizeof(ct::fvec2);

		// texture coordinate vec2
		vInputState.attributes[1].binding = 0;
		vInputState.attributes[1].location = 1;
		vInputState.attributes[1].format = vk::Format::eR32G32Sfloat;
		vInputState.attributes[1].offset = offset;
		offset += sizeof(ct::fvec2);

		vInputState.state = vk::PipelineVertexInputStateCreateInfo(
			vk::PipelineVertexInputStateCreateFlags(),
			1,
			&vInputState.binding,
			static_cast<uint32_t>(vInputState.attributes.size()),
			vInputState.attributes.data()
		);
	}
	P2_T2::P2_T2() {}
	P2_T2::P2_T2(ct::fvec2 vp) { p = vp; }
	P2_T2::P2_T2(ct::fvec2 vp, ct::fvec2 vt) { p = vp; t = vt; }

	void P3_N3_T2::GetInputState(PipelineVertexInputState& vInputState)
	{
		vInputState.binding.binding = 0;
		vInputState.binding.stride = sizeof(P3_N3_T2);
		vInputState.binding.inputRate = vk::VertexInputRate::eVertex;

		uint32_t offset = 0;

		// P3_N3_T2
		vInputState.attributes.resize(3);

		// vertex pos vec3
		vInputState.attributes[0].binding = 0;
		vInputState.attributes[0].location = 0;
		vInputState.attributes[0].format = vk::Format::eR32G32B32Sfloat;
		vInputState.attributes[0].offset = 0;
		offset += sizeof(ct::fvec3);

		// vertex normal vec3
		vInputState.attributes[1].binding = 0;
		vInputState.attributes[1].location = 1;
		vInputState.attributes[1].format = vk::Format::eR32G32B32Sfloat;
		vInputState.attributes[1].offset = offset;
		offset += sizeof(ct::fvec3);

		// texture coordinate vec2
		vInputState.attributes[2].binding = 0;
		vInputState.attributes[2].location = 2;
		vInputState.attributes[2].format = vk::Format::eR32G32Sfloat;
		vInputState.attributes[2].offset = offset;
		offset += sizeof(ct::fvec2);

		vInputState.state = vk::PipelineVertexInputStateCreateInfo(
			vk::PipelineVertexInputStateCreateFlags(),
			1,
			&vInputState.binding,
			static_cast<uint32_t>(vInputState.attributes.size()),
			vInputState.attributes.data()
		);
	}
	P3_N3_T2::P3_N3_T2() {}
	P3_N3_T2::P3_N3_T2(ct::fvec3 vp) { p = vp; }
	P3_N3_T2::P3_N3_T2(ct::fvec3 vp, ct::fvec3 vn) { p = vp; n = vn; }
	P3_N3_T2::P3_N3_T2(ct::fvec3 vp, ct::fvec3 vn, ct::fvec2 vt) { p = vp; n = vn; t = vt; }

	void P3_N3_T2_C4::GetInputState(PipelineVertexInputState& vInputState)
	{
		vInputState.binding.binding = 0;
		vInputState.binding.stride = sizeof(P3_N3_T2_C4);
		vInputState.binding.inputRate = vk::VertexInputRate::eVertex;

		uint32_t offset = 0;

		// P3_N3_T2_C4
		vInputState.attributes.resize(4);

		// vertex pos vec3
		vInputState.attributes[0].binding = 0;
		vInputState.attributes[0].location = 0;
		vInputState.attributes[0].format = vk::Format::eR32G32B32Sfloat;
		vInputState.attributes[0].offset = 0;
		offset += sizeof(ct::fvec3);

		// vertex normal vec3
		vInputState.attributes[1].binding = 0;
		vInputState.attributes[1].location = 1;
		vInputState.attributes[1].format = vk::Format::eR32G32B32Sfloat;
		vInputState.attributes[1].offset = offset;
		offset += sizeof(ct::fvec3);

		// texture coordinate vec2
		vInputState.attributes[2].binding = 0;
		vInputState.attributes[2].location = 2;
		vInputState.attributes[2].format = vk::Format::eR32G32Sfloat;
		vInputState.attributes[2].offset = offset;
		offset += sizeof(ct::fvec2);

		// vertex color vec4
		vInputState.attributes[3].binding = 0;
		vInputState.attributes[3].location = 3;
		vInputState.attributes[3].format = vk::Format::eR32G32B32A32Sfloat;
		vInputState.attributes[3].offset = offset;
		offset += sizeof(ct::fvec4);

		vInputState.state = vk::PipelineVertexInputStateCreateInfo(
			vk::PipelineVertexInputStateCreateFlags(),
			1,
			&vInputState.binding,
			static_cast<uint32_t>(vInputState.attributes.size()),
			vInputState.attributes.data()
		);
	}
	P3_N3_T2_C4::P3_N3_T2_C4() {}
	P3_N3_T2_C4::P3_N3_T2_C4(ct::fvec3 vp) { p = vp; }
	P3_N3_T2_C4::P3_N3_T2_C4(ct::fvec3 vp, ct::fvec3 vn) { p = vp; n = vn; }
	P3_N3_T2_C4::P3_N3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec2 vt) { p = vp; n = vn; t = vt; }
	P3_N3_T2_C4::P3_N3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec2 vt, ct::fvec4 vc) { p = vp; n = vn; t = vt; c = vc; }

	void P3_N3_TA3_BTA3_T2_C4::GetInputState(PipelineVertexInputState& vInputState)
	{
		vInputState.binding.binding = 0;
		vInputState.binding.stride = sizeof(P3_N3_TA3_BTA3_T2_C4);
		vInputState.binding.inputRate = vk::VertexInputRate::eVertex;

		uint32_t offset = 0;

		// P3_N3_TA3_BTA3_T2_C4
		vInputState.attributes.resize(6);

		// vertex pos vec3
		vInputState.attributes[0].binding = 0;
		vInputState.attributes[0].location = 0;
		vInputState.attributes[0].format = vk::Format::eR32G32B32Sfloat;
		vInputState.attributes[0].offset = 0;
		offset += sizeof(ct::fvec3);

		// vertex normal vec3
		vInputState.attributes[1].binding = 0;
		vInputState.attributes[1].location = 1;
		vInputState.attributes[1].format = vk::Format::eR32G32B32Sfloat;
		vInputState.attributes[1].offset = offset;
		offset += sizeof(ct::fvec3);

		// tangant vec3
		vInputState.attributes[2].binding = 0;
		vInputState.attributes[2].location = 2;
		vInputState.attributes[2].format = vk::Format::eR32G32B32Sfloat;
		vInputState.attributes[2].offset = offset;
		offset += sizeof(ct::fvec3);

		// bi-tangant vec3
		vInputState.attributes[3].binding = 0;
		vInputState.attributes[3].location = 3;
		vInputState.attributes[3].format = vk::Format::eR32G32B32Sfloat;
		vInputState.attributes[3].offset = offset;
		offset += sizeof(ct::fvec3);

		// texture coordinate vec2
		vInputState.attributes[4].binding = 0;
		vInputState.attributes[4].location = 4;
		vInputState.attributes[4].format = vk::Format::eR32G32Sfloat;
		vInputState.attributes[4].offset = offset;
		offset += sizeof(ct::fvec2);

		// vertex color vec4
		vInputState.attributes[5].binding = 0;
		vInputState.attributes[5].location = 5;
		vInputState.attributes[5].format = vk::Format::eR32G32B32A32Sfloat;
		vInputState.attributes[5].offset = offset;
		offset += sizeof(ct::fvec4);

		vInputState.state = vk::PipelineVertexInputStateCreateInfo(
			vk::PipelineVertexInputStateCreateFlags(),
			1,
			&vInputState.binding,
			static_cast<uint32_t>(vInputState.attributes.size()),
			vInputState.attributes.data()
		);
	}
	P3_N3_TA3_BTA3_T2_C4::P3_N3_TA3_BTA3_T2_C4() {}
	P3_N3_TA3_BTA3_T2_C4::P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp) { p = vp; }
	P3_N3_TA3_BTA3_T2_C4::P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn) { p = vp; n = vn; }
	P3_N3_TA3_BTA3_T2_C4::P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan) { p = vp; n = vn; tan = vtan; }
	P3_N3_TA3_BTA3_T2_C4::P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan, ct::fvec3 vbtan) { p = vp; n = vn; tan = vtan; btan = vbtan; }
	P3_N3_TA3_BTA3_T2_C4::P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan, ct::fvec3 vbtan, ct::fvec2 vt) { p = vp; n = vn; tan = vtan; btan = vbtan; t = vt; }
	P3_N3_TA3_BTA3_T2_C4::P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan, ct::fvec3 vbtan, ct::fvec2 vt, ct::fvec4 vc) { p = vp; n = vn; tan = vtan; btan = vbtan; t = vt; c = vc; }
}