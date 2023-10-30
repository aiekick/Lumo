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
#pragma warning(disable : 4251)

#include <ctools/cTools.h>

class LUMO_BACKEND_API ResizerInterface {
protected:
	bool m_ResizingByResizeEventIsAllowed = false;
	bool m_ResizingByHandIsAllowed = false;

public:
	virtual void NeedResizeByHand(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers = nullptr) { UNUSED(vNewSize); UNUSED(vCountColorBuffers); }
	virtual void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers = nullptr) = 0;

	void AllowResizeOnResizeEvents(const bool& vResizing) { m_ResizingByResizeEventIsAllowed = vResizing; }
	void AllowResizeByHandOrByInputs(const bool& vResizing) { m_ResizingByHandIsAllowed = vResizing; }

	bool IsResizeableByHand() const { return m_ResizingByHandIsAllowed; }
	bool IsResizeableByResizeEvent() const { return m_ResizingByResizeEventIsAllowed; }
};