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
#include <memory>
#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

class BaseNode;
typedef std::weak_ptr<BaseNode> BaseNodeWeak;
typedef std::shared_ptr<BaseNode> BaseNodePtr;

class NodeSlot;
typedef std::weak_ptr<NodeSlot> NodeSlotWeak;
typedef std::shared_ptr<NodeSlot> NodeSlotPtr;

class NodeLink;
typedef std::weak_ptr<NodeLink> NodeLinkWeak;
typedef std::shared_ptr<NodeLink> NodeLinkPtr;

class NodeSlotInput;
typedef std::weak_ptr<NodeSlotInput> NodeSlotInputWeak;
typedef std::shared_ptr<NodeSlotInput> NodeSlotInputPtr;

class NodeSlotOutput;
typedef std::weak_ptr<NodeSlotOutput> NodeSlotOutputWeak;
typedef std::shared_ptr<NodeSlotOutput> NodeSlotOutputPtr;
