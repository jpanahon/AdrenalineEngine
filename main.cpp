/*
	main.cpp
	Adrenaline Engine

    This starts the entire program.
	Copyright © 2021 Stole Your Shoes. All rights reserved.
*/

#include "engine/adrenaline.h"

int main() {
    Adren::Config config{}; 
    config.appName = "Faehm";

    config.models = {
        Adren::Model("engine/resources/models/batman.obj", "engine/resources/textures/batman.png", false),
        Adren::Model("engine/resources/models/testingthing.obj", "engine/resources/textures/gmod.jpg", false),
        Adren::Model("engine/resources/models/viking_room.obj", "engine/resources/textures/viking_room.png", false)
    };

    Adren::Engine engine(config); 

    try {
    	engine.run();
    } catch (const std::exception& e) {
 		std::cerr << e.what() << std::endl;
 		return EXIT_FAILURE; 
 	}

	return EXIT_SUCCESS;
}
