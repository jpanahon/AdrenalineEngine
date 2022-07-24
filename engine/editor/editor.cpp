#include "editor.h"
#include <imgui.h>

void Adren::Editor::start() {
    //bool yep = true;
    //ImGui::ShowDemoWindow(&yep);

    if (showCameraInfo) { cameraInfo(&showCameraInfo); }

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem("Camera Properties", " ", &showCameraInfo);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("File")) {
            importModel();
        }

        ImGui::EndMainMenuBar();
    }
}

void Adren::Editor::cameraInfo(bool* open) {
    ImGui::Begin("Camera Properties", open);
    if (ImGui::BeginTabBar("CameraTabBar")) {
        if (ImGui::BeginTabItem("Stats")) {
            ImGui::Text("Front: X: %.3f, Y: %.3f, Z: %.3f \n", camera.front.x, camera.front.y, camera.front.z);
            ImGui::Text("Pos: X: %.3f, Y: %.3f, Z: %.3f \n", camera.pos.x, camera.pos.y, camera.pos.z);
            ImGui::Text("Up: X: %.3f, Y: %.3f, Z: %.3f \n", camera.up.x, camera.up.y, camera.up.z);
            ImGui::Text("Last X: %.f \n", camera.lastX);
            ImGui::Text("Last Y: %.f \n", camera.lastY);
            ImGui::Text("Camera Resolution: %dx%d", camera.width, camera.height);
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


void Adren::Editor::importModel() {
    std::string path = "../engine/resources/models/sponza/Sponza.gltf";
    modelPaths.push_back(path);
}

void Adren::Editor::style() {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../engine/resources/fonts/Montserrat-Regular.ttf", 14);

    ImVec4 uiBlack = ImVec4(0.008f, 0.008f, 0.008f, 1.000f);
    ImVec4 uiPurple = ImVec4(0.431f, 0.235f, 1.000f, 1.000f);
    ImVec4 uiFrame = ImVec4(0.014f, 0.014f, 0.014f, 1.000f);
    ImVec4 uiScroll = ImVec4(0.018f, 0.018f, 0.018f, 1.000f);

    ImGuiStyle* style = &ImGui::GetStyle();
    style->WindowPadding = ImVec2(8.0f, 8.0f);
    ImVec4* colors = style->Colors;
    colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
    colors[ImGuiCol_WindowBg] = uiBlack;
    colors[ImGuiCol_TitleBg] = uiPurple;
    colors[ImGuiCol_TitleBgActive] = uiPurple;
    colors[ImGuiCol_TitleBgCollapsed] = uiPurple;
    colors[ImGuiCol_PopupBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.9000f);
    colors[ImGuiCol_Button] = uiPurple;
    colors[ImGuiCol_Header] = uiPurple;
    colors[ImGuiCol_HeaderHovered] = uiPurple;
    colors[ImGuiCol_HeaderActive] = uiPurple;
    colors[ImGuiCol_FrameBg] = uiFrame;
    colors[ImGuiCol_Border] = uiFrame;
    colors[ImGuiCol_ResizeGrip] = uiFrame;
    colors[ImGuiCol_ResizeGripHovered] = uiFrame;
    colors[ImGuiCol_ResizeGripActive] = uiFrame;
    colors[ImGuiCol_MenuBarBg] = uiBlack;
    colors[ImGuiCol_ScrollbarBg] = uiFrame;
    colors[ImGuiCol_ScrollbarGrab] = uiScroll;
    colors[ImGuiCol_ScrollbarGrabActive] = uiScroll;
    colors[ImGuiCol_ScrollbarGrabHovered] = uiScroll;
    colors[ImGuiCol_SliderGrab] = uiPurple;
    colors[ImGuiCol_SliderGrabActive] = uiPurple;
    colors[ImGuiCol_Tab] = uiPurple;
    colors[ImGuiCol_TabActive] = uiPurple;
    colors[ImGuiCol_TabHovered] = uiPurple;
    colors[ImGuiCol_TabUnfocused] = uiPurple;
    colors[ImGuiCol_TabUnfocusedActive] = uiPurple;
}