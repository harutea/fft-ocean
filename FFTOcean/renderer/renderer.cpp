#include "renderer.h"
#include <iostream>
#include <cstring>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace FFTOcean;
using namespace std;

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::setup()
{
    /* Setup Window */
    glfwInit();
    glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "FFTOcean by harutea", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "[Error] Failed to create GLFW window" << std::endl;
        glfwTerminate();
        // return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress))
    {
        cout << "[Error] Failed to initialize OpenGL context." << endl;
        return;
    };
    // glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);

    colorMode = COLOR_BLACK;

    /* Setup Camera */
    cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    cameraDirection = glm::normalize(cameraPos - cameraTarget);

    up = glm::vec3(0.0f, 1.0f, 0.0f);
    cameraRight = glm::normalize(glm::cross(up, cameraDirection));
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    cameraUp = glm::cross(cameraDirection, cameraRight);

    fov = 45.0f;

    yaw = -90.0f;
    pitch = 0.0f;

    lastX = 400, lastY = 300;

    /* Setup Callbacks */
    firstMouse = false;
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, &Renderer::framebuffer_size_callback);
    glfwSetCursorPosCallback(window, &Renderer::mouse_callback);
    glfwSetKeyCallback(window, &Renderer::key_callback);
    glfwSetScrollCallback(window, &Renderer::scroll_callback);

    /* Setup all objects in Object Pool */
    for (auto itr = objectPool.begin(); itr != objectPool.end(); itr++)
    {
        (*itr)->setup();
    }
}

void Renderer::render()
{
    deltaTime = 0.0f;
    float lastTime = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        this->processInput(window);
        if (colorMode == COLOR_WHITE)
            glClearColor(0.99f, 0.99f, 0.99f, 1.0f);
        else
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        projection = glm::perspective(glm::radians(fov), float(WINDOW_WIDTH) / float(WINDOW_HEIGHT), 0.1f, 100.0f);

        /* Render all objects in Object Pool */
        for (auto itr = objectPool.begin(); itr != objectPool.end(); itr++)
        {
            (*itr)->updateView(view);             // Update view transform matrix of objects
            (*itr)->updateProjection(projection); // Update projection transform matrix of objects
            (*itr)->updateCameraPos(cameraPos);
            (*itr)->render();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Renderer::clear()
{
    for (auto itr = objectPool.begin(); itr != objectPool.end(); itr++)
    {
        (*itr)->clear();
    }

    glfwTerminate();
}

void Renderer::registerObject(Object *object)
{
    objectPool.push_back(object);
}

void Renderer::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Renderer::mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    static_cast<Renderer *>(glfwGetWindowUserPointer(window))->mouse_event(xpos, ypos);
}

void Renderer::mouse_event(double xpos, double ypos)
{
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
        return;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
};

void Renderer::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    static_cast<Renderer *>(glfwGetWindowUserPointer(window))->key_event(key, scancode, action, mods);
}

void Renderer::key_event(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        if (colorMode == COLOR_BLACK)
            colorMode = COLOR_WHITE;
        else
            colorMode = COLOR_BLACK;
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Renderer::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    static_cast<Renderer *>(glfwGetWindowUserPointer(window))->scroll_event(xoffset, yoffset);
}

void Renderer::scroll_event(double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

void Renderer::processInput(GLFWwindow *window)
{
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
        return;

    float cameraSpeed = 2.0f * this->deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}
