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

#include <vulkan/vulkan.hpp>
enum class ModelRenderModeEnum : uint8_t
{
	MODEL_RENDER_MODE_POINTS = (uint8_t)vk::PrimitiveTopology::ePointList,
	MODEL_RENDER_MODE_LINES = (uint8_t)vk::PrimitiveTopology::eLineList,
	MODEL_RENDER_MODE_LINE_STRIP = (uint8_t)vk::PrimitiveTopology::eLineStrip,
	MODEL_RENDER_MODE_TRIANGLES = (uint8_t)vk::PrimitiveTopology::eTriangleList,
	MODEL_RENDER_MODE_TRIANGLE_STRIP = (uint8_t)vk::PrimitiveTopology::eTriangleStrip,
	MODEL_RENDER_MODE_TRIANGLE_FAN = (uint8_t)vk::PrimitiveTopology::eTriangleFan,
	MODEL_RENDER_MODE_PATCHES = (uint8_t)vk::PrimitiveTopology::ePatchList, // tesselation shader
	MODEL_RENDER_MODE_NONE,
	MODEL_RENDER_MODE_Count
};

static inline ModelRenderModeEnum GetModelRenderModeEnumFromString(const std::string& vString)
{
	if (vString == "POINTS") return ModelRenderModeEnum::MODEL_RENDER_MODE_POINTS;
	if (vString == "LINE_STRIP") return ModelRenderModeEnum::MODEL_RENDER_MODE_LINE_STRIP;
	if (vString == "LINES") return ModelRenderModeEnum::MODEL_RENDER_MODE_LINES;
	if (vString == "TRIANGLE_STRIP") return ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_STRIP;
	if (vString == "TRIANGLE_FAN") return ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_FAN;
	if (vString == "TRIANGLES") return ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES;
	if (vString == "PATCHES") return ModelRenderModeEnum::MODEL_RENDER_MODE_PATCHES;
	return ModelRenderModeEnum::MODEL_RENDER_MODE_NONE;
}

static inline const char* GetModelRenderModeEnumString(ModelRenderModeEnum vModelRenderModeEnum)
{
	switch (vModelRenderModeEnum)
	{
	case ModelRenderModeEnum::MODEL_RENDER_MODE_POINTS:			return "POINTS";
	case ModelRenderModeEnum::MODEL_RENDER_MODE_LINE_STRIP:		return "LINE_STRIP";
	case ModelRenderModeEnum::MODEL_RENDER_MODE_LINES:			return "LINES";
	case ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_STRIP:	return "TRIANGLE_STRIP";
	case ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_FAN:	return "TRIANGLE_FAN";
	case ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES:		return "TRIANGLES";
	case ModelRenderModeEnum::MODEL_RENDER_MODE_PATCHES:		return "PATCHES";
	}
	return "";
}

enum class BaseMeshEnum : uint8_t
{
	BASE_MESH_QUAD = 0,
	BASE_MESH_POINTS,
	BASE_MESH_MESH,
	BASE_MESH_NONE,
	BASE_MESH_Count
};

static inline const char* GetModelTypeEnumString(BaseMeshEnum vBaseMeshEnum)
{
	switch (vBaseMeshEnum)
	{
	case BaseMeshEnum::BASE_MESH_QUAD:		return "QUAD";
	case BaseMeshEnum::BASE_MESH_POINTS:	return "POINTS";
	case BaseMeshEnum::BASE_MESH_MESH:		return "MESH";
	}
	return "";
}

static inline BaseMeshEnum GetModelTypeEnumFromString(const std::string& vString)
{
	if (vString == "QUAD")		return BaseMeshEnum::BASE_MESH_QUAD;
	if (vString == "POINTS")	return BaseMeshEnum::BASE_MESH_POINTS;
	if (vString == "MESH")		return BaseMeshEnum::BASE_MESH_MESH;

	return BaseMeshEnum::BASE_MESH_NONE;
}