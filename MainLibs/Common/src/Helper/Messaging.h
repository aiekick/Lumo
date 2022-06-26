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

#include <ctools/cTools.h>

#include <functional>
#include <cstdarg>
#include <string>
#include <utility> // std::pair
#include <vector>
#include <memory>

class MessageData
{
private:
	std::shared_ptr<void> puDatas;

public:
	MessageData() = default;
	MessageData(std::nullptr_t) {}
	template<typename T>
	MessageData(const std::shared_ptr<T>& vDatas)
	{
		SetUserDatas(vDatas);
	}
	template<typename T>
	void SetUserDatas(const std::shared_ptr<T>& vDatas)
	{
		puDatas = vDatas;
	}
	template<typename T>
	std::shared_ptr<T> GetUserDatas()
	{
		return std::static_pointer_cast<T>(puDatas);
	}
};

class ProjectFile;
class Messaging
{
private:
	const ImVec4 puErrorColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	const ImVec4 puWarningColor = ImVec4(0.8f, 0.8f, 0.0f, 1.0f);

private:
	enum MessageTypeEnum
	{
		MESSAGE_TYPE_INFOS = 0,
		MESSAGE_TYPE_ERROR,
		MESSAGE_TYPE_WARNING
	};

	enum _MessageExistFlags
	{
		MESSAGE_EXIST_NONE = 0,
		MESSAGE_EXIST_INFOS = (1 << 0),
		MESSAGE_EXIST_ERROR = (1 << 1),
		MESSAGE_EXIST_WARNING = (1 << 2)
	};
	typedef int MessageExistFlags;
	MessageExistFlags puMessageExistFlags = MESSAGE_EXIST_NONE;

	int32_t currentMsgIdx = 0;
	typedef std::function<void(MessageData)> MessageFunc;
	typedef std::tuple<std::string, MessageTypeEnum, MessageData, MessageFunc> Messagekey;
	std::vector<Messagekey> puMessages;

private:
	void AddMessage(const std::string& vMsg, MessageTypeEnum vType, bool vSelect, MessageData vDatas, const MessageFunc& vFunction);
	void AddMessage(MessageTypeEnum vType, bool vSelect, MessageData vDatas, const MessageFunc& vFunction, const char* fmt, va_list args);
	bool DrawMessage(const size_t& vMsgIdx);
	bool DrawMessage(const Messagekey& vMsg);

public:
	void Draw();
	void AddInfos(bool vSelect, MessageData vDatas, const MessageFunc& vFunction, const char* fmt, ...); // select => set currentMsgIdx to this msg idx
	void AddWarning(bool vSelect, MessageData vDatas, const MessageFunc& vFunction, const char* fmt, ...); // select => set currentMsgIdx to this msg idx
	void AddError(bool vSelect, MessageData vDatas, const MessageFunc& vFunction, const char* fmt, ...); // select => set currentMsgIdx to this msg idx
	void ClearErrors();
	void ClearWarnings();
	void ClearInfos();
	void Clear();

public: // singleton
	static Messaging* Instance()
	{
		static Messaging _instance;
		return &_instance;
	}

protected:
	Messaging(); // Prevent construction
	Messaging(const Messaging&) = default;; // Prevent construction by copying
	Messaging& operator =(const Messaging&) { return *this; }; // Prevent assignment
	~Messaging(); // Prevent unwanted destruction
};
