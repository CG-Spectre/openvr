#include "input_controller.h"

#include <cmath>

void InputController::updateCameraFromInput(const Uint8* kb, float fps, camera& cam,
                                            Vector3d& translationalVelocity, Vector3d& rotationalVelocity) const {
    translationalVelocity = Vector3d();
    rotationalVelocity = Vector3d();

    float right = 0.0f;
    float forward = 0.0f;
    float up = 0.0f;

    float yaw = cam.getPos()->rotation.y * (M_PI / 180.0f);

    if (kb[SDL_SCANCODE_LEFT]) {
        cam.getPos()->setRotation(Vector3d(cam.getPos()->rotation.x, cam.getPos()->rotation.y - 10 / fps, cam.getPos()->rotation.z));
        rotationalVelocity.y -= 10;
    }
    if (kb[SDL_SCANCODE_RIGHT]) {
        cam.getPos()->setRotation(Vector3d(cam.getPos()->rotation.x, cam.getPos()->rotation.y + 10 / fps, cam.getPos()->rotation.z));
        rotationalVelocity.y += 10;
    }
    if (kb[SDL_SCANCODE_UP]) {
        cam.getPos()->setRotation(Vector3d(cam.getPos()->rotation.x + 10 / fps, cam.getPos()->rotation.y, cam.getPos()->rotation.z));
        rotationalVelocity.x += 10;
    }
    if (kb[SDL_SCANCODE_DOWN]) {
        cam.getPos()->setRotation(Vector3d(cam.getPos()->rotation.x - 10 / fps, cam.getPos()->rotation.y, cam.getPos()->rotation.z));
        rotationalVelocity.x -= 10;
    }
    if (kb[SDL_SCANCODE_W]) {
        forward += 1 / fps;
    }
    if (kb[SDL_SCANCODE_A]) {
        right -= 1 / fps;
    }
    if (kb[SDL_SCANCODE_S]) {
        forward -= 1 / fps;
    }
    if (kb[SDL_SCANCODE_D]) {
        right += 1 / fps;
    }
    if (kb[SDL_SCANCODE_SPACE]) {
        up += 1 / fps;
    }
    if (kb[SDL_SCANCODE_LSHIFT]) {
        up -= 1 / fps;
    }

    if (forward != 0 || right != 0) {
        cam.getPos()->setRotation(Vector3d(cam.getPos()->rotation.x, cam.getPos()->rotation.y, 0));
    } else {
        cam.getPos()->setRotation(Vector3d(cam.getPos()->rotation.x, cam.getPos()->rotation.y, 0));
    }

    translationalVelocity = Vector3d(right, up, forward) * fps;

    cam.getPos()->setTranslation(Vector3d(
        cam.getPos()->pose.x + right * std::cos(yaw) + forward * std::sin(yaw),
        cam.getPos()->pose.y + up,
        cam.getPos()->pose.z + forward * std::cos(yaw) - right * std::sin(yaw)));
}
