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

#include <array>
#include <vector>
#include <ctools/cTools.h>

class SceneAudiArt;
typedef std::shared_ptr<SceneAudiArt> SceneAudiArtPtr;
typedef std::weak_ptr<SceneAudiArt> SceneAudiArtWeak;

// NotifyEvent : need to update the audio buffer
#define SceneAudiArtUpdateDone "SceneAudiArtUpdateDone"

class SceneAudiArt {
public:
    static SceneAudiArtPtr Create();

private:
    static constexpr size_t scFftSize = 512U;
    static constexpr size_t scFullFftSize = scFftSize * 2U;
    static constexpr size_t scMaxAverageSize = 50U;

private:
    SceneAudiArtWeak m_This;
    std::array<double, scFullFftSize> m_AudioBuffer = {};

public:
    SceneAudiArt();
    ~SceneAudiArt();
    void Clear();
    bool IsOk() const;
};
