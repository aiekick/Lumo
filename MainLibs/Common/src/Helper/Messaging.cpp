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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "Messaging.h"
#include <ImWidgets/ImWidgets.h>
#include <FontIcons/CustomFont.h>
#include <FontIcons/CustomFont2.h>
#include <forward_list>

Messaging::Messaging() = default;
Messaging::~Messaging() = default;

///////////////////////////////////////////////////////////////////////////////////////////
///// PRIVATE /////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

static char Messaging_Message_Buffer[2048] = "\0";

void Messaging::AddMessage(MessageTypeEnum vType, bool vSelect, MessageData vDatas, const MessageFunc & vFunction, const char* fmt, va_list args)
{
	const auto size = vsnprintf(Messaging_Message_Buffer, 2047, fmt, args);
	if (size > 0)
		AddMessage(std::string(Messaging_Message_Buffer, size), vType, vSelect, vDatas, vFunction);
}

void Messaging::AddMessage(const std::string & vMsg, MessageTypeEnum vType, bool vSelect, MessageData vDatas, const MessageFunc & vFunction)
{
	if (vSelect)
	{
		currentMsgIdx = (int32_t)puMessages.size();
	}

	puMessages.emplace_back(vMsg, vType, vDatas, vFunction);
}

bool Messaging::DrawMessage(const size_t & vMsgIdx)
{
	auto res = false;

	if (vMsgIdx < puMessages.size())
	{
		const auto pa = puMessages[vMsgIdx];
		res |= DrawMessage(pa);
	}

	return res;
}

bool Messaging::DrawMessage(const Messagekey & vMsg)
{
	if (std::get<1>(vMsg) == MessageTypeEnum::MESSAGE_TYPE_INFOS)
	{
		ImGui::Text("%s ", ICON_NDP_INFO_CIRCLE);
	}
	else if (std::get<1>(vMsg) == MessageTypeEnum::MESSAGE_TYPE_WARNING)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, puWarningColor);
		ImGui::Text("%s ", ICON_NDP_EXCLAMATION_TRIANGLE);
		ImGui::PopStyleColor();
	}
	else if (std::get<1>(vMsg) == MessageTypeEnum::MESSAGE_TYPE_ERROR)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, puErrorColor);
		ImGui::Text("%s ", ICON_NDP_TIMES_CIRCLE);
		ImGui::PopStyleColor();
	}
	//ImGui::SameLine(); // used only for when displayed in list. no effect when diplsayed in status bar
	ImGui::PushID(&vMsg);
	const auto check = ImGui::Selectable_FramedText("%s", std::get<0>(vMsg).c_str());
	ImGui::PopID();
	if (check)
	{
		const auto datas = std::get<2>(vMsg);
		const auto& func = std::get<3>(vMsg);
		if (func)
			func(datas);
	}
	return check;
}

///////////////////////////////////////////////////////////////////////////////////////////
///// PUBLIC //////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void Messaging::Draw()
{
	ImGui::Text("Messages :");

	if (ImGui::MenuItem(ICON_NDP_REFRESH "##Refresh"))
	{
		Messaging::Instance()->Clear();
	}

	if (!puMessages.empty())
	{
		// on type of message only
		if (puMessageExistFlags == MESSAGE_EXIST_INFOS ||
			puMessageExistFlags == MESSAGE_EXIST_WARNING ||
			puMessageExistFlags == MESSAGE_EXIST_ERROR)
		{
			if (ImGui::MenuItem(ICON_NDP_DESTROY "##clear"))
			{
				Clear();
			}
		}
		else
		{
			if (ImGui::BeginMenu(ICON_NDP_DESTROY "##clear"))
			{
				if (ImGui::MenuItem("All")) Clear();
				ImGui::Separator();
				if (puMessageExistFlags & MESSAGE_EXIST_INFOS)
					if (ImGui::MenuItem("Infos")) ClearInfos();
				if (puMessageExistFlags & MESSAGE_EXIST_WARNING)
					if (ImGui::MenuItem("Warnings")) ClearWarnings();
				if (puMessageExistFlags & MESSAGE_EXIST_ERROR)
					if (ImGui::MenuItem("Errors")) ClearErrors();

				ImGui::EndMenu();
			}
		}
	}
	if (!puMessages.empty())
	{
		if (puMessages.size() > 1)
		{
			if (ImGui::MenuItem(ICON_NDP2_CHEVRON_LEFT_BOX "##left"))
			{
				currentMsgIdx = ct::maxi<int32_t>(--currentMsgIdx, 0);
			}
			if (ImGui::MenuItem(ICON_NDP2_CHEVRON_RIGHT_BOX "##right"))
			{
				currentMsgIdx = ct::maxi<int32_t>(++currentMsgIdx, (int32_t)puMessages.size() - 1);
			}
			if (ImGui::BeginMenu(ICON_NDP2_CHEVRON_UP_BOX "##up"))
			{
				for (auto& msg : puMessages)
				{
					if (DrawMessage(msg))
						break;
				}
				ImGui::EndMenu();
			}
		}
		currentMsgIdx = ct::clamp<int32_t>(currentMsgIdx, 0, (int32_t)puMessages.size() - 1);
		DrawMessage(currentMsgIdx);
	}
}

void Messaging::AddInfos(bool vSelect, MessageData vDatas, const MessageFunc & vFunction, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	AddMessage(MessageTypeEnum::MESSAGE_TYPE_INFOS, vSelect, vDatas, vFunction, fmt, args);
	va_end(args);
	puMessageExistFlags = (MessageExistFlags)(puMessageExistFlags | MESSAGE_EXIST_INFOS);
}

void Messaging::AddWarning(bool vSelect, MessageData vDatas, const MessageFunc & vFunction, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	AddMessage(MessageTypeEnum::MESSAGE_TYPE_WARNING, vSelect, vDatas, vFunction, fmt, args);
	va_end(args);
	puMessageExistFlags = (MessageExistFlags)(puMessageExistFlags | MESSAGE_EXIST_WARNING);
}

void Messaging::AddError(bool vSelect, MessageData vDatas, const MessageFunc & vFunction, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	AddMessage(MessageTypeEnum::MESSAGE_TYPE_ERROR, vSelect, vDatas, vFunction, fmt, args);
	va_end(args);
	puMessageExistFlags = (MessageExistFlags)(puMessageExistFlags | MESSAGE_EXIST_ERROR);
}

void Messaging::ClearErrors()
{
	std::forward_list<int> msgToErase;
	auto idx = 0;
	for (auto& msg : puMessages)
	{
		if (std::get<1>(msg) == MessageTypeEnum::MESSAGE_TYPE_ERROR)
			msgToErase.push_front(idx);
		++idx;
	}

	for (auto& id : msgToErase)
	{
		puMessages.erase(puMessages.begin() + id);
	}

	puMessageExistFlags &= ~MESSAGE_EXIST_ERROR;
}

void Messaging::ClearWarnings()
{
	std::forward_list<int> msgToErase;
	auto idx = 0;
	for (auto& msg : puMessages)
	{
		if (std::get<1>(msg) == MessageTypeEnum::MESSAGE_TYPE_WARNING)
			msgToErase.push_front(idx);
		++idx;
	}

	for (auto& id : msgToErase)
	{
		puMessages.erase(puMessages.begin() + id);
	}

	puMessageExistFlags &= ~MESSAGE_EXIST_WARNING;
}

void Messaging::ClearInfos()
{
	std::forward_list<int> msgToErase;
	auto idx = 0;
	for (auto& msg : puMessages)
	{
		if (std::get<1>(msg) == MessageTypeEnum::MESSAGE_TYPE_INFOS)
			msgToErase.push_front(idx);
		++idx;
	}

	for (auto& id : msgToErase)
	{
		puMessages.erase(puMessages.begin() + id);
	}

	puMessageExistFlags &= ~MESSAGE_EXIST_INFOS;
}

void Messaging::Clear()
{
	puMessages.clear();
	puMessageExistFlags = MESSAGE_EXIST_NONE;
}