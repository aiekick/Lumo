#pragma once

#include <ImGuiPack.h>
#include <cstdint>
#include <memory>
#include <string>

class ProjectFile;
class ConsolePane : public AbstractPane {
public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(
        const uint32_t& vCurrentFrame, PaneFlags& vInOutPaneShown, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

public:  // singleton
    static std::shared_ptr<ConsolePane> Instance() {
        static std::shared_ptr<ConsolePane> _instance = std::make_shared<ConsolePane>();
        return _instance;
    }

public:
    ConsolePane();                              // Prevent construction
    ConsolePane(const ConsolePane&) = default;  // Prevent construction by copying
    ConsolePane& operator=(const ConsolePane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~ConsolePane();  // Prevent unwanted destruction};
};
