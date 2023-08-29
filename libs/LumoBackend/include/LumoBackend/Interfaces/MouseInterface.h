#pragma once
#pragma warning(disable : 4251)

#include <LumoBackend/Headers/LumoBackendDefs.h>

class MouseInterface {
public:
	bool canUpdateMouse = false;
	bool buttonDown[3] = {false, false, false};
	bool buttonDownLastFrame[3] = { false, false, false };
	float px = 0.0f;
	float py = 0.0f;
};
