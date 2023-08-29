#include <LumoBackend/Base/TaskRenderer.h>

TaskRenderer::TaskRenderer(GaiApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{

}

TaskRenderer::TaskRenderer(GaiApi::VulkanCorePtr vVulkanCorePtr, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool)
	: BaseRenderer(vVulkanCorePtr, vCommandPool, vDescriptorPool)
{

}

bool TaskRenderer::IsTheGoodFrame(const uint32_t& vFrame)
{
	if (m_LastExecutedFrame == vFrame)
		return true;

	for (auto pass : m_ShaderPasses)
	{
		auto pass_ptr = pass.lock();
		if (pass_ptr)
		{
			if (pass_ptr->GetLastExecutedFrame() == vFrame)
				return true;
		}
	}

	return false;
}