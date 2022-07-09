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

#include <Graph/Base/NodeSlot.h>

enum class NotifyEvent
{
	// need to update the model
	ModelUpdateDone = 0,
	// need to update the texture
	TextureUpdateDone,
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
	// count of notification message
	CountEvents
};

class NotifyInterface
{
public:
	virtual void Notify(
		const NotifyEvent& vEvent, 
		const NodeSlotWeak& vEmmiterSlot, 
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak()) = 0;
};
	