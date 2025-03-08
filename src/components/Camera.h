#pragma once

namespace Vu {
    struct Camera {

        float fov = 90.0f;

        float near = 0.01f;
        float far = 10000.0f;

        float cameraSpeed = 5.0f;
        float sensitivity = 0.005f;

        float yaw = 0.0f;
        float pitch = 0.0f;
        float roll = 0.0f;

        float lastX = 0;
        float lastY = 0;

        bool firstClick = true;
    };
}
