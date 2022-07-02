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

#include <ctools/cTools.h>

class ResizerInterface
{
public:
	ct::fvec2 m_fOutputSize;

public:
	virtual void NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer) = 0; 
	virtual ct::fvec2 GetOutputSize() { return m_fOutputSize; }
};