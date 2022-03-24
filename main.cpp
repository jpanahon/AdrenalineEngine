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
    //Model bistro("../engine/resources/models/bistro/bistro2.gltf");

    Model scientist("../engine/resources/models/scientist/scene.gltf");

    //Model revenant("../engine/resources/models/revenant/scene.gltf");
    //revenant.position = glm::vec3(0.0f, 0.0f, -15.5f);
    //revenant.scale = 50.0f;
    Model deccer("../engine/resources/models/deccer/SM_Deccer_Cubes_Textured_Embedded.gltf");
    //Model batman("../engine/resources/models/batman.gltf");
    //batman.scale = 50.0f;
    //batman.rotationAngle = 90.0f;
    //batman.rotationAxis = ADREN_X_AXIS;

    config.models = {sponza, deccer};

    Adren::Engine engine(config); 

    try {
    	engine.run();
    } catch (const std::exception& e) {
 		std::cerr << e.what() << std::endl;
 		return EXIT_FAILURE; 
 	}

	return EXIT_SUCCESS;
}
