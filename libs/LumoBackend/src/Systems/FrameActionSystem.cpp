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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <LumoBackend/Systems/FrameActionSystem.h>

void FrameActionSystem::Insert(ActionStamp vAction) {
    if (vAction)
        puActions.push_front(vAction);
}

void FrameActionSystem::Add(ActionStamp vAction) {
    if (vAction)
        puActions.push_back(vAction);
}

void FrameActionSystem::Clear() {
    puActions.clear();
}

void FrameActionSystem::RunActions() {
    if (!puActions.empty()) {
        const auto action = *puActions.begin();
        if (action())  // one action per frame, it true we can continue by deleting the current
        {
            if (!puActions.empty())  // because an action can clear actions
            {
                puActions.pop_front();
            }
        }
    }
}