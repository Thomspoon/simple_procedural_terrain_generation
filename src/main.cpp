#include <iostream>
#include <limits>

#include "glm/glm.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "camera.hpp"
#include "drawable.hpp"
#include "shader.hpp"
#include "window.hpp"

#include "drawables/cube.hpp"
#include "drawables/terrain_squares.hpp"

// settings
constexpr auto WINDOW_WIDTH = 1440;
constexpr auto WINDOW_HEIGHT = 900;

constexpr auto GRID_SIZE = 150;

auto camera_settings = CameraSettings(CameraDefault::ZOOM, WINDOW_WIDTH / WINDOW_HEIGHT, 0.1, 1000.0);
auto camera = Camera<Perspective>(camera_settings, glm::vec3(-50.0f, 60.0f, GRID_SIZE / 2.0f), glm::vec3(0.0, 1.0, 0.0), 0.0, -35.0);

Window window(WINDOW_WIDTH, WINDOW_HEIGHT, "Procedural Terrain Generation");

bool focus = true;

GenerationSettings settings;

// custom callback 
void process_input(float delta_time)
{
    using namespace Keyboard;

    if(window.get_key(Key::KEY_ESCAPE) == KeyState::PRESSED) {
        focus = false;
        window.set_mouse_mode(MouseMode::NORMAL);
    }

    if(window.get_key(Key::KEY_W) == KeyState::PRESSED) {
        camera.process_keyboard(CameraMovement::FORWARD, delta_time);
    }

    if(window.get_key(Key::KEY_A) == KeyState::PRESSED) {
        camera.process_keyboard(CameraMovement::LEFT, delta_time);
    }

    if(window.get_key(Key::KEY_S) == KeyState::PRESSED) {
        camera.process_keyboard(CameraMovement::BACKWARD, delta_time);
    }

    if(window.get_key(Key::KEY_D) == KeyState::PRESSED) {
        camera.process_keyboard(CameraMovement::RIGHT, delta_time);
    }

    if(window.get_key(Key::KEY_Q) == KeyState::PRESSED) {
        window.polygon_mode(PolygonMode::FILL);
    }

    if(window.get_key(Key::KEY_E) == KeyState::PRESSED) {
        window.polygon_mode(PolygonMode::LINE);
    }

    if(window.get_key(Key::KEY_LEFT) == KeyState::PRESSED) {
        settings.offset.x -= 0.01;
    }

    if(window.get_key(Key::KEY_RIGHT) == KeyState::PRESSED) {
        settings.offset.x += 0.01;
    }

    if(window.get_key(Key::KEY_UP) == KeyState::PRESSED) {
        settings.offset.y += 0.01;
    }

    if(window.get_key(Key::KEY_DOWN) == KeyState::PRESSED) {
        settings.offset.y -= 0.01;
    }

}

void process_mouse_button(GLFWwindow* glfw_window, int button, int action, int mods) {
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && focus == false && !ImGui::GetIO().WantCaptureMouse) {
        focus = true;
        window.set_mouse_mode(MouseMode::DISABLED);
    }
}

void process_mouse_movement(GLFWwindow* glfw_window, double xpos, double ypos) {
    static auto first_mouse = true;
    static auto last_x = 0.0;
    static auto last_y = 0.0;

    if (first_mouse)
    {
        last_x = xpos;
        last_y = ypos;
        first_mouse = false;
    }

    float xoffset = xpos - last_x;
    float yoffset = ypos - last_y;

    last_x = xpos;
    last_y = ypos;

    if(focus) {
        camera.process_mouse_movement(xoffset, yoffset);
    }
}

int main() try {
    window.set_mouse_callback(process_mouse_button, process_mouse_movement);
    window.set_mouse_mode(MouseMode::DISABLED);
    window.enable_capability(Capability::DEPTH_TEST);

    auto mvm_shader = Shader::create<Shaders::Mvm>();
    auto terrain_shader = Shader::create<Shaders::Terrain>();

    auto light = Cube::create();
    auto light_position = glm::vec3(GRID_SIZE / 2.0f, 100.0f, GRID_SIZE / 2.0f);

    auto terrain = TerrainSquares::create(GRID_SIZE);

    auto delta_time = 0.0f;
    auto last_frame = 0.0f;

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void) io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window.get_window(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    GenerationSettings last_settings;

    while (!window.should_close())
    {
        auto current_frame = window.get_elapsed_time();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        process_input(delta_time);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        window.clear_screen();

        glm::mat4 view = camera.get_view_matrix();
        glm::mat4 projection = camera.get_projection();

        ImGui::Begin("Procedural Generation Renderer");

        ImGui::Text("Edit Configuration Parameters");

        ImGui::SliderInt("seed", &settings.seed, 0, std::numeric_limits<int>::max() / 10);
        ImGui::SliderFloat("height", &settings.height_scale, 1.0f, 100.0f);
        ImGui::SliderFloat("scale", &settings.scale, 1.0f, 100.0f);
        ImGui::SliderInt("octaves", &settings.octaves, 1, 10);
        ImGui::SliderFloat("persistance", &settings.persistence, 0.1f, 2.5f);
        ImGui::SliderFloat("Lacunarity", &settings.lacunarity, 0.1f, 2.5f);
        ImGui::SliderFloat("X Offset", &settings.offset.x, -100.0f, 100.0f);
        ImGui::SliderFloat("Y Offset", &settings.offset.y, -100.0f, 100.0f);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Render();

        if(!(settings == last_settings)) {
            last_settings = settings;
            terrain->update(settings);
        }

        ///////////////////////////////////////////////////////////////////////
        //
        // Draw the light cube
        //
        ///////////////////////////////////////////////////////////////////////
        mvm_shader.use();
        mvm_shader.set_vec3("object_color", glm::vec3(1.0f, 1.0f, 1.0f));
        mvm_shader.set_mat4("projection", projection);
        mvm_shader.set_mat4("view", view);
        mvm_shader.set_mat4("model", glm::translate(glm::mat4x4(1.0), light_position));
        light->draw();
        
        terrain_shader.use();
        terrain_shader.set_vec3("light_color", glm::vec3(1.0, 1.0, 1.0));
        terrain_shader.set_vec3("light_pos", light_position);
        terrain_shader.set_mat4("projection", projection);
        terrain_shader.set_mat4("view", view);
        terrain_shader.set_mat4("model", glm::translate(glm::mat4x4(1.0), glm::vec3(0.0f, -1.0f, 0.0f)));
        terrain->draw();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // swap buffers and poll events
        window.swap_and_poll();
    }

    return 0;
} catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
}
