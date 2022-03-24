/* 
	adrenaline.cpp
	Adrenaline Engine

	This file defines the entire engine class which connects all the things together.
*/

#include "adrenaline.h"

void Adren::Engine::run() {
	rpc->initialize();
	rpc->update();

	Renderer renderer(config);
	renderer.run();

}