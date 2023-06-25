#pragma once
#include <memory>
#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>

#if defined(__WIN32__) || defined(WIN32) || defined(_WIN32) || defined(__WIN64__) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
#if defined(vkFramework_EXPORTS)
#define VKFRAMEWORK_API __declspec(dllexport)
#elif defined(BUILD_SHARED_LIBS)
#define VKFRAMEWORK_API __declspec(dllimport)
#else
#define VKFRAMEWORK_API
#endif
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__EMSCRIPTEN__) || defined(__APPLE__)
#define VKFRAMEWORK_API
#endif


typedef std::vector<ct::fvec2> fvec2Vector;
typedef std::vector<vk::DescriptorImageInfo> DescriptorImageInfoVector;

class Texture2D;
typedef std::shared_ptr<Texture2D> Texture2DPtr;
typedef ct::cWeak<Texture2D> Texture2DWeak;

class TextureCube;
typedef std::shared_ptr<TextureCube> TextureCubePtr;
typedef ct::cWeak<TextureCube> TextureCubeWeak;

class VulkanImGuiRenderer;
typedef std::shared_ptr<VulkanImGuiRenderer> VulkanImGuiRendererPtr;
typedef ct::cWeak<VulkanImGuiRenderer> VulkanImGuiRendererWeak;

class VulkanShader;
typedef std::shared_ptr<VulkanShader> VulkanShaderPtr;
typedef ct::cWeak<VulkanShader> VulkanShaderWeak;

namespace vkApi
{
	class VulkanSwapChain;
	typedef std::shared_ptr<VulkanSwapChain> VulkanSwapChainPtr;
	typedef ct::cWeak<VulkanSwapChain> VulkanSwapChainWeak;

	class VulkanCore;
	typedef std::shared_ptr<VulkanCore> VulkanCorePtr;
	typedef ct::cWeak<VulkanCore> VulkanCoreWeak;

	class VulkanWindow;
	typedef std::shared_ptr<VulkanWindow> VulkanWindowPtr;
	typedef ct::cWeak<VulkanWindow> VulkanWindowWeak;

	class VulkanDevice;
	typedef std::shared_ptr<VulkanDevice> VulkanDevicePtr;
	typedef ct::cWeak<VulkanDevice> VulkanDeviceWeak;

	class VulkanImGuiOverlay;
	typedef std::shared_ptr<VulkanImGuiOverlay> VulkanImGuiOverlayPtr;
	typedef ct::cWeak<VulkanImGuiOverlay> VulkanImGuiOverlayWeak;
}
