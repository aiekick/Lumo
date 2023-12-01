#pragma once

#include <Gaia/Core/VulkanCore.h>
#include <LumoBackend/Base/BaseRenderer.h>

class DisplaySizeQuadRenderer;
typedef std::shared_ptr<DisplaySizeQuadRenderer> DisplaySizeQuadRendererPtr;
typedef std::weak_ptr<DisplaySizeQuadRenderer> DisplaySizeQuadRendererWeak;

class DisplaySizeQuadPass;
typedef std::shared_ptr<DisplaySizeQuadPass> DisplaySizeQuadPassPtr;
typedef std::weak_ptr<DisplaySizeQuadPass> DisplaySizeQuadPassWeak;

class DisplaySizeQuadPass;
class DisplaySizeQuadRenderer : public BaseRenderer {
public:
    static std::shared_ptr<DisplaySizeQuadRenderer> Create(GaiApi::VulkanCoreWeak vVulkanCore);

private:
    DisplaySizeQuadRendererWeak m_This;
    DisplaySizeQuadPassPtr m_DisplaySizeQuadPassPtr = nullptr;

public:
    DisplaySizeQuadRenderer(GaiApi::VulkanCoreWeak vVulkanCore);
    ~DisplaySizeQuadRenderer() override;

    bool init();

    void SetImageInfos(const vk::DescriptorImageInfo* vImageInfosPtr);
};