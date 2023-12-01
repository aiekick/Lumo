#include "DisplaySizeQuadRenderer.h"
#include <Gaia/Core/VulkanCore.h>
#include <Rendering/Pass/DisplaySizeQuadPass.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / STATIC ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

DisplaySizeQuadRendererPtr DisplaySizeQuadRenderer::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res = std::make_shared<DisplaySizeQuadRenderer>(vVulkanCore);
    res->m_This = res;
    if (!res->init()) {
        res.reset();
    }
    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / CONSTRUCTOR //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

DisplaySizeQuadRenderer::DisplaySizeQuadRenderer(GaiApi::VulkanCoreWeak vVulkanCore) : BaseRenderer(vVulkanCore) {
    ZoneScoped;
}

DisplaySizeQuadRenderer::~DisplaySizeQuadRenderer() {
    ZoneScoped;
    m_DisplaySizeQuadPassPtr.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / INIT /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DisplaySizeQuadRenderer::init() {
    ZoneScoped;

    ct::uvec2 map_size = 512;

    m_Loaded = true;

    if (BaseRenderer::InitPixel(map_size)) {
        m_DisplaySizeQuadPassPtr = std::make_shared<DisplaySizeQuadPass>(m_VulkanCore);
        if (m_DisplaySizeQuadPassPtr != nullptr) {
            auto corePtr = m_VulkanCore.lock();
            assert(corePtr != nullptr);
            if (m_DisplaySizeQuadPassPtr->InitPixelWithoutFBO(map_size, 1U, false, &corePtr->getMainRenderPassRef(), vk::SampleCountFlagBits::e1)) {
                AddGenericPass(m_DisplaySizeQuadPassPtr);
                SetMergedRendering(true);
                m_Loaded = true;
            }
        }
    }

    return m_Loaded;
}

void DisplaySizeQuadRenderer::SetImageInfos(const vk::DescriptorImageInfo* vImageInfosPtr) {
    if (m_DisplaySizeQuadPassPtr != nullptr) {
        m_DisplaySizeQuadPassPtr->SetImageInfos(vImageInfosPtr);
        // en temps normal UpdateDescriptors est fait automatiquement, mais ici on chinte le systeme pour juste faire un appel a drawmodel
        // dans le cas du rendu dnas la render pass de la swap chain
        // alors il faut update les descriptors nous meme
        UpdateDescriptorsBeforeCommandBuffer();
    }
}
