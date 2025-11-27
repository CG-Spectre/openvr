#pragma once

#include <SDL3/SDL.h>

#include "render/camera.h"

class InputController {
public:
    InputController() = default;

    void updateCameraFromInput(const Uint8* keyboardState, float fps, camera& cam,
                               Vector3d& translationalVelocity, Vector3d& rotationalVelocity) const;
};
