#ifndef _NABLA_SYSTEM_WORLD_H_
#define _NABLA_SYSTEM_WORLD_H_
#include "isystem.h"
#include "containers/vector.h"
#include "containers/map.h"

namespace nabla {
class World {
public:
	void Initialize();

	void Accept(ISystem* isys);

	void NextFrame() {

	}

	void OnGui() {

	}

	void Update() {
		for (auto sys : systems_) {

		}
	}

	Vector<ISystem*> systems_;
	Clock clock;
};
}


#endif // !_NABLA_SYSTEM_WORLD_H_
