#pragma once
#pragma warning(disable : 4251)

#include <memory>
#include <LumoBackend/Headers/LumoBackendDefs.h>

template<typename T>
class LUMO_BACKEND_API WeakThisInterface {
protected:
	std::weak_ptr<T> m_This;
	
public:
	std::weak_ptr<T> GetWeak(){return m_This;}
};