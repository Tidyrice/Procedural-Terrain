#include "scene.h"
#include "camera.h"

Scene::Scene() = default;
Scene::~Scene() = default; //must define pure virtual destructor

glm::mat4 Scene::GetViewMatrix() const
{
    return camera->GetViewMatrix();
}
