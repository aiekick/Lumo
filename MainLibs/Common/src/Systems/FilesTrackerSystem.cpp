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

#include "FilesTrackerSystem.h"
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
		LogVarDebug("Debug : DIR (%s) FILE (%s) has been Modified", vDir.c_str(), vFileName.c_str());
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