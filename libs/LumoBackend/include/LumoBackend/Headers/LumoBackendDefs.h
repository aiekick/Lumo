/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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
#pragma warning(disable : 4251)

#include <string>
#include <memory>
#include <Gaia/gaia.h>

#if defined(__WIN32__) || defined(WIN32) || defined(_WIN32) || defined(__WIN64__) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
#if defined(LumoBackend_EXPORTS)
#define LUMO_BACKEND_API __declspec(dllexport)
#elif defined(BUILD_LUMO_BACKEND_SHARED_LIBS)
#define LUMO_BACKEND_API __declspec(dllimport)
#else
#define LUMO_BACKEND_API
#endif
#else
#define LUMO_BACKEND_API
#endif

enum class ModelRenderModeEnum : uint8_t {
    MODEL_RENDER_MODE_POINTS = (uint8_t)vk::PrimitiveTopology::ePointList,
    MODEL_RENDER_MODE_LINES = (uint8_t)vk::PrimitiveTopology::eLineList,
    MODEL_RENDER_MODE_LINE_STRIP = (uint8_t)vk::PrimitiveTopology::eLineStrip,
    MODEL_RENDER_MODE_TRIANGLES = (uint8_t)vk::PrimitiveTopology::eTriangleList,
    MODEL_RENDER_MODE_TRIANGLE_STRIP = (uint8_t)vk::PrimitiveTopology::eTriangleStrip,
    MODEL_RENDER_MODE_TRIANGLE_FAN = (uint8_t)vk::PrimitiveTopology::eTriangleFan,
    MODEL_RENDER_MODE_PATCHES = (uint8_t)vk::PrimitiveTopology::ePatchList,  // tesselation shader
    MODEL_RENDER_MODE_NONE,
    MODEL_RENDER_MODE_Count
};

static inline ModelRenderModeEnum GetModelRenderModeEnumFromString(const std::string& vString) {
    if (vString == "POINTS")
        return ModelRenderModeEnum::MODEL_RENDER_MODE_POINTS;
    if (vString == "LINE_STRIP")
        return ModelRenderModeEnum::MODEL_RENDER_MODE_LINE_STRIP;
    if (vString == "LINES")
        return ModelRenderModeEnum::MODEL_RENDER_MODE_LINES;
    if (vString == "TRIANGLE_STRIP")
        return ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_STRIP;
    if (vString == "TRIANGLE_FAN")
        return ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_FAN;
    if (vString == "TRIANGLES")
        return ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES;
    if (vString == "PATCHES")
        return ModelRenderModeEnum::MODEL_RENDER_MODE_PATCHES;
    return ModelRenderModeEnum::MODEL_RENDER_MODE_NONE;
}

static inline const char* GetModelRenderModeEnumString(ModelRenderModeEnum vModelRenderModeEnum) {
    switch (vModelRenderModeEnum) {
        case ModelRenderModeEnum::MODEL_RENDER_MODE_POINTS: return "POINTS";
        case ModelRenderModeEnum::MODEL_RENDER_MODE_LINE_STRIP: return "LINE_STRIP";
        case ModelRenderModeEnum::MODEL_RENDER_MODE_LINES: return "LINES";
        case ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_STRIP: return "TRIANGLE_STRIP";
        case ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_FAN: return "TRIANGLE_FAN";
        case ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES: return "TRIANGLES";
        case ModelRenderModeEnum::MODEL_RENDER_MODE_PATCHES: return "PATCHES";
    }
    return "";
}

enum class BaseMeshEnum : uint8_t { BASE_MESH_QUAD = 0, BASE_MESH_POINTS, BASE_MESH_MESH, BASE_MESH_NONE, BASE_MESH_Count };

static inline const char* GetModelTypeEnumString(BaseMeshEnum vBaseMeshEnum) {
    switch (vBaseMeshEnum) {
        case BaseMeshEnum::BASE_MESH_QUAD: return "QUAD";
        case BaseMeshEnum::BASE_MESH_POINTS: return "POINTS";
        case BaseMeshEnum::BASE_MESH_MESH: return "MESH";
    }
    return "";
}

static inline BaseMeshEnum GetModelTypeEnumFromString(const std::string& vString) {
    if (vString == "QUAD")
        return BaseMeshEnum::BASE_MESH_QUAD;
    if (vString == "POINTS")
        return BaseMeshEnum::BASE_MESH_POINTS;
    if (vString == "MESH")
        return BaseMeshEnum::BASE_MESH_MESH;

    return BaseMeshEnum::BASE_MESH_NONE;
}

class BaseRenderer;
typedef std::shared_ptr<BaseRenderer> BaseRendererPtr;
typedef std::weak_ptr<BaseRenderer> BaseRendererWeak;

class FrameBuffer;
typedef std::shared_ptr<FrameBuffer> FrameBufferPtr;
typedef std::weak_ptr<FrameBuffer> FrameBufferWeak;

class ComputeBuffer;
typedef std::shared_ptr<ComputeBuffer> ComputeBufferPtr;
typedef std::weak_ptr<ComputeBuffer> ComputeBufferWeak;

class ShaderPass;
typedef std::shared_ptr<ShaderPass> ShaderPassPtr;
typedef std::weak_ptr<ShaderPass> ShaderPassWeak;

class GizmoInterface;
typedef std::shared_ptr<GizmoInterface> GizmoInterfacePtr;
typedef std::weak_ptr<GizmoInterface> GizmoInterfaceWeak;

class SceneMerger;
typedef std::shared_ptr<SceneMerger> SceneMergerPtr;
typedef std::weak_ptr<SceneMerger> SceneMergerWeak;

// colors for RenderDoc
#define QUAD_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.6f, 0.8f, 0.9f, 0.5f)
#define MESH_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.7f, 0.9f, 0.6f, 0.5f)
#define VERTEX_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.7f, 0.4f, 0.6f, 0.5f)
#define COMPUTE_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.7f, 0.6f, 0.9f, 0.5f)
#define GENERIC_RENDERER_DEBUG_COLOR ct::fvec4(0.8f, 0.8f, 0.5f, 0.5f)
#define IMGUI_RENDERER_DEBUG_COLOR ct::fvec4(0.9f, 0.6f, 0.6f, 0.5f)
