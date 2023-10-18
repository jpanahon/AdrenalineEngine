/*
	main.cpp
	Adrenaline Engine

    This starts the entire program.
*/

#include "engine/adrenaline.h"

int main() {
    Adren::Engine engine{};

    try {
    	engine.run();
    } catch (const std::exception& e) {
        std::cerr << "Shutting down!" << "\n \n" << std::endl;
        engine.cleanup();
 		std::cerr << e.what() << std::endl;
 		return EXIT_FAILURE; 
 	}

	return EXIT_SUCCESS;
}
