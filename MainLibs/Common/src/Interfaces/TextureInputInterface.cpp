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

#include "TextureInputInterface.h"

#include <memory>
#include <ctools/cTools.h>
#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/BaseNode.h>
#include <Graph/Base/NodeSlot.h>
#include <vkFramework/VulkanCore.h>
#include <Interfaces/TextureOutputInterface.h>

void TextureInputFunctions::UpdateInputDescriptorImageInfos(const std::map<uint32_t, NodeSlotPtr>& vInputs)
{
	for (const auto& input : vInputs) {
		if (input.second) {
			for (auto slot : input.second->linkedSlots) {
				auto otherSLotPtr = slot.getValidShared();
				if (otherSLotPtr) {
					auto otherParentPtr = otherSLotPtr->parentNode.getValidShared();
					if (otherParentPtr) {
						auto otherNodePtr = dynamic_pointer_cast<TextureOutputInterface>(otherParentPtr);
						if (otherNodePtr) {
							SetTexture(input.second->descriptorBinding, 
								otherNodePtr->GetDescriptorImageInfo(
									otherSLotPtr->descriptorBinding)); 
						}}}}}}
}
