/* 
	adrenaline.cpp
	Adrenaline Engine

	This file defines the entire engine class which connects all the things together.
*/
#include "adrenaline.h"

/* void Adren::Engine::setupRenderer() {
	renderer.run();
} */

void Adren::Engine::run() {
	Renderer renderer(config.models, config.debug);
	// setupRenderer();
}