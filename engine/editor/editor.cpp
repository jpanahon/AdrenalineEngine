#include "editor.h"
#include <glm/gtc/type_ptr.hpp>
#include "nfd.h"

void Adren::Editor::start(ImGuiContext* ctx, Camera& camera, Renderer& renderer) {
    ImGui::SetCurrentContext(ctx);

    //bool yep = true;
    //ImGui::ShowDemoWindow(&yep);

    if (showCameraInfo) cameraInfo(&showCameraInfo, camera);

    leftPanel();
    rightPanel();
    bottomPanel();
    topPanel();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Import Model...")) {
                NFD_Init();
                nfdchar_t* outPath;
                nfdfilteritem_t filterItem[1] = { { "glTF Scene", "gltf" } };
                nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
                if (result == NFD_OKAY) {
                    renderer.addModel(outPath);
                    NFD_FreePath(outPath);
                } else if (result == NFD_CANCEL) {
                    std::cerr << "User pressed cancel" << std::endl;
                } else {
                    std::cerr << "Error: " << NFD_GetError() << "\n \n" << std::endl;
                }

                NFD_Quit();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem("Camera Properties", " ", &showCameraInfo);
            ImGui::EndMenu();
        }


        ImGui::EndMainMenuBar();
    }
}

void Adren::Editor::cameraInfo(bool* open, Camera& camera) {
    ImGui::Begin("Camera Properties", open);
    if (ImGui::BeginTabBar("CameraTabBar")) {
        if (ImGui::BeginTabItem("Stats")) {
            ImGui::Text("Front: X: %.3f, Y: %.3f, Z: %.3f \n", camera.front.x, camera.front.y, camera.front.z);
            ImGui::Text("Pos: X: %.3f, Y: %.3f, Z: %.3f \n", camera.pos.x, camera.pos.y, camera.pos.z);
            ImGui::Text("Up: X: %.3f, Y: %.3f, Z: %.3f \n", camera.up.x, camera.up.y, camera.up.z);
            ImGui::Text("Last X: %.f \n", camera.lastX);
            ImGui::Text("Last Y: %.f \n", camera.lastY);
            ImGui::Text("Camera Resolution: %dx%d", camera.getWidth(), camera.getHeight());
            ImGui::EndTabItem();
        }

        const char* format = "%.3f";
        if (ImGui::BeginTabItem("Viewport")) {
            ImGui::Text("Camera FOV");
            ImGui::SliderInt("30 to 120", &camera.fov, 30, 120);
            ImGui::Text("Camera Draw Distance (x 1000)");
            ImGui::SliderInt("1 to 100", &camera.drawDistance, 1, 100);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Settings")) {
            ImGui::InputFloat3("Camera Front", glm::value_ptr(camera.front));
            ImGui::InputFloat3("Camera Position", glm::value_ptr(camera.pos));
            ImGui::Text("Camera Speed");
            ImGui::SliderFloat("0.001 to 100.0", &camera.speed, 0.001f, 100.0f, format);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void Adren::Editor::leftPanel() {
    ImGui::Begin("Left Panel");
    ImGui::Text("LEFT PANEL");
    ImGui::End();
}

void Adren::Editor::rightPanel() {
    ImGui::Begin("Right Panel");
    ImGui::Text("RIGHT PANEL");
    ImGui::End();
}

void Adren::Editor::bottomPanel() {
    ImGui::Begin("Bottom Panel");
    ImGui::Text("BOTTOM PANEL");
    ImGui::End();
}

void Adren::Editor::topPanel() {
    ImGui::Begin("Top Panel");
    ImGui::Text("TOP PANEL");
    ImGui::End();
}
