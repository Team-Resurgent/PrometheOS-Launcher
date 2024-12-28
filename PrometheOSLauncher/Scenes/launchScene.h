#pragma once

#include "scene.h"

#include "..\xboxInternals.h"

class launchScene : public scene
{
public:
	launchScene();
	void update();
	void render();
private:
	int mSelectedControl;

	int mCounter;
};
