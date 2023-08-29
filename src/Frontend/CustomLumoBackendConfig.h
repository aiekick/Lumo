#pragma once

// codes from sdfmFont.h
#define ICON_SDFM_TRASH_CAN_OUTLINE u8"\ufa79"
#define ICON_SDFM_PAUSE u8"\uf3e4"
#define ICON_SDFM_PLAY u8"\uf40a"
#define ICON_SDFM_PENCIL u8"\uf3eb"
#define ICON_SDFM_LOCK u8"\uf33e"

#define LUMO_BACKEND_ICON_LABEL_RESET ICON_SDFM_TRASH_CAN_OUTLINE
#define LUMO_BACKEND_ICON_LABEL_PAUSE ICON_SDFM_PAUSE
#define LUMO_BACKEND_ICON_LABEL_PLAY ICON_SDFM_PLAY
#define LUMO_BACKEND_ICON_LABEL_EDIT ICON_SDFM_PENCIL
#define LUMO_BACKEND_CAMERA_ICON_LABEL_LOCK ICON_SDFM_LOCK

// colors for RenderDoc
#define QUAD_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.6f, 0.8f, 0.9f, 0.5f)
#define MESH_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.7f, 0.9f, 0.6f, 0.5f)
#define VERTEX_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.7f, 0.4f, 0.6f, 0.5f)
#define COMPUTE_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.7f, 0.6f, 0.9f, 0.5f)
#define GENERIC_RENDERER_DEBUG_COLOR ct::fvec4(0.8f, 0.8f, 0.5f, 0.5f)
#define IMGUI_RENDERER_DEBUG_COLOR ct::fvec4(0.9f, 0.6f, 0.6f, 0.5f)
#define SHADERBLOCK_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.3f, 0.6f, 0.6f, 0.5f)