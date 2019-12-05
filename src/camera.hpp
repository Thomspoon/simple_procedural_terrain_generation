// Adapted from the following resource:
// https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/camera.h

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <memory>
#include <vector>

struct CameraSettings {
public:
    CameraSettings(float zoom, float aspect_ratio, float near, float far);

    float zoom;
    float aspect_ratio;
    float near;
    float far;
};

class CameraProjection {
public:
    CameraProjection(CameraSettings settings) 
        : m_settings(settings)
    {
    }

    virtual ~CameraProjection(){}

    virtual glm::mat4 get_projection() = 0;

    CameraSettings& get_settings() {
        return m_settings;
    }

private:
    CameraSettings m_settings;
};

class Perspective : private CameraProjection {
public:
    Perspective(CameraSettings settings) 
        : CameraProjection(settings)
    {
    }

    glm::mat4 get_projection() {
        auto settings = get_settings();
        return glm::perspective(
            glm::radians(settings.zoom),
            settings.aspect_ratio,
            settings.near,
            settings.far
        );
    }
};

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
namespace CameraDefault {
    constexpr float YAW         = -90.0f;
    constexpr float PITCH       =  0.0f;
    constexpr float SPEED       =  5.0f;
    constexpr float SENSITIVITY =  0.1f;
    constexpr float ZOOM        =  45.0f;
}

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
template<typename Projection>
class Camera
{
public:
    Camera(
        CameraSettings settings,
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
        float yaw = CameraDefault::YAW, 
        float pitch = CameraDefault::PITCH
    )
      : m_front(glm::vec3(0.0f, 0.0f, -1.0f)), 
        m_movement_speed(CameraDefault::SPEED), 
        m_mouse_sensitivity(CameraDefault::SENSITIVITY),
        m_projection(std::make_unique<Projection>(settings))
    {
        m_position = position;
        m_world_up = up;
        m_yaw = yaw;
        m_pitch = pitch;

        update_camera_vectors();
    }

    glm::mat4 get_view_matrix() {
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    glm::mat4 get_projection() {
        return m_projection->get_projection();
    }

    glm::vec3 get_position() {
        return m_position;
    }

    void process_keyboard(CameraMovement direction, float delta_time) {
        float velocity = m_movement_speed * delta_time;
        if (direction == CameraMovement::FORWARD)
            m_position += m_front * velocity;
        if (direction == CameraMovement::BACKWARD)
            m_position -= m_front * velocity;
        if (direction == CameraMovement::LEFT)
            m_position -= m_right * velocity;
        if (direction == CameraMovement::RIGHT)
            m_position += m_right * velocity;
    }

    void process_mouse_movement(float xoffset, float yoffset, GLboolean constrain_pitch = true) {
        xoffset *= m_mouse_sensitivity;
        yoffset *= m_mouse_sensitivity;

        m_yaw   += xoffset;
        m_pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrain_pitch)
        {
            if (m_pitch > 89.0f) {
                m_pitch = 89.0f;
            }

            if (m_pitch < -89.0f) {
                m_pitch = -89.0f;
            }
        }

        // Update m_front, m_right and m_up vectors using the updated Euler angles
        update_camera_vectors();
    }

    void process_mouse_scroll(float yoffset) {
        auto settings = m_projection->get_settings();
        if (settings.zoom >= 1.0f && settings.zoom <= 45.0f) {
            settings.zoom -= yoffset;
        }

        if (settings.zoom <= 1.0f) {
            settings.zoom = 1.0f;
        }

        if (settings.zoom >= 45.0f) {
            settings.zoom = 45.0f;
        }
    }

private:
    // Camera Attributes
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_world_up;

    // Euler Angles
    float m_yaw;
    float m_pitch;
    
    // Camera options
    float m_movement_speed;
    float m_mouse_sensitivity;

    // Calculates the front vector from the Camera's (updated) Euler Angles
    void update_camera_vectors() {
        // Calculate the new m_front vector
        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);

        // Also re-calculate the m_right and m_up vector
        m_right = glm::normalize(glm::cross(m_front, m_world_up));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        m_up    = glm::normalize(glm::cross(m_right, m_front));
    }

    std::unique_ptr<Projection> m_projection;
};
