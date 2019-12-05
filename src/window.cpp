#include <iostream>

#include "window.hpp"

// Default callback will just resize the OpenGL viewport
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //int width;
    //int height;
    //glfwGetFramebufferSize(window, &width, &height);
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

Window::Window(const unsigned int width, const unsigned int height, const char *window_name) {
    // glfw: initialize and configure
    // ------------------------------
    glfwSetErrorCallback(glfw_error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create internal glfw window
    GLFWwindow* window = glfwCreateWindow(width, height, window_name, nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load all function pointers
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD");
    }  

    m_window = window;
}

Window::~Window() {
    glfwTerminate();
}

void Window::clear_screen() {
    // TODO: Set custom clear color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    // TODO: Turn on/off clear bits
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool Window::should_close() {
    return glfwWindowShouldClose(m_window);
}

GLFWwindow* Window::get_window() {
    return m_window;
}

void Window::enable_capability(Capability capability) {
    glEnable(static_cast<GLenum>(capability));
}

void Window::polygon_mode(PolygonMode mode) {
    glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(mode));
}

void Window::close() {
    glfwSetWindowShouldClose(m_window, true);
}

void Window::swap_and_poll() {
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

Keyboard::KeyState Window::get_key(Keyboard::Key key) {
    return static_cast<Keyboard::KeyState>(glfwGetKey(m_window, static_cast<GLint>(key)));
}

float Window::get_elapsed_time() {
    return glfwGetTime();
}

void Window::set_mouse_callback(GLFWmousebuttonfun mouse_btn_func, GLFWcursorposfun mouse_pos_func) {
    if(mouse_btn_func) {
        glfwSetMouseButtonCallback(m_window, mouse_btn_func);
    }

    if(mouse_pos_func) {
        glfwSetCursorPosCallback(m_window, mouse_pos_func);
    }
}

void Window::set_mouse_mode(MouseMode mode) {
    glfwSetInputMode(m_window, GLFW_CURSOR, static_cast<int>(mode));
}

void Window::set_zoom_callback(GLFWscrollfun func) {
    glfwSetScrollCallback(m_window, func);
}
