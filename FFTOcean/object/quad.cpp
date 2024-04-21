#include "quad.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <FFTOcean/renderer/shader.h>
#include <FFTOcean/renderer/compute_shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <string>

using namespace FFTOcean;
using namespace std;

Quad::Quad()
{
    initX = 0.0f;
    initY = 0.0f;
    initZ = 0.0f;
}

Quad::Quad(float _initX, float _initY, float _initZ)
{
    initX = _initX;
    initY = _initY;
    initZ = _initZ;
}

Quad::~Quad()
{
}

void Quad::setup()
{
    cout << "setup quad" << endl;
    this->compShader0 = new ComputeShader("./shaders/initial_spectrum.comp");
    this->compShader1 = new ComputeShader("./shaders/fourier_component.comp");
    this->shader = new Shader("./shaders/quad.vert", "./shaders/quad.frag");

    shader->use();

    /* Textures for Compute Shader */

    glGenTextures(1, &texture0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA,
                 GL_FLOAT, NULL);

    glGenTextures(1, &texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA,
                 GL_FLOAT, NULL);

    glGenTextures(1, &texture2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA,
                 GL_FLOAT, NULL);

    glBindImageTexture(0, texture0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(1, texture1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(2, texture2, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    /* Calculate Vertex Positions */

    float vertices[] = {
        // vertex position, texture
        -1, -1, 0, 0, 0,
        -1, 1, 0, 0, 1,
        1, 1, 0, 1, 1,
        1, -1, 0, 1, 0};

    unsigned int indices[] = {0, 1, 2,
                              0, 2, 3};

    /* VAO, VBO, EBO */
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 5 * 4, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Quad::render()
{
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // cout << "render Ocean" << endl;

    compShader0->use();
    compShader0->setInt("N", 256);
    compShader0->setInt("L", 1000);
    compShader0->setFloat("A", 4);
    compShader0->setVec2("w", 1, 1);
    compShader0->setFloat("V", 40);
    glDispatchCompute((unsigned int)TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    compShader1->use();
    compShader1->setInt("N", 256);
    compShader1->setInt("L", 1000);
    compShader1->setFloat("t", float(glfwGetTime()));
    glDispatchCompute((unsigned int)TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);

    // make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* Transform */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->use();
    shader->setInt("tex", 2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glm::mat4 model = glm::mat4(1.0f);

    // model = glm::scale(model, glm::vec3(10.0f, 0.5f, 0.5f));
    model = glm::translate(model, glm::vec3(initX, initY, initZ));

    // glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));

    int modelLoc = glGetUniformLocation(shader->getID(), "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    // int normalMatrixLoc = glGetUniformLocation(shader->getID(), "normalMatrix");
    // glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    int viewLoc = glGetUniformLocation(shader->getID(), "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    int projectionLoc = glGetUniformLocation(shader->getID(), "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    /* Draw */
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Quad::clear()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    // glDeleteBuffers(1, &EBO);
}

void Quad::updateView(glm::mat4 _view)
{
    this->view = _view;
}

void Quad::updateProjection(glm::mat4 _projection)
{
    this->projection = _projection;
}

void Quad::updateCameraPos(glm::vec3 _cameraPos)
{
    this->cameraPos = _cameraPos;
}