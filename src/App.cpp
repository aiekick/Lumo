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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "App.h"

#include <Headers/LumoBuild.h>
#include <ctools/FileHelper.h>
#include <Backend/MainBackend.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

int App::run(int /*argc*/, char** argv) {
    ZoneScoped;

    printf("-----------\n");
    printf("[[ SdfMesher Beta %s ]]\n", Lumo_BuildId);

    FileHelper::Instance()->SetAppPath(argv[0]);
    FileHelper::Instance()->SetCurDirectory(FileHelper::Instance()->GetAppPath());

#ifdef _DEBUG
    FileHelper::Instance()->CreateDirectoryIfNotExist("debug");
#endif

    FileHelper::Instance()->CreateDirectoryIfNotExist("shaders");

    MainBackend::Instance()->run();

    return 0;
}
