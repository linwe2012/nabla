#ifndef _NABLA_CORE_CONFIG_H_
#define _NABLA_CORE_CONFIG_H_
#define NA_DEVELOPMENT
#if 0
#include "yaml-cpp/yaml.h"
#include <string>

#include "containers/vector.h"
#include "containers/map.h"
#include "logger.h"
#include "filesystem.h"

namespace nabla {
	
	class ConfigManager {		
	public:
		using Config = YAML::Node;

		ConfigManager(const char *basedir);

		void LoadFromFile(const char* nodename, const char* filename) noexcept;
		YAML::Node GetNode(const char* nodename) {
			auto itr = root_.find(nodename);

			if (itr == root_.end()) {
				NA_LOG_ERROR("%s not exist", nodename);
				return YAML::Node();
			}

			return itr->second;
		}

	private:

		Map<std::string, YAML::Node> root_;
		// std::vector<File> files_;
	};

	
	/*
	inline void ConfigManager::LoadFromFile(const char* nodename, const char* filename)
	{
		try {
			YAML::Node node = YAML::LoadFile(filename);

			if (root_.find(nodename) != root_.end()) {
				NA_LOG_ERROR("%s already exits", nodename);
				return;
			}

			root_[nodename] = node;

		}
		catch (YAML::BadFile & bf) {
			NA_LOG_ERROR("Unable to open file %s", filename);
			return;
		} 
		catch (YAML::ParserException& e) {
			NA_LOG_ERROR("Syntax error: %s", e.what());
			return;
		}
	}*/
}

#endif
#endif // !_NABLA_CORE_CONFIG_H_
