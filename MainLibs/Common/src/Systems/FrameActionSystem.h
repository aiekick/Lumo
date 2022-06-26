/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <list>
#include <functional>

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