/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

#pragma once

#include <SceneGraph/SceneAudiart.h>

#include <map>
#include <string>

class SceneAudiartInputInterface {
protected:
    std::map<std::string, SceneAudiartWeak> m_SceneAudiarts;

public:
    virtual void SetSceneAudiart(const std::string& vName, SceneAudiartWeak vSceneAudiart) = 0;
};
