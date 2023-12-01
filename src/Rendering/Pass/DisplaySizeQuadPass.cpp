#include <Rendering/Pass/DisplaySizeQuadPass.h>
#include <Headers/Globals.h>
using namespace GaiApi;

//////////////////////////////////////////////////////////////
//// CHANNEL RENDERER PASS ///////////////////////////////////
//////////////////////////////////////////////////////////////

DisplaySizeQuadPass::DisplaySizeQuadPass(GaiApi::VulkanCoreWeak vVulkanCore) : QuadShaderPass(vVulkanCore, MeshShaderPassType::PIXEL) {
    ZoneScoped;
    SetRenderDocDebugName("Shader Pass : Display Size Quad", QUAD_SHADER_PASS_DEBUG_COLOR);
    m_DontUseShaderFilesOnDisk = true;
}

DisplaySizeQuadPass::~DisplaySizeQuadPass() {
    ZoneScoped;
    Unit();
}

void DisplaySizeQuadPass::ActionBeforeInit() {
    ZoneScoped;
    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);
    m_ImageInfos = *corePtr->getEmptyTexture2DDescriptorImageInfo();
}

void DisplaySizeQuadPass::SetImageInfos(const vk::DescriptorImageInfo* vImageInfosPtr) {
    if (vImageInfosPtr != nullptr) {
        m_ImageInfos = *vImageInfosPtr;
    } else {
        auto corePtr = m_VulkanCore.lock();
        assert(corePtr != nullptr);
        m_ImageInfos = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DisplaySizeQuadPass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;
    return AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
}

bool DisplaySizeQuadPass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;
    return AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos);
}

void DisplaySizeQuadPass::SetInputStateBeforePipelineCreation() {
    VertexStruct::P2_T2::GetInputState(m_InputState);
}

std::string DisplaySizeQuadPass::GetVertexShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "DisplaySizeQuadPass_Vertex";

    if (!m_VertexCode.empty()) {
        return m_VertexCode;
    }

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 v_uv;

void main() 
{
	v_uv = uv;
	gl_Position = vec4(pos, 0.0, 1.0);
}
)";
}

std::string DisplaySizeQuadPass::GetFragmentShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "DisplaySizeQuadPass_Fragment";

    if (!m_FragmentCode.empty()) {
        return m_FragmentCode;
    }

    return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 v_uv;
layout(binding = 0) uniform sampler2D u_texture;

void main() 
{
	fragColor = texture(u_texture, v_uv);
    fragColor.a = 1.0;
}
)";
}
