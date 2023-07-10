/*
Copyright 2022-2022 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once
#pragma warning(disable : 4251)

#include <string>
#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>
#include <gaia/VulkanCore.h>
#include <gaia/VulkanRessource.h>
#include <gaia/gaia.h>

class GAIA_API Texture2D
{
public:
	static bool loadPNG(const std::string& inFile, std::vector<uint8_t>& outBuffer, uint32_t& outWidth, uint32_t& outHeight);
	static bool loadImage(const std::string& inFile, std::vector<uint8_t>& outBuffer, uint32_t& outWidth, uint32_t& outHeight, uint32_t& outChannels);
	static bool loadImageWithMaxH(const std::string& inFile, const uint32_t& maxHeight, uint32_t& outWidth, std::vector<uint8_t>& outBuffer);
	//static vk::DescriptorImageInfo GetImageInfoFromMemory(vkApi::VulkanCorePtr vVulkanCorePtr, uint8_t* buffer, const uint32_t& width, const uint32_t& height, const uint32_t& channels);

public:
	static Texture2DPtr CreateFromFile(vkApi::VulkanCorePtr vVulkanCorePtr, std::string vFilePathName, const uint32_t& vMaxHeight = 0U);
	static Texture2DPtr CreateFromMemory(vkApi::VulkanCorePtr vVulkanCorePtr, uint8_t* buffer, const uint32_t& width, const uint32_t& height, const uint32_t& channels);
	static Texture2DPtr CreateEmptyTexture(vkApi::VulkanCorePtr vVulkanCorePtr, ct::uvec2 vSize, vk::Format vFormat);
	static Texture2DPtr CreateEmptyImage(vkApi::VulkanCorePtr vVulkanCorePtr, ct::uvec2 vSize, vk::Format vFormat);

public:
	std::shared_ptr<VulkanImageObject> m_Texture2D = nullptr;
	vk::ImageView m_TextureView = {};
	vk::Sampler m_Sampler = {};
	vk::DescriptorImageInfo m_DescriptorImageInfo = {};
	uint32_t m_MipLevelCount = 1u;
	uint32_t m_Width = 0u;
	uint32_t m_Height = 0u;
	float m_Ratio = 0.0f;
	bool m_Loaded = false;

private:
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;

public:
	Texture2D(vkApi::VulkanCorePtr vVulkanCorePtr);
	~Texture2D();
	bool LoadFile(const std::string& vFilePathName, const vk::Format& vFormat = vk::Format::eR8G8B8A8Unorm, const uint32_t& vMipLevelCount = 1u, const uint32_t& vMaxHeight = 0U);
	bool LoadMemory(uint8_t* buffer, const uint32_t& width, const uint32_t& height, const uint32_t& channels, const vk::Format& vFormat = vk::Format::eR8G8B8A8Unorm, const uint32_t& vMipLevelCount = 1u);
	bool LoadEmptyTexture(const ct::uvec2& vSize = 1, const vk::Format& vFormat = vk::Format::eR8G8B8A8Unorm);
	bool LoadEmptyImage(const ct::uvec2& vSize = 1, const vk::Format& vFormat = vk::Format::eR8G8B8A8Unorm);
	void Destroy();

public:
	bool SaveToPng(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::uvec2& vNewSize);
	bool SaveToBmp(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::uvec2& vNewSize);
	bool SaveToJpg(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const int& vQualityFrom0To100, const ct::uvec2& vNewSize);
	bool SaveToHdr(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::uvec2& vNewSize);
	bool SaveToTga(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::uvec2& vNewSize);
};