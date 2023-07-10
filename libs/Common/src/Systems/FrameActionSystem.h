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

#include <list>
#include <functional>
#include <Common/Globals.h>

/*
 list of sequential actions
 executed time to time only if executed action retrun true
 each succesfull action is erased
 not sucessfull action is applied again each frame
*/

class ProjectFile;
class FrameActionSystem
{
private:
	typedef std::function<bool()> ActionStamp;
	std::list<ActionStamp> puActions;

public:
	// insert an action at first, cause :
	// this action will be executed first at the next frame
	void Insert(ActionStamp vAction);
	// add an action at end
	// this action will be executed at least after all others
	void Add(ActionStamp vAction);
	// clear all actions
	void Clear();
	// apply first action each frame until true is returned
	// if return true, erase action
	// let the next frame call the next action
	// il false, action executed until true
	void RunActions();
};