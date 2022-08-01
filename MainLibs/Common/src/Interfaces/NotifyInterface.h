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

#include <Graph/Graph.h>

enum class NotifyEvent
{
	// need to update the model
	ModelUpdateDone = 0,
	// need to update the texture
	TextureUpdateDone,
	// need to update the texel buffer
	TexelBufferUpdateDone,
	// need to update the texel buffer group
	TexelBufferGroupUpdateDone,
	// need to update the texture group
	TextureGroupUpdateDone,
	// need to update the light
	LightGroupUpdateDone,
	// need to update the variable
	VariableUpdateDone,
	// the node link is breked
	NodeLinkIsBreaked,
	// some task was updated
	SomeTasksWasUpdated,
	// graph loaded (so after all is finalized)
	GraphIsLoaded,
	// a new frame is available
	NewFrameAvailable,
	// a accel structure (tofix : for use in pluginand defin our own notify)
	AccelStructureUpdateDone,
	// count of notification message
	CountEvents
};

class NotifyInterface
{
public:
	virtual void Notify(
		const NotifyEvent& vEvent, 
		const NodeSlotWeak& vEmitterSlot, 
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak()) = 0;

/*public:
	// ModelUpdateDone
	virtual void UpdateModel(
		const BaseNodeWeak& vBaseNode,
		const NodeSlotWeak& vEmitterSlot,
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());
	// TextureUpdateDone
	virtual void UpdateTexture(
		const BaseNodeWeak& vBaseNode,
		const NodeSlotWeak& vEmitterSlot,
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());
	// TexelBufferUpdateDone
	virtual void UpdateTexelBuffer(
		const BaseNodeWeak& vBaseNode,
		const NodeSlotWeak& vEmitterSlot,
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());
	// TexelBufferGroupUpdateDone
	virtual void UpdateTexelBufferGroup(
		const BaseNodeWeak& vBaseNode,
		const NodeSlotWeak& vEmitterSlot,
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());
	// TextureGroupUpdateDone
	virtual void UpdateTextureGroup(
		const BaseNodeWeak& vBaseNode,
		const NodeSlotWeak& vEmitterSlot,
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());
	// LightGroupUpdateDone
	virtual void UpdateLightGroup(
		const BaseNodeWeak& vBaseNode,
		const NodeSlotWeak& vEmitterSlot,
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());
	// VariableUpdateDone
	virtual void UpdateVariable(
		const BaseNodeWeak& vBaseNode,
		const NodeSlotWeak& vEmitterSlot,
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());
	// AccelStructureUpdateDone
	virtual void UpdateAccelStructure(
		const BaseNodeWeak& vBaseNode,
		const NodeSlotWeak& vEmitterSlot,
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());*/
};
	