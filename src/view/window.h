#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Shader;
class Scene;
class Texture;

class Window {
    public:
        static Window* GetActiveWindow(Window* instance = nullptr);
        static GLFWwindow* GetActiveGlfwWindowPtr() { return GetActiveWindow()->glfw_window_ptr_; }

        Window(int w, int h, std::string window_name, Scene* s);
        ~Window() = default;

        void RegisterWindowCallbacks();
        void SetFullScreen(bool full_screen);
        bool IsFullScreen() const;

        enum class RenderMode {
            FILL_MODE,
            WIREFRAME_MODE
        };
        void SetRenderMode(RenderMode mode);
        void RenderScene();

        void SetShader(Shader* s);
        inline void SetScene(Scene* s) { scene_ = s; }

        inline const Shader& GetShader() const { return *shader_; }
        inline int GetWindowWidth() const { return window_width_; }
        inline int GetWindowHeight() const { return window_height_; }

        inline float GetDeltaTime() const { return delta_time_; }

        void Clear();

    private:
        Window(Window const&) = delete;
        Window(Window&&) = delete;
        Window& operator=(Window const&) = delete;
        Window& operator=(Window&&) = delete;
        static void WindowResizeCallback(GLFWwindow* glfw_window, int w, int h);
        void HandleResize(int w, int h);

        const glm::mat4 GetViewMatrix() const;
        const glm::mat4 GetProjectionMatrix() const;

        void UpdateDeltaTime();

        /* PRIVATE MEMBERS */
        bool full_screen_ = false;
        Shader* shader_;
        Scene* scene_;

        GLFWwindow* glfw_window_ptr_;
        int window_width_ = 0;
        int window_height_ = 0;
        int window_width_prev_, window_height_prev_, window_x_prev, window_y_prev; //for fullscreen toggle

        float delta_time_ = 0.0f;
        float last_frame_ = 0.0f;
};

#endif // WINDOW_H