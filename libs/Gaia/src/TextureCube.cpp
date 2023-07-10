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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <gaia/TextureCube.h>
#include <lodepng/lodepng.h>
#include <ctools/Logger.h>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <stb/stb_image_resize.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// STATIC /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TextureCube::loadPNG(const std::string& inFile, std::vector<uint8_t>& outBuffer, uint32_t& outWidth, uint32_t& outHeight)
{
	ZoneScoped;

	std::vector<uint8_t> raw_buf;
	if (lodepng::load_file(raw_buf, inFile) != 0)
		return false;
	lodepng::State state;
	if (lodepng::decode(outBuffer, outWidth, outHeight, state, raw_buf) != 0)
		return false;

	return true;
}

bool TextureCube::loadImage(const std::string& inFile, std::vector<uint8_t>& outBuffer, uint32_t& outWidth, uint32_t& outHeight, uint32_t& outChannels)
{
	ZoneScoped;

	int w, h, chans;
	unsigned char* data = stbi_load(inFile.c_str(), &w, &h, &chans, STBI_rgb_alpha);

	outWidth = static_cast<uint32_t>(w);
	outHeight = static_cast<uint32_t>(h);
	outChannels = static_cast<uint32_t>(chans);

	if (!data)
		return false;

	auto size = (size_t)outWidth * (size_t)outHeight * 4;
	outBuffer.resize(size);
	memcpy(outBuffer.data(), data, size);
	stbi_image_free(data);

	return true;
}

/*vk::DescriptorImageInfo TextureCube::GetImageInfoFromMemory(vkApi::VulkanCorePtr vVulkanCorePtr, std::array<uint8_t*, 6U> vBuffers, const uint32_t& width, const uint32_t& height, const uint32_t& channels)
{
	vk::DescriptorImageInfo imgInfo;

	if (width == 0 || height == 0 || channels == 0)
		return imgInfo;

	bool isok = true;
	for (auto buffer : vBuffers)
	{
		if (!buffer)
		{
			isok = false;
			break;
		}
	}

	if (isok)
	{
		auto tex2d = VulkanRessource::createTextureImage2D(vVulkanCorePtr, width, height, 1U, vk::Format::eR8G8B8A8Unorm, buffer);

		vk::ImageViewCreateInfo imViewInfo = {};
		imViewInfo.flags = vk::ImageViewCreateFlags();
		imViewInfo.image = tex2d->image;
		imViewInfo.viewType = vk::ImageViewType::e2D;
		imViewInfo.format = vk::Format::eR8G8B8A8Unorm;
		imViewInfo.components = vk::ComponentMapping();
		imViewInfo.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1U, 0, 1);
		imgInfo.imageView = vVulkanCorePtr->getDevice().createImageView(imViewInfo);

		vk::SamplerCreateInfo samplerInfo = {};
		samplerInfo.flags = vk::SamplerCreateFlags();
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge; // U
		samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge; // V
		samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge; // W
		samplerInfo.mipLodBias = 0.0f;
		//samplerInfo.anisotropyEnable = false;
		//samplerInfo.maxAnisotropy = 0.0f;
		//samplerInfo.compareEnable = false;
		//samplerInfo.compareOp = vk::CompareOp::eAlways;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(1U);
		//samplerInfo.unnormalizedCoordinates = false;
		imgInfo.sampler = vVulkanCorePtr->getDevice().createSampler(samplerInfo);

		imgInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	}

	return imgInfo;
}
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TextureCubePtr TextureCube::CreateFromFiles(vkApi::VulkanCorePtr vVulkanCorePtr, std::array<std::string,6U> vFilePathNames)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<TextureCube>(vVulkanCorePtr);

	if (!res->LoadFiles(vFilePathNames))
	{
		res.reset();
	}

	return res;
}

/*
TextureCubePtr TextureCube::CreateFromMemory(vkApi::VulkanCorePtr vVulkanCorePtr, uint8_t* buffer, const uint32_t& width, const uint32_t& height, const uint32_t& channels)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<TextureCube>(vVulkanCorePtr);

	if (!res->LoadMemory(buffer, width, height, channels))
	{
		res.reset();
	}

	return res;
}
*/

TextureCubePtr TextureCube::CreateEmptyTexture(vkApi::VulkanCorePtr vVulkanCorePtr, ct::uvec2 vSize, vk::Format vFormat)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<TextureCube>(vVulkanCorePtr);

	if (!res->LoadEmptyTexture(vSize, vFormat))
	{
		res.reset();
	}

	return res;
}

/*
TextureCubePtr TextureCube::CreateEmptyImage(vkApi::VulkanCorePtr vVulkanCorePtr, ct::uvec2 vSize, vk::Format vFormat)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<TextureCube>(vVulkanCorePtr);

	if (!res->LoadEmptyImage(vSize, vFormat))
	{
		res.reset();
	}

	return res;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TextureCube::TextureCube(vkApi::VulkanCorePtr vVulkanCorePtr)
	: m_VulkanCorePtr(vVulkanCorePtr)
{
	ZoneScoped;
}

TextureCube::~TextureCube()
{
	ZoneScoped;

	Destroy();
}

static inline uint32_t GetMiplevelCount(uint32_t width, uint32_t height)
{
	ZoneScoped;

	uint32_t levels = 0;
	while (width || height)
	{
		width >>= 1;
		height >>= 1;
		++levels;
	}
	// other fucntion :
	// levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	return levels;
}

bool TextureCube::LoadFiles(const std::array<std::string, 6U>& vFilePathNames, const vk::Format& vFormat, const uint32_t& vMipLevelCount)
{
	ZoneScoped;

	m_Loaded = false;

	bool isok = true;
	for (auto filePathName : vFilePathNames)
	{
		if (filePathName.empty())
		{
			isok = false;
			break;
		}
	}

	if (isok)
	{
		Destroy();
		std::array<std::vector<uint8_t>, 6U> buffers;

		uint32_t channels = 4;
		for (size_t idx = 0U; idx < 6U; ++idx)
		{
			std::vector<uint8_t> image_data;
			if (!loadImage(vFilePathNames[idx], image_data, m_Width, m_Height, channels))
				return false;
			{
				buffers[idx] = image_data;
			}

			if (m_Width == 0 || m_Height == 0)
				return false;
		}

		LoadMemories(buffers, m_Width, m_Height, channels, vFormat, vMipLevelCount);
	}

	return m_Loaded;
}

bool TextureCube::LoadMemories(
	const std::array<std::vector<uint8_t>, 6U>& buffers,
	const uint32_t& width, const uint32_t& height, const uint32_t& channels,
	const vk::Format& vFormat, const uint32_t& vMipLevelCount)
{
	ZoneScoped;

	m_Loaded = false;

	if (width == 0 || height == 0 || channels == 0)
		return false;

	bool isok = true;
	for (auto buffer : buffers)
	{
		if (buffer.empty())
		{
			isok = false;
			break;
		}
	}

	if (isok)
	{
		m_Width = width;
		m_Height = height;

		uint32_t maxMipLevelCount = GetMiplevelCount(m_Width, m_Height);

		m_MipLevelCount = ct::clamp(vMipLevelCount, 1u, maxMipLevelCount);

		m_TextureCubePtr = VulkanRessource::createTextureImageCube(m_VulkanCorePtr, m_Width, m_Height, m_MipLevelCount, vFormat, buffers);
		if (m_TextureCubePtr)
		{
			vk::ImageViewCreateInfo imViewInfo = {};
			imViewInfo.flags = vk::ImageViewCreateFlags();
			imViewInfo.image = m_TextureCubePtr->image;
			imViewInfo.viewType = vk::ImageViewType::eCube;
			imViewInfo.format = vFormat;
			imViewInfo.components = vk::ComponentMapping();
			imViewInfo.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0U, m_MipLevelCount, 0U, 6U);
			m_TextureView = m_VulkanCorePtr->getDevice().createImageView(imViewInfo);

			vk::SamplerCreateInfo samplerInfo = {};
			samplerInfo.flags = vk::SamplerCreateFlags();
			samplerInfo.magFilter = vk::Filter::eLinear;
			samplerInfo.minFilter = vk::Filter::eLinear;
			samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
			samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge; // U
			samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge; // V
			samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge; // W
			samplerInfo.mipLodBias = 0.0f;
			//samplerInfo.anisotropyEnable = false;
			//samplerInfo.maxAnisotropy = 0.0f;
			//samplerInfo.compareEnable = false;
			//samplerInfo.compareOp = vk::CompareOp::eAlways;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = static_cast<float>(m_MipLevelCount);
			//samplerInfo.unnormalizedCoordinates = false;
			m_Sampler = m_VulkanCorePtr->getDevice().createSampler(samplerInfo);

			m_DescriptorImageInfo.sampler = m_Sampler;
			m_DescriptorImageInfo.imageView = m_TextureView;
			m_DescriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			m_Ratio = (float)m_Width / (float)m_Height;

			m_Loaded = true;
		}
	}

	return m_Loaded;
}

bool TextureCube::LoadEmptyTexture(const ct::uvec2& vSize, const vk::Format& vFormat)
{
	ZoneScoped;

	m_Loaded = false;

	Destroy();

	std::array<std::vector<uint8_t>, 6U> image_datas;

	uint32_t channels = 0;
	uint32_t elem_size = 0;

	switch (vFormat)
	{
	case vk::Format::eB8G8R8A8Unorm:
	case vk::Format::eR8G8B8A8Unorm:
		channels = 4;
		elem_size = 8 / 8;
		break;
	case vk::Format::eB8G8R8Unorm:
	case vk::Format::eR8G8B8Unorm:
		channels = 3;
		elem_size = 8 / 8;
		break;
	case vk::Format::eR8Unorm:
		channels = 1;
		elem_size = 8 / 8;
		break;
	case vk::Format::eD16Unorm:
		channels = 1;
		elem_size = 16 / 8;
		break;
	case vk::Format::eR32G32B32A32Sfloat:
		channels = 4;
		elem_size = 32 / 8; // sizeof(float)
		break;
	case vk::Format::eR32G32B32Sfloat:
		channels = 3;
		elem_size = 32 / 8; // sizeof(float)
		break;
	case vk::Format::eR32Sfloat:
		channels = 1;
		elem_size = 32 / 8; // sizeof(float)
		break;
	default:
		LogVarDebug("Debug : unsupported type: %s", vk::to_string(vFormat).c_str());
		throw std::invalid_argument("unsupported fomat type!");
	}

	size_t dataSize = vSize.x * vSize.y;
	if (dataSize > 1)
	{
		for (auto& image_data : image_datas)
		{
			image_data.resize(dataSize * channels * elem_size);
			memset(image_data.data(), 0, image_data.size());
		}
	}
	else
	{
		for (auto& image_data : image_datas)
		{
			image_data.resize(channels * elem_size);
			memset(image_data.data(), 0, image_data.size());
		}
	}
	
	m_TextureCubePtr = VulkanRessource::createTextureImageCube(
		m_VulkanCorePtr, vSize.x, vSize.y, 1, vFormat, image_datas);

	vk::ImageViewCreateInfo imViewInfo = {};
	imViewInfo.flags = vk::ImageViewCreateFlags();
	imViewInfo.image = m_TextureCubePtr->image;
	imViewInfo.viewType = vk::ImageViewType::eCube;
	imViewInfo.format = vFormat;
	imViewInfo.components = vk::ComponentMapping();
	imViewInfo.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, m_MipLevelCount, 0, 6U);
	m_TextureView = m_VulkanCorePtr->getDevice().createImageView(imViewInfo);

	vk::SamplerCreateInfo samplerInfo = {};
	samplerInfo.flags = vk::SamplerCreateFlags();
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.minFilter = vk::Filter::eLinear;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge; // U
	samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge; // V
	samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge; // W
	//samplerInfo.mipLodBias = 0.0f;
	//samplerInfo.anisotropyEnable = false;
	//samplerInfo.maxAnisotropy = 0.0f;
	//samplerInfo.compareEnable = false;
	//samplerInfo.compareOp = vk::CompareOp::eAlways;
	//samplerInfo.minLod = 0.0f;
	//samplerInfo.maxLod = static_cast<float>(m_MipLevelCount);
	//samplerInfo.unnormalizedCoordinates = false;
	m_Sampler = m_VulkanCorePtr->getDevice().createSampler(samplerInfo);

	m_DescriptorImageInfo.sampler = m_Sampler;
	m_DescriptorImageInfo.imageView = m_TextureView;
	m_DescriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	m_Ratio = (float)m_Width / (float)m_Height;

	m_Loaded = true;

	return m_Loaded;
}

// for compute
/*
bool TextureCube::LoadEmptyImage(const ct::uvec2& vSize, const vk::Format& vFormat)
{
	ZoneScoped;

	m_Loaded = false;

	Destroy();

	m_TextureCube = VulkanRessource::createComputeTarget2D(m_VulkanCorePtr, vSize.x, vSize.y, 1U, vFormat, vk::SampleCountFlagBits::e1);

	vk::ImageViewCreateInfo imViewInfo = {};
	imViewInfo.flags = vk::ImageViewCreateFlags();
	imViewInfo.image = m_TextureCube->image;
	imViewInfo.viewType = vk::ImageViewType::e2D;
	imViewInfo.format = vFormat;
	imViewInfo.components = vk::ComponentMapping();
	imViewInfo.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0U, m_MipLevelCount, 0U, 1U);
	m_TextureView = m_VulkanCorePtr->getDevice().createImageView(imViewInfo);

	vk::SamplerCreateInfo samplerInfo = {};
	samplerInfo.flags = vk::SamplerCreateFlags();
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.minFilter = vk::Filter::eLinear;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge; // U
	samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge; // V
	samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge; // W
	//samplerInfo.mipLodBias = 0.0f;
	//samplerInfo.anisotropyEnable = false;
	//samplerInfo.maxAnisotropy = 0.0f;
	//samplerInfo.compareEnable = false;
	//samplerInfo.compareOp = vk::CompareOp::eAlways;
	//samplerInfo.minLod = 0.0f;
	//samplerInfo.maxLod = static_cast<float>(m_MipLevelCount);
	//samplerInfo.unnormalizedCoordinates = false;
	m_Sampler = m_VulkanCorePtr->getDevice().createSampler(samplerInfo);

	m_DescriptorImageInfo.sampler = m_Sampler;
	m_DescriptorImageInfo.imageView = m_TextureView;
	m_DescriptorImageInfo.imageLayout = vk::ImageLayout::eGeneral;

	m_Ratio = (float)m_Width / (float)m_Height;

	m_Loaded = true;

	return m_Loaded;
}
*/

void TextureCube::Destroy()
{
	ZoneScoped;

	if (!m_Loaded) return;

	m_VulkanCorePtr->getDevice().waitIdle();
	m_VulkanCorePtr->getDevice().destroySampler(m_Sampler);
	m_VulkanCorePtr->getDevice().destroyImageView(m_TextureView);
	m_TextureCubePtr.reset();

	m_Loaded = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// SAVE TO PICTURE FILES (PBG, BMP, TGA, HDR) SO STB EXPORT FILES //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
bool TextureCube::SaveToPng(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::uvec2& vNewSize)
{
	ZoneScoped;

	bool res = false;

	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t bufSize = 0;
	uint8_t* bmBytesRGBA = nullptr;// GetRGBABytesFromFrameBuffer(&width, &height, &bufSize, vAttachmentId);

	uint32_t bytesPerPixel = 4;

	int ss = vSubSamplesCount;

	// Sub Sampling
	if (ss > 0 && bufSize > 0 && bmBytesRGBA)
	{
		auto tempdata = new uint8_t[bufSize];
		memcpy(tempdata, bmBytesRGBA, bufSize);

		uint32_t indexMaxSize = width * height;
		for (uint32_t index = 0; index < indexMaxSize; ++index)
		{
			uint32_t count = 0;
			uint32_t r = 0, g = 0, b = 0, a = 0;

			for (int i = -ss; i <= ss; i += ss)
			{
				for (int j = -ss; j <= ss; j += ss)
				{
					int ssIndex = index + width * j + i;
					if (ssIndex > 0 && (unsigned int)ssIndex < indexMaxSize)
					{
						r += tempdata[ssIndex * bytesPerPixel + 0];
						g += tempdata[ssIndex * bytesPerPixel + 1];
						b += tempdata[ssIndex * bytesPerPixel + 2];
						a += tempdata[ssIndex * bytesPerPixel + 3];

						++count;
					}
				}
			}

			bmBytesRGBA[index * bytesPerPixel + 0] = (uint8_t)((float)r / (float)count);
			bmBytesRGBA[index * bytesPerPixel + 1] = (uint8_t)((float)g / (float)count);
			bmBytesRGBA[index * bytesPerPixel + 2] = (uint8_t)((float)b / (float)count);
			bmBytesRGBA[index * bytesPerPixel + 3] = (uint8_t)((float)a / (float)count);
		}

		SAFE_DELETE_ARRAY(tempdata);
	}

	// resize
	if (vNewSize.x != width || vNewSize.y != height)
	{
		// resize
		int newWidth = vNewSize.x;
		int newHeight = vNewSize.y;
		unsigned int newBufSize = newWidth * newHeight * bytesPerPixel;
		auto resizedData = new uint8_t[newBufSize];

		int resizeRes = stbir_resize_uint8(bmBytesRGBA, width, height, width * bytesPerPixel,
			resizedData, newWidth, newHeight, newWidth * bytesPerPixel,
			bytesPerPixel);

		if (resizeRes)
		{
			if (vFlipY)
				stbi_flip_vertically_on_write(1);
			int resWrite = stbi_write_png(vFilePathName.c_str(),
				newWidth,
				newHeight,
				bytesPerPixel,
				resizedData,
				newWidth * bytesPerPixel);

			if (resWrite)
				res = true;
		}

		SAFE_DELETE_ARRAY(resizedData);
	}
	else
	{
		if (vFlipY)
			stbi_flip_vertically_on_write(1);
		int resWrite = stbi_write_png(vFilePathName.c_str(),
			width,
			height,
			bytesPerPixel,
			bmBytesRGBA,
			width * bytesPerPixel);

		if (resWrite)
			res = true;
	}

	SAFE_DELETE_ARRAY(bmBytesRGBA);

	return res;
}
*/

/*
bool TextureCube::SaveToBmp(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::uvec2& vNewSize)
{
	ZoneScoped;

	bool res = false;

	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t bufSize = 0;
	uint8_t* bmBytesRGB = 0;// GetRGBBytesFromFrameBuffer(&width, &height, &bufSize, vAttachmentId);

	uint32_t bytesPerPixel = 3;

	int ss = vSubSamplesCount;

	// Sub Sampling
	if (ss > 0)
	{
		uint8_t* tempdata = new uint8_t[bufSize];
		memcpy(tempdata, bmBytesRGB, bufSize);

		uint32_t indexMaxSize = width * height;
		for (uint32_t index = 0; index < indexMaxSize; ++index)
		{
			uint32_t count = 0;
			uint32_t r = 0, g = 0, b = 0;

			for (int i = -ss; i <= ss; i += ss)
			{
				for (int j = -ss; j <= ss; j += ss)
				{
					int ssIndex = index + width * j + i;
					if (ssIndex > 0 && (unsigned int)ssIndex < indexMaxSize)
					{
						r += tempdata[ssIndex * bytesPerPixel + 0];
						g += tempdata[ssIndex * bytesPerPixel + 1];
						b += tempdata[ssIndex * bytesPerPixel + 2];

						++count;
					}
				}
			}

			bmBytesRGB[index * bytesPerPixel + 0] = (uint8_t)((float)r / (float)count);
			bmBytesRGB[index * bytesPerPixel + 1] = (uint8_t)((float)g / (float)count);
			bmBytesRGB[index * bytesPerPixel + 2] = (uint8_t)((float)b / (float)count);
		}

		SAFE_DELETE_ARRAY(tempdata);
	}

	// resize
	if (vNewSize.x != width || vNewSize.y != height)
	{
		// resize
		uint32_t newWidth = vNewSize.x;
		uint32_t newHeight = vNewSize.y;
		uint32_t newBufSize = newWidth * newHeight * bytesPerPixel;
		uint8_t* resizedData = new uint8_t[newBufSize];

		int resizeRes = stbir_resize_uint8(bmBytesRGB, width, height, width * bytesPerPixel,
			resizedData, newWidth, newHeight, newWidth * bytesPerPixel,
			bytesPerPixel);

		if (resizeRes)
		{
			if (vFlipY)
				stbi_flip_vertically_on_write(1);
			int resWrite = stbi_write_bmp(vFilePathName.c_str(),
				newWidth,
				newHeight,
				bytesPerPixel,
				resizedData);

			if (resWrite)
				res = true;
		}

		SAFE_DELETE_ARRAY(resizedData);
	}
	else
	{
		if (vFlipY)
			stbi_flip_vertically_on_write(1);
		int resWrite = stbi_write_bmp(vFilePathName.c_str(),
			width,
			height,
			bytesPerPixel,
			bmBytesRGB);

		if (resWrite)
			res = true;
	}

	SAFE_DELETE_ARRAY(bmBytesRGB);

	return res;
}
*/

/*
bool TextureCube::SaveToJpg(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const int& vQualityFrom0To100, const ct::uvec2& vNewSize)
{
	ZoneScoped;

	bool res = false;

	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t bufSize = 0;
	uint32_t bytesPerPixel = 3;
	uint8_t* bmBytesRGB = 0;// GetRGBBytesFromFrameBuffer(&width, &height, &bufSize, vAttachmentId);

	int ss = vSubSamplesCount;

	// Sub Sampling
	if (ss > 0)
	{
		uint8_t* tempdata = new uint8_t[bufSize];
		memcpy(tempdata, bmBytesRGB, bufSize);

		unsigned indexMaxSize = width * height;
		for (unsigned int index = 0; index < indexMaxSize; ++index)
		{
			int count = 0;
			int r = 0, g = 0, b = 0;

			for (int i = -ss; i <= ss; i += ss)
			{
				for (int j = -ss; j <= ss; j += ss)
				{
					int ssIndex = index + width * j + i;
					if (ssIndex > 0 && (unsigned int)ssIndex < indexMaxSize)
					{
						r += tempdata[ssIndex * bytesPerPixel + 0];
						g += tempdata[ssIndex * bytesPerPixel + 1];
						b += tempdata[ssIndex * bytesPerPixel + 2];

						++count;
					}
				}
			}

			bmBytesRGB[index * bytesPerPixel + 0] = (uint8_t)((float)r / (float)count);
			bmBytesRGB[index * bytesPerPixel + 1] = (uint8_t)((float)g / (float)count);
			bmBytesRGB[index * bytesPerPixel + 2] = (uint8_t)((float)b / (float)count);
		}

		SAFE_DELETE_ARRAY(tempdata);
	}

	// resize
	if (vNewSize.x != width || vNewSize.y != height)
	{
		// resize
		uint32_t newWidth = vNewSize.x;
		uint32_t newHeight = vNewSize.y;
		uint32_t newBufSize = newWidth * newHeight * bytesPerPixel;
		uint8_t* resizedData = new uint8_t[newBufSize];

		int resizeRes = stbir_resize_uint8(bmBytesRGB, width, height, width * bytesPerPixel,
			resizedData, newWidth, newHeight, newWidth * bytesPerPixel,
			bytesPerPixel);

		if (resizeRes)
		{
			if (vFlipY)
				stbi_flip_vertically_on_write(1);
			int resWrite = stbi_write_jpg(vFilePathName.c_str(),
				newWidth,
				newHeight,
				bytesPerPixel,
				resizedData,
				vQualityFrom0To100);

			if (resWrite)
				res = true;
		}

		SAFE_DELETE_ARRAY(resizedData);
	}
	else
	{
		if (vFlipY)
			stbi_flip_vertically_on_write(1);
		int resWrite = stbi_write_jpg(vFilePathName.c_str(),
			width,
			height,
			bytesPerPixel,
			bmBytesRGB,
			vQualityFrom0To100);

		if (resWrite)
			res = true;
	}

	SAFE_DELETE_ARRAY(bmBytesRGB);

	return res;
}
*/

/*
bool TextureCube::SaveToHdr(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::uvec2& vNewSize)
{
	UNUSED(vNewSize);
	UNUSED(vSubSamplesCount);
	UNUSED(vFlipY);
	UNUSED(vFilePathName);

	return res;
}
*/
/*

bool TextureCube::SaveToTga(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::uvec2& vNewSize)
{
	ZoneScoped;

	bool res = false;

	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t bufSize = 0;
	uint8_t* bmBytesRGBA = 0;// GetRGBABytesFromFrameBuffer(&width, &height, &bufSize, vAttachmentId);

	uint32_t bytesPerPixel = 4;

	int ss = vSubSamplesCount;

	// Sub Sampling
	if (ss > 0)
	{
		uint8_t* tempdata = new uint8_t[bufSize];
		memcpy(tempdata, bmBytesRGBA, bufSize);

		unsigned indexMaxSize = width * height;
		for (unsigned int index = 0; index < indexMaxSize; ++index)
		{
			int count = 0;
			int r = 0, g = 0, b = 0, a = 0;

			for (int i = -ss; i <= ss; i += ss)
			{
				for (int j = -ss; j <= ss; j += ss)
				{
					int ssIndex = index + width * j + i;
					if (ssIndex > 0 && (unsigned int)ssIndex < indexMaxSize)
					{
						r += tempdata[ssIndex * bytesPerPixel + 0];
						g += tempdata[ssIndex * bytesPerPixel + 1];
						b += tempdata[ssIndex * bytesPerPixel + 2];
						a += tempdata[ssIndex * bytesPerPixel + 3];

						++count;
					}
				}
			}

			bmBytesRGBA[index * bytesPerPixel + 0] = (uint8_t)((float)r / (float)count);
			bmBytesRGBA[index * bytesPerPixel + 1] = (uint8_t)((float)g / (float)count);
			bmBytesRGBA[index * bytesPerPixel + 2] = (uint8_t)((float)b / (float)count);
			bmBytesRGBA[index * bytesPerPixel + 3] = (uint8_t)((float)a / (float)count);
		}

		SAFE_DELETE_ARRAY(tempdata);
	}

	// resize
	if (vNewSize.x != width || vNewSize.y != height)
	{
		// resize
		uint32_t newWidth = vNewSize.x;
		uint32_t newHeight = vNewSize.y;
		uint32_t newBufSize = newWidth * newHeight * bytesPerPixel;
		uint8_t* resizedData = new uint8_t[newBufSize];

		int resizeRes = stbir_resize_uint8(bmBytesRGBA, width, height, width * bytesPerPixel,
			resizedData, newWidth, newHeight, newWidth * bytesPerPixel,
			bytesPerPixel);

		if (resizeRes)
		{
			if (vFlipY)
				stbi_flip_vertically_on_write(1);
			int resWrite = stbi_write_tga(vFilePathName.c_str(),
				newWidth,
				newHeight,
				bytesPerPixel,
				resizedData);

			if (resWrite)
				res = true;
		}

		SAFE_DELETE_ARRAY(resizedData);
	}
	else
	{
		if (vFlipY)
			stbi_flip_vertically_on_write(1);
		int resWrite = stbi_write_tga(vFilePathName.c_str(),
			width,
			height,
			bytesPerPixel,
			bmBytesRGBA);

		if (resWrite)
			res = true;
	}

	SAFE_DELETE_ARRAY(bmBytesRGBA);

	return res;
}
*/