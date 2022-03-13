/*
	main.cpp
	Adrenaline Engine

    This starts the entire program.
*/

#include "engine/adrenaline.h"

int main() {
    Config config{};
    config.debug = false;
    config.enableGUI = true;
    
    Model sponza("../engine/resources/models/sponza/sponza.gltf");

    Model scientist("../engine/resources/models/scientist/scene.gltf");
    scientist.rotationAngle = -90.0f;
    scientist.rotationAxis = ADREN_X_AXIS;

    //Model deccer("../engine/resources/models/deccer/SM_Deccer_Cubes_Textured_Embedded.gltf");

    config.models = {sponza, scientist};

    Adren::Engine engine(config); 

    try {
    	engine.run();
    } catch (const std::exception& e) {
 		std::cerr << e.what() << std::endl;
 		return EXIT_FAILURE; 
 	}

	return EXIT_SUCCESS;
}
