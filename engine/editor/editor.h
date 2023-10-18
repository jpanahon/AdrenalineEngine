#pragma once
#include <imgui.h>
#include <vector>
#include <string>
#include <glfw/glfw3.h>
#include "renderer/renderer.h"

namespace Adren {
class Editor {
public:
    void start(ImGuiContext* ctx, Camera& camera, Renderer& renderer);
    void cameraInfo(bool* open, Camera& camera);
    void leftPanel();
    void rightPanel();
    void bottomPanel();
    void topPanel();
private:
    bool showCameraInfo = false;
};
}

/*
    To implement models at run time I would need to consider the following:
    
    Reload the scene by destroying the index, vertex, and dynamic uniform buffers.
    Import the model path from the Editor class into the Renderer superclass,
    without recreating an entire models vector. Code wise reloading the scene 
    is easy but the implementation of sending model paths from the editor class to
    the renderer class to turn into models is a difficult task without having to
    basically recreate the model vector.

    I could approach this with an add model function that manipulates the model
    vector.
*/