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

#include <vector>
#include <ctools/cTools.h>

class SceneAudiArt;
typedef std::shared_ptr<SceneAudiArt> SceneAudiArtPtr;
typedef ct::cWeak<SceneAudiArt> SceneAudiArtWeak;

// NotifyEvent : need to update the accel structure
#define SceneAudiArtUpdateDone "SceneAudiArtUpdateDone"

class SceneAudiArt
{
public:
	static SceneAudiArtPtr Create();

private:
	SceneAudiArtWeak m_This;

public:
	SceneAudiArt();
	~SceneAudiArt();
	void Clear();
	bool IsOk() const;
};
