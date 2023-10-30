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

#include <LumoBackend/Systems/FilesTrackerSystem.h>
#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>

FilesTrackerSystem::FilesTrackerSystem()
{
	// Create the file system watcher instance
	// efsw::FileWatcher allow a first boolean parameter that indicates if it should start with the generic file watcher instead of the platform specific backend
	m_FilesTracker = std::make_unique<efsw::FileWatcher>();
}

FilesTrackerSystem::~FilesTrackerSystem()
{
	for (auto wid : m_WatchIDs)
		m_FilesTracker->removeWatch(wid);
	m_FilesTracker.reset();
}

void FilesTrackerSystem::addWatch(std::string& vPath)
{
	// Add a folder to watch, and get the efsw::WatchID
	// It will watch the /tmp folder recursively ( the third parameter indicates that is recursive )
	// Reporting the files and directories changes to the instance of the listener
	m_WatchIDs.emplace(m_FilesTracker->addWatch(vPath, this, false));
}

void FilesTrackerSystem::update()
{
	m_FilesTracker->watch();
}

void FilesTrackerSystem::handleFileAction(efsw::WatchID vWatchid, const std::string& vDir, const std::string& vFileName, efsw::Action vAction, std::string vOldFilename)
{
	UNUSED(vWatchid);

	switch (vAction)
	{
	case efsw::Actions::Modified:
	{
		//LogVarDebugError("Debug : DIR (%s) FILE (%s) has been Modified", vDir.c_str(), vFileName.c_str());
		auto ps = FileHelper::Instance()->ParsePathFileName(vDir + vFileName);
		if (ps.isOk)
		{
			files.emplace(ps.GetFPNE());
			Changes = true;
		}
		break;
	}
	case efsw::Actions::Add:
	case efsw::Actions::Delete:
	case efsw::Actions::Moved:
	default:
		break;
	}
}