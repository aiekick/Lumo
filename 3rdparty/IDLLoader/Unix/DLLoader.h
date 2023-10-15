#pragma once

#ifdef UNIX
#include <iostream>
#include <ctools/Logger.h>
#include <dlfcn.h>
#include <IDLLoader/IDLLoader.h>

namespace dlloader {
template <class T>
class DLLoader : public IDLLoader<T> {
private:
	void *_handle;
	std::string _pathToLib;
	std::string _allocClassSymbol;
	std::string _deleteClassSymbol;
	bool _isAPlugin = true;

public:
	DLLoader() = default;
	DLLoader(std::string const &pathToLib,
		std::string const &allocClassSymbol = "allocator",
		std::string const &deleteClassSymbol = "deleter")
		: _handle(nullptr),
		  _pathToLib(pathToLib),
		  _allocClassSymbol(allocClassSymbol),
		  _deleteClassSymbol(deleteClassSymbol) {}

	~DLLoader() = default;

	bool IsValid() const override { return _handle != nullptr; }

	bool IsAPlugin() const override { return _isAPlugin; }

	void DLOpenLib() override {
		if (!(_handle = dlopen(_pathToLib.c_str(), RTLD_NOW | RTLD_LAZY))) {
			LogVarError("%s", dlerror());
		}
	}

	std::shared_ptr<T> DLGetInstance() override {
		using allocClass = T *(*)();
		using deleteClass = void (*)(T *);
		if (_handle != nullptr) {
			auto allocFunc = reinterpret_cast<allocClass>(
					dlsym(_handle, _allocClassSymbol.c_str()));
			auto deleteFunc = reinterpret_cast<deleteClass>(
					dlsym(_handle, _deleteClassSymbol.c_str()));

			if (!allocFunc || !deleteFunc) {
                _isAPlugin = false;
				DLCloseLib();
				//LogVarDebugError("%s", dlerror());
			}

			return std::shared_ptr<T>(
					allocFunc(),
					[deleteFunc](T *p){ deleteFunc(p); });
		}
		return nullptr;
	}

	void DLCloseLib() override {
		if (dlclose(_handle) != 0) {
			//LogVarDebugError("%s", dlerror());
		}
	}
};

}  // namespace dlloader
#endif