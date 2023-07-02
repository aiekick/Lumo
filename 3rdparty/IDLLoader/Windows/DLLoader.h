#pragma once

#ifdef WIN32
#include <iostream>
#include <ctools/Logger.h>
#include <IDLLoader/IDLLoader.h>
#include "Windows.h"

namespace dlloader
{
	template <class T>
	class DLLoader : public IDLLoader<T>
	{

	private:
		HMODULE			_handle = nullptr;
		std::string		_pathToLib;
		std::string		_allocClassSymbol;
		std::string		_deleteClassSymbol;

	public:
		DLLoader() = default;
		DLLoader(std::string const &pathToLib,
			std::string const &allocClassSymbol = "allocator",
			std::string const &deleteClassSymbol = "deleter") :
			_handle(nullptr), _pathToLib(pathToLib),
			_allocClassSymbol(allocClassSymbol), _deleteClassSymbol(deleteClassSymbol)
		{}

		~DLLoader() = default;

		bool IsValid() const
		{
			return _handle != nullptr;
		}

		void DLOpenLib() override
		{
			if (!(_handle = LoadLibrary(_pathToLib.c_str()))) {
				LogVarDebug("Can't open and load %s", _pathToLib.c_str());
			}
		}

		std::shared_ptr<T> DLGetInstance() override
		{
			using allocClass = T * (*)();
			using deleteClass = void(*)(T *);

			auto allocFunc = reinterpret_cast<allocClass>(
				GetProcAddress(_handle, _allocClassSymbol.c_str()));
			auto deleteFunc = reinterpret_cast<deleteClass>(
				GetProcAddress(_handle, _deleteClassSymbol.c_str()));

			if (!allocFunc || !deleteFunc) {
				DLCloseLib();
				//LogVarDebug("Can't find allocator or deleter symbol in %s", _pathToLib.c_str());
				return nullptr;
			}

			return std::shared_ptr<T>(allocFunc(), [deleteFunc](T* p) 
			{
				deleteFunc(p);
			});
		}

		void DLCloseLib() override
		{
			if (_handle && FreeLibrary(_handle) == 0) {
				LogVarDebug("Can't close %s", _pathToLib.c_str());
			}
		}

	};

}
#endif