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

#include <ImGuiPack.h>
#include <ctools/Logger.h>

// messaging
#include <Res/sdfmToolbarFont.cpp>
#define MESSAGING_CODE_INFOS 0
#define MESSAGING_LABEL_INFOS ICON_SDFMT_INFORMATION
#define MESSAGING_CODE_WARNINGS 1
#define MESSAGING_LABEL_WARNINGS ICON_SDFMT_BELL_ALERT
#define MESSAGING_CODE_ERRORS 2
#define MESSAGING_LABEL_ERRORS ICON_SDFMT_CLOSE_CIRCLE
#define MESSAGING_LABEL_VKLAYER ICON_SDFMT_LAYERS
#define MESSAGING_LABEL_DEBUG ICON_SDFMT_BUG

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

int App::run(int /*argc*/, char** argv) {
    ZoneScoped;

    printf("-----------\n");
    printf("[[ Lumo Beta %s ]]\n", Lumo_BuildId);

    FileHelper::Instance()->SetAppPath(argv[0]);
    FileHelper::Instance()->SetCurDirectory(FileHelper::Instance()->GetAppPath());

#ifdef _DEBUG
    FileHelper::Instance()->CreateDirectoryIfNotExist("debug");
    FileHelper::Instance()->CreateDirectoryIfNotExist("debug/shaders");
#endif

    FileHelper::Instance()->CreateDirectoryIfNotExist("shaders");

    m_InitMessaging();

    MainBackend::Instance()->run();

    return 0;
}

void App::m_InitMessaging() {
    Messaging::Instance()->AddCategory(MESSAGING_CODE_INFOS, "Infos(s)", MESSAGING_LABEL_INFOS, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
    Messaging::Instance()->AddCategory(MESSAGING_CODE_WARNINGS, "Warnings(s)", MESSAGING_LABEL_WARNINGS, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
    Messaging::Instance()->AddCategory(MESSAGING_CODE_ERRORS, "Errors(s)", MESSAGING_LABEL_ERRORS, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
    Messaging::Instance()->AddCategory(MESSAGING_TYPE_VKLAYER, "Vk Layer(s)", MESSAGING_LABEL_VKLAYER, ImVec4(0.8f, 0.0f, 0.4f, 1.0f));
    Messaging::Instance()->AddCategory(MESSAGING_TYPE_DEBUG, "Debug(s)", MESSAGING_LABEL_DEBUG, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));

    Messaging::Instance()->SetLayoutManager(LayoutManager::Instance());
    Logger::sStandardLogFunction = [](const int& vType, const std::string& vMessage) {
        MessageData msg_datas;
        const auto& type = vType;
        Messaging::Instance()->AddMessage(vMessage, type, false, msg_datas, {});
    };
}