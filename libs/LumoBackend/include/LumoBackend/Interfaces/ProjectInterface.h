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

#include <LumoBackend/Headers/LumoBackendDefs.h>
#include <ctools/cTools.h>
#include <cstdint>
#include <array>

class LUMO_BACKEND_API ProjectInterface {
public:
    // since a user datas of type void* is used
    // we cant do a dynamic_cast, so for test if we have a ProjectInterface
    // we define this magic number, becasue this member will not caus compiling issue
    // and if this is not a ProjectInterface, magic_number will point to a random memory zone
    // so will not have this value
    int64_t magic_id = ct::EncodeId("IProject");

public:
    virtual bool IsProjectLoaded() const = 0;
    virtual bool IsProjectNeverSaved() const = 0;
    virtual bool IsThereAnyProjectChanges() const = 0;
    virtual void SetProjectChange(bool vChange = true) = 0;
    virtual bool WasJustSaved() = 0;
};
