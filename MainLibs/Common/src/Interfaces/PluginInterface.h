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

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <Graph/Graph.h>
#include <Graph/Library/LibraryCategory.h>

class PluginInterface;
typedef ct::cWeak<PluginInterface> PluginInterfaceWeak;
typedef std::shared_ptr<PluginInterface> PluginInterfacePtr;

class FileHelper;
class VulkanShader;
class CommonSystem;
struct ImGuiContext;
namespace vkApi { class VulkanCore; }
namespace ImGui { class CustomStyle; }
class PluginInterface
{
public:
	virtual ~PluginInterface() = default;

	virtual bool Init(
		vkApi::VulkanCore* vVkCore, 
		FileHelper* vFileHelper, 
		CommonSystem* vCommonSystem,
		ImGuiContext* vContext,
		ImGui::CustomStyle* vCustomStyle) = 0;
	virtual void Unit() = 0;
	virtual uint32_t GetVersionMajor() const = 0;
	virtual uint32_t GetVersionMinor() const = 0;
	virtual uint32_t GetVersionBuild() const = 0;
	virtual std::string GetName() const = 0;
	virtual std::string GetVersion() const = 0;
	virtual std::string GetDescription() const = 0;
	virtual std::vector<std::string> GetNodes() const = 0;
	virtual std::vector<LibraryEntry> GetLibrary() const = 0;
	virtual BaseNodePtr CreatePluginNode(const std::string& vPluginNodeName) = 0; // factory for nodes

	// will reset the ids but will return the id count pre reset
	virtual int ResetImGuiID(const int& vWidgetId) = 0;
};
