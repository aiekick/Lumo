/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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

#include <string>
#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

// NotifyEvent : need to update the model
#define ModelUpdateDone "ModelUpdateDone"
// NotifyEvent : need to update the texture
#define TextureUpdateDone "TextureUpdateDone"
// NotifyEvent : need to update the texel buffer
#define TexelBufferUpdateDone "TexelBufferUpdateDone"
// NotifyEvent : need to update the texel buffer group
#define TexelBufferGroupUpdateDone "TexelBufferGroupUpdateDone"
// NotifyEvent : need to update the texture group
#define TextureGroupUpdateDone "TextureGroupUpdateDone"
// NotifyEvent : need to update the light
#define LightGroupUpdateDone "LightGroupUpdateDone"
// NotifyEvent : need to update the variable
#define VariableUpdateDone "VariableUpdateDone"
// NotifyEvent : need to update the storage buffer
#define StorageBufferUpdateDone "StorageBufferUpdateDone"
// NotifyEvent : the node link is breked
#define NodeLinkIsBreaked "NodeLinkIsBreaked"
// NotifyEvent : some task was updated
#define SomeTasksWasUpdated "SomeTasksWasUpdated"
// NotifyEvent : graph loaded (so after all is finalized)
#define GraphIsLoaded "GraphIsLoaded"
// NotifyEvent : a new frame is available
#define NewFrameAvailable "NewFrameAvailable"
// NotifyEvent : count of notification message
#define CountEvents "CountEvents"
// NotifyEvent : need to update the shader pass
#define ShaderPassUpdateDone "ShaderPassUpdateDone"
// NotifyEvent : need to update the code
#define CodeUpdateDone "CodeUpdateDone"

typedef std::string NotifyEvent;

class LUMO_BACKEND_API NotifyInterface {
public:
    virtual void Notify(NotifyEvent vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot = NodeSlotWeak()) = 0;
};
