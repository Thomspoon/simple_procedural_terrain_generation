#include "camera.hpp"

CameraSettings::CameraSettings(float t_zoom, float t_aspect_ratio, float t_near, float t_far) 
    : zoom(t_zoom), aspect_ratio(t_aspect_ratio), near(t_near), far(t_far)
{
}
