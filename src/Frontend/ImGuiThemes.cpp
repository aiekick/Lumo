#include <Frontend/MainFrontend.h>

ImGuiTheme  GetOrangeBlueTheme() {
    ImGuiTheme res;

    res.style.Colors[ImGuiCol_Text]                  = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    res.style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
    res.style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    res.style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    res.style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    res.style.Colors[ImGuiCol_Border]                = ImVec4(0.26f, 0.28f, 0.29f, 1.00f);
    res.style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.32f, 0.34f, 0.36f, 1.00f);
    res.style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.21f, 0.29f, 0.36f, 1.00f);
    res.style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.26f, 0.59f, 0.98f, 0.71f);
    res.style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 0.59f, 0.98f, 0.93f);
    res.style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.18f, 0.20f, 0.21f, 1.00f);
    res.style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.23f, 0.25f, 0.26f, 1.00f);
    res.style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.30f, 0.33f, 0.35f, 1.00f);
    res.style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    res.style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.21f, 0.29f, 0.36f, 0.89f);
    res.style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.13f, 0.52f, 0.94f, 0.45f);
    res.style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.13f, 0.71f, 1.00f, 0.89f);
    res.style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.24f, 0.78f, 0.78f, 0.31f);
    res.style.Colors[ImGuiCol_CheckMark]             = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    res.style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    res.style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    res.style.Colors[ImGuiCol_Button]                = ImVec4(1.00f, 0.60f, 0.00f, 0.80f);
    res.style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(1.00f, 0.48f, 0.00f, 0.80f);
    res.style.Colors[ImGuiCol_ButtonActive]          = ImVec4(1.00f, 0.40f, 0.00f, 0.80f);
    res.style.Colors[ImGuiCol_Header]                = ImVec4(0.13f, 0.52f, 0.94f, 0.66f);
    res.style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.13f, 0.52f, 0.94f, 1.00f);
    res.style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.13f, 0.52f, 0.94f, 0.59f);
    res.style.Colors[ImGuiCol_Separator]             = ImVec4(0.18f, 0.35f, 0.58f, 0.59f);
    res.style.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    res.style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    res.style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    res.style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    res.style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    res.style.Colors[ImGuiCol_Tab]                   = ImVec4(0.20f, 0.41f, 0.68f, 0.00f);
    res.style.Colors[ImGuiCol_TabHovered]            = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    res.style.Colors[ImGuiCol_TabActive]             = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
    res.style.Colors[ImGuiCol_TabUnfocused]          = ImVec4(0.20f, 0.41f, 0.68f, 0.00f);
    res.style.Colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
    res.style.Colors[ImGuiCol_DockingPreview]        = ImVec4(0.13f, 0.52f, 0.94f, 1.00f);
    res.style.Colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.20f, 0.20f, 0.20f, 0.00f);
    res.style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    res.style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    res.style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.13f, 0.52f, 0.94f, 0.95f);
    res.style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    res.style.Colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    res.style.Colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    res.style.Colors[ImGuiCol_TableBorderLight]      = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    res.style.Colors[ImGuiCol_TableRowBg]            = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    res.style.Colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    res.style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    res.style.Colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    res.style.Colors[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    res.style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    res.style.Colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    res.style.Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    res.style.Colors[ImGuiCol_WindowBg].w   = 1.00f;
    res.style.Colors[ImGuiCol_ChildBg].w    = 0.00f;
    res.style.Colors[ImGuiCol_MenuBarBg]    = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    res.style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    res.goodColor = ImVec4(0.00f, 0.35f, 0.00f, 1.00f);
    res.badColor  = ImVec4(0.35f, 0.00f, 0.00f, 1.00f);

    // Main
    res.style.WindowPadding     = ImVec2(4.00f, 4.00f);
    res.style.FramePadding      = ImVec2(4.00f, 4.00f);
    res.style.ItemSpacing       = ImVec2(4.00f, 4.00f);
    res.style.ItemInnerSpacing  = ImVec2(4.00f, 4.00f);
    res.style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
    res.style.IndentSpacing     = 8.00f;
    res.style.ScrollbarSize     = 10.00f;
    res.style.GrabMinSize       = 8.00f;

    // Borders
    res.style.WindowBorderSize = 0.00f;
    res.style.ChildBorderSize  = 0.00f;
    res.style.PopupBorderSize  = 1.00f;
    res.style.FrameBorderSize  = 0.00f;
    res.style.TabBorderSize    = 0.00f;

    // Rounding
    res.style.WindowRounding    = 2.00f;
    res.style.ChildRounding     = 2.00f;
    res.style.FrameRounding     = 2.00f;
    res.style.PopupRounding     = 2.00f;
    res.style.ScrollbarRounding = 2.00f;
    res.style.GrabRounding      = 2.00f;
    res.style.TabRounding       = 2.00f;

    // Alignment
    res.style.WindowTitleAlign         = ImVec2(0.50f, 0.50f);
    res.style.WindowMenuButtonPosition = ImGuiDir_Left;
    res.style.ColorButtonPosition      = ImGuiDir_Right;
    res.style.ButtonTextAlign          = ImVec2(0.50f, 0.50f);
    res.style.SelectableTextAlign      = ImVec2(0.00f, 0.50f);

    // Safe Area Padding
    res.style.DisplaySafeAreaPadding = ImVec2(3.00f, 3.00f);

    res.fileTypeInfos[".glsl"] = IGFD::FileStyle(ImVec4(0.1f, 0.9f, 0.5f, 1.0f));

    return res;
}

bool MainFrontend::m_build_themes() {
    ImGuiThemeHelper::Instance()->AddTheme("Orange/Blue", GetOrangeBlueTheme());

    ImGuiThemeHelper::Instance()->SetDefaultTheme("Orange/Blue");
    ImGuiThemeHelper::Instance()->ApplyDefaultTheme();
    return true;
}
