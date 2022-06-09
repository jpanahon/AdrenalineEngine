#pragma once
#include "config.h"
#include <glfw/glfw3.h>

namespace Adren {

class Editor {
public:
    Editor() {}

    void start();
    void cameraInfo(bool* open);
    void style();
    void importModel();
};
}
