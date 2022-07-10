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

#include "TextureGroupInputInterface.h"

#include <memory>
#include <ctools/cTools.h>
#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/BaseNode.h>
#include <Graph/Base/NodeSlot.h>
#include <vkFramework/VulkanCore.h>
#include <Interfaces/TextureGroupOutputInterface.h>

void TextureGroupInputFunctions::UpdateTextureGroupInputDescriptorImageInfos(const std::map<uint32_t, NodeSlotPtr>& vInputs)
{
	for (const auto& input : vInputs) {
		if (input.second && input.second->slotType == NodeSlotTypeEnum::TEXTURE_2D_GROUP) {
			for (auto slot : input.second->linkedSlots) {
				auto otherSLotPtr = slot.getValidShared();
				if (otherSLotPtr) {
					if (otherSLotPtr->slotType == NodeSlotTypeEnum::TEXTURE_2D_GROUP) {
						auto otherParentPtr = otherSLotPtr->parentNode.getValidShared();
						if (otherParentPtr) {
							auto otherNodePtr = dynamic_pointer_cast<TextureGroupOutputInterface>(otherParentPtr);
							if (otherNodePtr) {
								SetTextures(input.second->descriptorBinding,
									otherNodePtr->GetDescriptorImageInfos(
										otherSLotPtr->descriptorBinding));
							}
						}
					}
				}
			}
		}
	}
}