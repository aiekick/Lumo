
#if ZEP_SINGLE_HEADER == 1
#define ZEP_SINGLE_HEADER_BUILD
#endif

//#define ZEP_CONSOLE 
#include "ZepImGuiEditor.h"
#include <functional>
#include <filesystem>

#ifdef ZEP_CONSOLE
#include <zep\imgui\console_imgui.h>
#endif
namespace fs = std::filesystem;

using namespace Zep;

using cmdFunc = std::function<void(const std::vector<std::string>&)>; 
class ZepCmd : public ZepExCommand
{
public:
    ZepCmd(ZepEditor& editor, const std::string name, cmdFunc fn)
        : ZepExCommand(editor)
        , m_name(name)
        , m_func(fn)
    {
    }

    virtual void Run(const std::vector<std::string>& args) override
    {
        m_func(args);
    }

    virtual const char* ExCommandName() const override
    {
        return m_name.c_str();
    }

private:
    std::string m_name;
    cmdFunc m_func;
};

struct ZepWrapper : public Zep::IZepComponent
{
    ZepWrapper(const fs::path& root_path, const Zep::NVec2f& pixelScale, std::function<void(std::shared_ptr<Zep::ZepMessage>)> fnCommandCB)
        : zepEditor(Zep::ZepPath(root_path.string()), pixelScale)
        , Callback(fnCommandCB)
    {
        zepEditor.RegisterCallback(this);

    }

    virtual Zep::ZepEditor& GetEditor() const override
    {
        return (Zep::ZepEditor&)zepEditor;
    }

    virtual void Notify(std::shared_ptr<Zep::ZepMessage> message) override
    {
        Callback(message);

        return;
    }

    virtual void HandleInput()
    {
        zepEditor.HandleInput();
    }

    Zep::ZepEditor_ImGui zepEditor;
    std::function<void(std::shared_ptr<Zep::ZepMessage>)> Callback;
};

std::shared_ptr<ZepWrapper> spZep;


// From here: https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.00.pdf
static std::unordered_set<std::string> glsl_keywords{
    "#version", "attribute", "const", "uniform", "varying", "layout", "centroid", "flat", "smooth", "noperspective", "patch", "sample", "break", "continue", "do", "for", "while", "switch", "case", "default",
    "if", "else", "subroutine", "in", "out", "inout", "float", "double", "int", "void", "bool", "true", "false", "invariant", "discard", "return", "mat2", "mat3", "mat4", "dmat2", "dmat3", "dmat4",
    "mat2x2", "mat2x3", "mat2x4", "dmat2x2", "dmat2x3", "dmat2x4", "mat3x2", "mat3x3", "mat3x4", "dmat3x2", "dmat3x3", "dmat3x4", "mat4x2", "mat4x3", "mat4x4", "dmat4x2", "dmat4x3", "dmat4x4",
    "vec2", "vec3", "vec4", "ivec2", "ivec3", "ivec4", "bvec2", "bvec3", "bvec4", "dvec2", "dvec3", "dvec4", "uint", "uvec2", "uvec3", "uvec4", "lowp", "mediump", "highp", "precision",
    "sampler1D", "sampler2D", "sampler3D", "samplerCube", "sampler1DShadow", "sampler2DShadow", "samplerCubeShadow", "sampler1DArray", "sampler2DArray", "sampler1DArrayShadow", "sampler2DArrayShadow",
    "isampler1D", "isampler2D", "isampler3D", "isamplerCube", "isampler1DArray", "isampler2DArray", "usampler1D", "usampler2D", "usampler3D", "usamplerCube", "usampler1DArray", "usampler2DArray",
    "sampler2DRect", "sampler2DRectShadow", "isampler2DRect", "usampler2DRect", "samplerBuffer", "isamplerBuffer", "usamplerBuffer", "sampler2DMS", "isampler2DMS", "usampler2DMS",
    "sampler2DMSArray", "isampler2DMSArray", "usampler2DMSArray", "samplerCubeArray", "samplerCubeArrayShadow", "isamplerCubeArray", "usamplerCubeArray", "struct",
    "gl_Position", "binding", "location", "EmitVertex", "EndPrimitive", "gl_in", "triangles", "line_strip", "max_vertices"
};

static std::unordered_set<std::string> glsl_identifiers = {
    "abort", "abs", "acos", "asin", "atan", "atexit", "atof", "atoi", "atol", "ceil", "clock", "cosh", "ctime", "div", "exit", "fabs", "floor", "fmod", "getchar", "getenv", "isalnum", "isalpha", "isdigit", "isgraph",
    "ispunct", "isspace", "isupper", "kbhit", "log10", "log2", "log", "memcmp", "modf", "pow", "putchar", "putenv", "puts", "rand", "remove", "rename", "sinh", "sqrt", "srand", "strcat", "strcmp", "strerror", "time", "tolower", "toupper",
};

void zep_init(const Zep::NVec2f& pixelScale)
{
    spZep = std::make_shared<ZepWrapper>(PROJECT_PATH, Zep::NVec2f(pixelScale.x, pixelScale.y), 
        [](std::shared_ptr<ZepMessage> spMessage) -> void 
        {
            // to call for changes tracking
        }
    );

    spZep->GetEditor().RegisterSyntaxFactory({".vk_vert", ".vk_frag", ".vk_geom"}, 
        SyntaxProvider{"glsl_shader", tSyntaxFactory([](ZepBuffer* pBuffer) {
            return std::make_shared<ZepSyntax>(*pBuffer, glsl_keywords, glsl_identifiers);
            })
        });

    auto& display = spZep->GetEditor().GetDisplay();
    auto pImFont = ImGui::GetIO().Fonts[0].Fonts[0];
    auto pixelHeight = pImFont->FontSize * pImFont->Scale;
    display.SetFont(ZepTextType::UI, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight)));
    display.SetFont(ZepTextType::Text, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight)));
    display.SetFont(ZepTextType::Heading1, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight * 1.5)));
    display.SetFont(ZepTextType::Heading2, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight * 1.25)));
    display.SetFont(ZepTextType::Heading3, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight * 1.125)));
}

void zep_update()
{
    if (spZep)
    {
        spZep->GetEditor().RefreshRequired(); 
    }
}

void zep_destroy()
{
    spZep.reset();
}

ZepEditor& zep_get_editor()
{
    return spZep->GetEditor();
}

void zep_load(const std::string& vName, const std::string& vCode)
{
    zep_get_editor().InitWithText(vName, vCode);
    zep_get_editor().SetGlobalMode(ZepMode_Standard::StaticName());
}

void zep_show(const Zep::NVec2i& displaySize)
{
    bool show = true;

    const auto pBuffer = spZep->GetEditor().GetActiveBuffer();

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::BeginMenu("Editor Mode", pBuffer))
            {
                bool enabledVim = strcmp(pBuffer->GetMode()->Name(), Zep::ZepMode_Vim::StaticName()) == 0;
                bool enabledNormal = !enabledVim;
                if (ImGui::MenuItem("Vim", "CTRL+2", &enabledVim))
                {
                    spZep->GetEditor().SetGlobalMode(Zep::ZepMode_Vim::StaticName());
                }
                else if (ImGui::MenuItem("Standard", "CTRL+1", &enabledNormal))
                {
                    spZep->GetEditor().SetGlobalMode(Zep::ZepMode_Standard::StaticName());
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Theme"))
            {
                bool enabledDark = spZep->GetEditor().GetTheme().GetThemeType() == ThemeType::Dark ? true : false;
                bool enabledLight = !enabledDark;

                if (ImGui::MenuItem("Dark", "", &enabledDark))
                {
                    spZep->GetEditor().GetTheme().SetThemeType(ThemeType::Dark);
                }
                else if (ImGui::MenuItem("Light", "", &enabledLight))
                {
                    spZep->GetEditor().GetTheme().SetThemeType(ThemeType::Light);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window"))
        {
            auto pTabWindow = spZep->GetEditor().GetActiveTabWindow();
            bool selected = false;
            if (ImGui::MenuItem("Horizontal Split", "", &selected, pBuffer != nullptr))
            {
                pTabWindow->AddWindow(pBuffer, pTabWindow->GetActiveWindow(), RegionLayoutType::VBox);
            }
            else if (ImGui::MenuItem("Vertical Split", "", &selected, pBuffer != nullptr))
            {
                pTabWindow->AddWindow(pBuffer, pTabWindow->GetActiveWindow(), RegionLayoutType::HBox);
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }    

    auto min = ImGui::GetCursorScreenPos();
    auto max = ImGui::GetContentRegionAvail();
    if (max.x <= 0)
        max.x = 1;
    if (max.y <= 0)
        max.y = 1;
    ImGui::InvisibleButton("ZepContainer", max);

    // Fill the window
    max.x = min.x + max.x;
    max.y = min.y + max.y;

    spZep->zepEditor.SetDisplayRegion(Zep::NVec2f(min.x, min.y), Zep::NVec2f(max.x, max.y));
    spZep->zepEditor.Display();
    bool zep_focused = ImGui::IsWindowFocused();
    if (zep_focused)
    {
        spZep->zepEditor.HandleInput();
    }

    // TODO: A Better solution for this; I think the audio graph is creating a new window and stealing focus
    static int focus_count = 0;
    if (focus_count++ < 2)
    {
        ImGui::SetWindowFocus();
    }
}
