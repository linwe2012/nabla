#ifndef _NABLA_CORE_SERVICE_LOCATOR_H_
#define _NABLA_CORE_SERVICE_LOCATOR_H_
#include <unordered_map>
#include <string>

#include "logger.h"

namespace nabla {

class ServiceLocator {
public:
	template<typename K, typename V>
	using HashMap = std::unordered_map<K, V>;
	template<typename Serv>
	void launch(const char* serv_name) {
		if (Locate<Serv>(serv_name)) {
			NA_LOG_ERROR("%s already exists", serv_name);
			return;
		}
		Serv* serv = Serv::initService();
		
		services_[serv_name] = serv;
	}

	template<typename Serv>
	Serv* Locate(const char* name) {
		auto itr = services_.find(name);
		if (itr == services_.end()) { return nullptr; }

		return static_cast<Serv*>(itr->second);
	}

private:
	HashMap<std::string, void*> services_;
};

extern ServiceLocator GlobalServices;

}
#endif // !_NABLA_CORE_SERVICE_LOCATOR_H_
