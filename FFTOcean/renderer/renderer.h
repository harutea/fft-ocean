#ifndef RENDERER_H
#define RENDERER_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include <FFTOcean/object/object.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace FFTOcean
{
    class Renderer
    {
    public:
        Renderer();
        ~Renderer();
        void setup();
        void render();
        void clear();
        void registerObject(Object *object);

    private:
        GLFWwindow *window;
        const int WINDOW_WIDTH = 1200, WINDOW_HEIGHT = 900;

        glm::vec3 cameraPos;
        glm::vec3 cameraTarget;
        glm::vec3 cameraDirection;
        glm::vec3 up;
        glm::vec3 cameraRight;
        glm::vec3 cameraFront;
        glm::vec3 cameraUp;
        glm::mat4 view;
        glm::mat4 projection;
        float fov;
        float yaw;
        float pitch;
        float lastX, lastY;
        bool firstMouse;

        float deltaTime;

        enum Color
        {
            COLOR_BLACK,
            COLOR_WHITE
        };

        Color colorMode;

        std::vector<Object *> objectPool;

        static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
        static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
        void mouse_event(double xpos, double ypos);
        static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
        void key_event(int key, int scancode, int action, int mods);
        static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
        void scroll_event(double xoffset, double yoffset);
        void processInput(GLFWwindow *window);
    };

}

#endif RENDERER_H