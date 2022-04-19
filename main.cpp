/*
	main.cpp
	Adrenaline Engine

    This starts the entire program.
*/

#include "engine/adrenaline.h"

int main() {
    Config config{};
    config.debug = true;
    config.enableGUI = true;
    
    Model sponza("../engine/resources/models/sponza2/Sponza.gltf");

    Model scientist("../engine/resources/models/scientist/scene.gltf");
    
    Model deccer("../engine/resources/models/deccer/SM_Deccer_Cubes_Textured_Embedded.gltf");
    
    config.models = {sponza, deccer, scientist};

    Adren::Engine engine(config); 

    try {
    	engine.run();
    } catch (const std::exception& e) {
 		std::cerr << e.what() << std::endl;
 		return EXIT_FAILURE; 
 	}

	return EXIT_SUCCESS;
}
