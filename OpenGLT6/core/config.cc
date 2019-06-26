#include "config.h"
#include "logger.h"
#if 0

namespace nabla {

	ConfigManager::ConfigManager(const char* basedir)
	{
		fs::path base(basedir);
		const char* dirs[] = {
			"materials",
			"shaders",
		};
		
		for (int i = 0; i < sizeof(dirs) / sizeof(const char*); ++i) {
			const char* dir = dirs[i];
			
		}
	}


}
#endif