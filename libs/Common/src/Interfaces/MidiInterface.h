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

#include <Common/Globals.h>

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

struct COMMON_API MidiMessage
{
	std::string name;
	std::vector<uint8_t> bytes;
};

struct COMMON_API MidiStruct
{
	std::string deviceName;
	MidiMessage lastMessage;
	MidiMessage currentMessage;
};

class COMMON_API MidiInterface
{
protected:
	std::vector<MidiStruct> m_MidiDevices;

public:
	bool updateMidiNeeded = false;

public:
	uint32_t GetCountDevices()
	{
		return (uint32_t)m_MidiDevices.size();
	}
	virtual std::vector<MidiMessage> GetMessages() = 0;
	virtual MidiMessage GetMessage(const uint32_t& vPort) = 0;
	virtual MidiStruct GetMessageDB(const uint32_t& vPort) = 0;
	virtual std::string GetDeviceName(const uint32_t& vPort) = 0;
};