#pragma once

#include <Gaia/gaia.h>
#include <memory>
#include <LumoBackend/Headers/LumoBackendDefs.h>

class MainBackend;
typedef std::shared_ptr<MainBackend> MainBackendPtr;
typedef std::weak_ptr<MainBackend> MainBackendWeak;

class MainFrontend;
typedef std::shared_ptr<MainFrontend> MainFrontendPtr;
typedef std::weak_ptr<MainFrontend> MainFrontendWeak;

class ImGuiOverlay;
typedef std::shared_ptr<ImGuiOverlay> ImGuiOverlayPtr;
typedef std::weak_ptr<ImGuiOverlay> ImGuiOverlayWeak;

// Colors for Debug Pass Name in RenderDoc
#define QUAD_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.6f, 0.8f, 0.9f, 0.5f)
#define MESH_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.7f, 0.9f, 0.6f, 0.5f)
#define VERTEX_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.7f, 0.4f, 0.6f, 0.5f)
#define COMPUTE_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.7f, 0.6f, 0.9f, 0.5f)
#define GENERIC_RENDERER_DEBUG_COLOR ct::fvec4(0.8f, 0.8f, 0.5f, 0.5f)
#define IMGUI_RENDERER_DEBUG_COLOR ct::fvec4(0.9f, 0.6f, 0.6f, 0.5f)
