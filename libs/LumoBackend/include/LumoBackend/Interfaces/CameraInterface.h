#pragma once
#pragma warning(disable : 4251)

#include <LumoBackend/Headers/LumoBackendDefs.h>
#include <glm/mat4x4.hpp>

#include <string>
#include <memory>

#ifndef CAM_FAR
#define CAM_FAR 1000.0f
#endif

class LUMO_BACKEND_API CameraInterface {
public:
    glm::mat4 uView = glm::mat4(1.0f);
    glm::mat4 uProj = glm::mat4(1.0f);
    glm::mat4 uModel = glm::mat4(1.0f);
    glm::mat4 uCam = glm::mat4(1.0f);
    glm::mat4 uInvCam = glm::mat4(1.0f);
    glm::mat4 uNormalMatrix = glm::mat4(1.0f);
    float cam_near = 0.001f;
    float cam_far = CAM_FAR;

public:
    virtual void NeedCamChange() = 0;
    virtual void SetTranslation(const glm::vec3& vTranslation) = 0;
};