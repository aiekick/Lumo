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

#include <string>
#include <memory>
#include <ctools/cTools.h>
#include <ctools/Logger.h>

class BaseNode;
typedef ct::cWeak<BaseNode> BaseNodeWeak;
typedef std::shared_ptr<BaseNode> BaseNodePtr;

class NodeSlot;
typedef ct::cWeak<NodeSlot> NodeSlotWeak;
typedef std::shared_ptr<NodeSlot> NodeSlotPtr;

class NodeSlotInput;
typedef ct::cWeak<NodeSlotInput> NodeSlotInputWeak;
typedef std::shared_ptr<NodeSlotInput> NodeSlotInputPtr;

class NodeSlotOutput;
typedef ct::cWeak<NodeSlotOutput> NodeSlotOutputWeak;
typedef std::shared_ptr<NodeSlotOutput> NodeSlotOutputPtr;