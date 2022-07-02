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
#pragma warning(disable : 4324)

#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>

namespace VertexStruct
{
	class PipelineVertexInputState
	{
	public:
		vk::PipelineVertexInputStateCreateInfo state = {};
		vk::VertexInputBindingDescription binding = {};
		std::vector<vk::VertexInputAttributeDescription> attributes;
	};

	// ne pas utiliser size_t e, X64 il utilise des int64 ald des int32 en x86
	// win64 => typedef unsigned __int64 size_t;
	// win32 => typedef unsigned int     size_t;
	// glBufferData supporte les uint mais pas les uint64
	// vulkan, il semeble que uint64 verole les indes dans le gpu, est ce uniquement du au binaire x86 ?
	// a tester sur x64. vk::DeviceSize est un uint64_t curieusement, mais peut etre que un indexBuffer ne peut supporter ce format
	typedef uint32_t I1;

	class P3_N3_C4
	{
	public:
		static void GetInputState(PipelineVertexInputState& vInputState);

	public:
		ct::fvec3 p;// pos
		ct::fvec3 n;// normal
		ct::fvec4 c;// color

	public:
		P3_N3_C4();
		P3_N3_C4(ct::fvec3 vp);
		P3_N3_C4(ct::fvec3 vp, ct::fvec3 vn);
		P3_N3_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec4 vc);
	};

	class P3_N3_C4_D1
	{
	public:
		static void GetInputState(PipelineVertexInputState& vInputState);

	public:
		ct::fvec3 p;// pos
		ct::fvec3 n;// normal
		ct::fvec4 c;// color
		float d = 0.0f; // distance field

	public:
		P3_N3_C4_D1();
		P3_N3_C4_D1(ct::fvec3 vp);
		P3_N3_C4_D1(ct::fvec3 vp, ct::fvec3 vn);
		P3_N3_C4_D1(ct::fvec3 vp, ct::fvec3 vn, ct::fvec4 vc);
		P3_N3_C4_D1(ct::fvec3 vp, ct::fvec3 vn, ct::fvec4 vc, float vd);
	};

	class P2_T2
	{
	public:
		static void GetInputState(PipelineVertexInputState& vInputState);

	public:
		ct::fvec2 p;// pos
		ct::fvec2 t;// tex coord

	public:
		P2_T2();
		P2_T2(ct::fvec2 vp);
		P2_T2(ct::fvec2 vp, ct::fvec2 vt);
	};

	class P3_N3_T2
	{
	public:
		static void GetInputState(PipelineVertexInputState& vInputState);

	public:
		ct::fvec3 p;// pos
		ct::fvec3 n;// normal
		ct::fvec2 t;// tex coord


	public:
		P3_N3_T2();
		P3_N3_T2(ct::fvec3 vp);
		P3_N3_T2(ct::fvec3 vp, ct::fvec3 vn);
		P3_N3_T2(ct::fvec3 vp, ct::fvec3 vn, ct::fvec2 vt);
	};

	class P3_N3_T2_C4
	{
	public:
		static void GetInputState(PipelineVertexInputState& vInputState);

	public:
		ct::fvec3 p;// pos
		ct::fvec3 n;// normal
		ct::fvec2 t;// tex coord
		ct::fvec4 c;// color


	public:
		P3_N3_T2_C4();
		P3_N3_T2_C4(ct::fvec3 vp);
		P3_N3_T2_C4(ct::fvec3 vp, ct::fvec3 vn);
		P3_N3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec2 vt);
		P3_N3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec2 vt, ct::fvec4 vc);
	};

	class P3_N3_TA3_BTA3_T2_C4
	{
	public:
		static void GetInputState(PipelineVertexInputState& vInputState);

	public:
		ct::fvec3 p;// pos
		ct::fvec3 n;// normal
		ct::fvec3 tan;// tangent
		ct::fvec3 btan;// bitangent
		ct::fvec2 t;// tex coord
		ct::fvec4 c;// color


	public:
		P3_N3_TA3_BTA3_T2_C4();
		P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp);
		P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn);
		P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan);
		P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan, ct::fvec3 vbtan);
		P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan, ct::fvec3 vbtan, ct::fvec2 vt);
		P3_N3_TA3_BTA3_T2_C4(ct::fvec3 vp, ct::fvec3 vn, ct::fvec3 vtan, ct::fvec3 vbtan, ct::fvec2 vt, ct::fvec4 vc);
	};
}
