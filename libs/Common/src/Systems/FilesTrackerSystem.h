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

#include <efsw/efsw.hpp>
#include <set>
#include <list>
#include <string>
#include <memory>

class FilesTrackerSystem : public efsw::FileWatchListener
{
public:
	bool Changes = false;
	std::set<std::string> files;

private:
	std::unique_ptr<efsw::FileWatcher> m_FilesTracker = nullptr;
	std::set<efsw::WatchID> m_WatchIDs;

public:
	static FilesTrackerSystem* Instance()
	{
		static FilesTrackerSystem _instance;
		return &_instance;
	}

protected:
	void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename = "") override;

	FilesTrackerSystem(); // Prevent construction
	FilesTrackerSystem(const FilesTrackerSystem&) = default; // Prevent construction by copying
	FilesTrackerSystem& operator =(const FilesTrackerSystem&) { return *this; }; // Prevent assignment
	~FilesTrackerSystem(); // Prevent unwanted destruction

public:
	void addWatch(std::string& vPath);
	void update();
};
