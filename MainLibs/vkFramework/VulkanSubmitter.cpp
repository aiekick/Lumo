/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "VulkanSubmitter.h"
#include "VulkanCore.h"
#include <ctools/Logger.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

namespace vkApi
{
	// STATIC
	std::mutex VulkanSubmitter::criticalSectionMutex;

	bool VulkanSubmitter::Submit(
		vkApi::VulkanCore* vVulkanCore,
		vk::QueueFlagBits vQueueType,
		vk::SubmitInfo vSubmitInfo,
		vk::Fence vWaitFence)
	{
		ZoneScoped;

		std::unique_lock<std::mutex> lck(VulkanSubmitter::criticalSectionMutex, std::defer_lock);
		lck.lock();
		auto result = vVulkanCore->getQueue(vQueueType).vkQueue.submit(1, &vSubmitInfo, vWaitFence);
		if (result == vk::Result::eErrorDeviceLost)
		{
			// driver lost, we'll crash in this case:
			LogVarError("Driver Lost after submit");
			return false;
		}
		lck.unlock();

		return true;
	}
}