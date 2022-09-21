/*
	main.cpp
	Adrenaline Engine

    This starts the entire program.
*/

#include "engine/adrenaline.h"

int main() {
    
    /*Model sponza("../engine/resources/models/sponza/Sponza.gltf");

    Model newSponza("../engine/resources/models/newsponza/NewSponza_Main_Blender_glTF.gltf");

    Model scientist("../engine/resources/models/scientist/scene.gltf");

    Model deccer("../engine/resources/models/deccer/SM_Deccer_Cubes_Textured_Embedded.gltf");
    
    Model batman("../engine/resources/models/batman.gltf");

    Model loba("../engine/resources/models/loba/scene.gltf");
    
    Model cj("../engine/resources/models/CJ/scene.gltf");*/

    Adren::Engine* engine = new Adren::Engine;

    try {
    	engine->run();
    } catch (const std::exception& e) {
        std::cerr << "Shutting down!" << "\n \n" << std::endl;
        engine->cleanup();
        delete engine;
 		std::cerr << e.what() << std::endl;
 		return EXIT_FAILURE; 
 	}

	return EXIT_SUCCESS;
}
