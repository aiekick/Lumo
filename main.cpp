/*
MIT License

Copyright (c) 2022-2023 Stephane Cuillerdier (aka aiekick)

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

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif  // PROFILER_INCLUDE
#ifndef ZoneScoped
#define ZoneScoped
#endif  // ZoneScoped

#include <cstdlib>

#ifdef PROFILER_INCLUDE
#ifdef TRACY_ENABLE
void* operator new(size_t count) {
    auto ptr = std::malloc(count);
    TracyAlloc(ptr, count);
    return ptr;
}
void* operator new[](size_t count) {
    auto ptr = std::malloc(count);
    TracyAlloc(ptr, count);
    return ptr;
}
void operator delete(void* ptr) noexcept {
    TracyFree(ptr);
    std::free(ptr);
}
void operator delete[](void* ptr) noexcept {
    TracyFree(ptr);
    std::free(ptr);
}
#endif  // TRACY_ENABLE
#endif  // PROFILER_INCLUDE

#include "src/App.h"
#include <ctools/Logger.h>
#include <string>
#include <iostream>

int main(int argc, char** argv) {
#ifdef _MSC_VER
#ifdef _DEBUG
    // active memory leak detector
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

    ZoneScoped;

    try {
        App app;
        app.run(argc, argv);
    } catch (const std::exception& e) {
        LogVarLightInfo("Exception %s", e.what());
        Logger::Instance()->Close();
        CTOOL_DEBUG_BREAK;
        return EXIT_FAILURE;
    }

    Logger::Instance()->Close();
    return EXIT_SUCCESS;
}