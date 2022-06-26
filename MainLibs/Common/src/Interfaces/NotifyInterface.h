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

#include <Graph/Base/NodeSlot.h>

enum class NotifyEvent
{
	// need to update the model
	ModelUpdateDone = 0,
	// need to update the texture
	TextureUpdateDone,
	// need to update the light
	LightUpdateDone,
	// the node link is breked
	NodeLinkIsBreaked,
	// some task was updated
	SomeTasksWasUpdated,
	// graph loaded (so after all is finalized)
	GraphIsLoaded,
	// a new frame is available
	NewFrameAvailable,
	// count of notification message
	CountEvents
};

class NotifyInterface
{
public:
	virtual void Notify(
		const NotifyEvent& vEvent, 
		const NodeSlotWeak& vEmmiterSlot, 
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak()) = 0;
};
	