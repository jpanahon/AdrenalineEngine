/* 
	adrenaline.cpp
	Adrenaline Engine

	This file defines the entire engine class which connects all the things together.
	Copyright © 2020 Stole Your Shoes. All rights reserved.
*/
#include "adrenaline.h"

void Adren::Engine::setupRenderer() {
	Renderer renderer(config.windowHeight, config.windowHeight, config.appName, config.models);
	renderer.run();
}

void Adren::Engine::run() {
	setupRenderer();
}