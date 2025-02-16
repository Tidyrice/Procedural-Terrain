#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <memory>
#include <GLFW/glfw3.h>

class Window;
class Scene;

class Controller {
    public:
        static Controller* GetActiveController(Controller* instance = nullptr);

        Controller(Scene* s): scene_{s} {};
        ~Controller() = default;

        void UpdateKeyEvents(GLFWwindow* glfw_window);
        static void ProcessKeyEventsCallback(GLFWwindow* glfw_window, int key, int scancode, int action, int mods);
        static void ProcessMouseMovementCallback(GLFWwindow* glfw_window, double x, double y);

    private:
        void HandleKeysInterrupt(int key, int action);
        void HandleMouseMovement(double x, double y);

        Scene* scene_;
};

#endif // CONTROLLER_H