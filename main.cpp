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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <vkProfiler/Profiler.h>

#include <cstdlib>

#ifdef TRACY_ENABLE
void* operator new(size_t count)
{
	auto ptr = std::malloc(count);
	TracyAlloc(ptr, count);
	return ptr;
}
void* operator new[](size_t count)
{
	auto ptr = std::malloc(count);
	TracyAlloc(ptr, count);
	return ptr;
}
void operator delete(void* ptr) noexcept
{
	TracyFree(ptr);
	std::free(ptr);
}
void operator delete[](void* ptr) noexcept
{
	TracyFree(ptr);
	std::free(ptr);
}
#endif

#include "src/App.h"
#include <ctools/Logger.h>
#include <string>
#include <iostream>
#include <vkFramework/VulkanWindow.h>

int main(int, char** argv)
{
#ifdef _MSC_VER
#ifdef _DEBUG
	// active memory leak detector
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

	ZoneScoped;

#ifdef _DEBUG
	App::Instance()->Run(std::string(argv[0]));
#else
	try
	{
		App::Instance()->Run(std::string(argv[0]));
	}
	catch (const std::exception& e)
	{
		LogVarLightInfo("Exception %s", e.what());
		Logger::Instance()->Close();

		CTOOL_DEBUG_BREAK;

		/*if (App::Instance()->GetWindowPtr())
		{
			const auto& main_window = App::Instance()->GetWindowPtr()->getWindowPtr();
			App::Instance()->Unit(main_window);
		}*/

		return EXIT_FAILURE;
	}
#endif

	return EXIT_SUCCESS;
}