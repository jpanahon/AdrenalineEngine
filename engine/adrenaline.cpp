/* 
	adrenaline.cpp
	Adrenaline Engine

	This file defines the entire engine class which connects all the things together.
*/

#include "adrenaline.h"

void Adren::Engine::run() {
	Renderer renderer(config.models, config.debug, config.enableGUI);
	renderer.run();
}