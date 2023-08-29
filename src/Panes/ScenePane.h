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

#include <LumoBackend/Headers/LumoBackendDefs.h>
#include <LumoBackend/Graph/Graph.h>
#include <ImGuiPack.h>
#include <AbstractPane.h>
#include <stdint.h>
#include <string>
#include <map>

class ProjectFile;
class ScenePane : public AbstractPane
{
public:
	bool Init() override;
	void Unit() override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawPanes(const uint32_t& vCurrentFrame, PaneFlags& vInOutPaneShown, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;

public: // singleton
	static std::shared_ptr<ScenePane> Instance()
	{
		static auto _instance = std::make_shared<ScenePane>();
		return _instance;
	}

public:
	ScenePane(); // Prevent construction
	ScenePane(const ScenePane&) = default; // Prevent construction by copying
	ScenePane& operator =(const ScenePane&) { return *this; }; // Prevent assignment
	virtual ~ScenePane(); // Prevent unwanted destruction};
};
