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

Quad::Quad(float _initX, float _initY, float _initZ, int _planeSize)
{
    initX = _initX;
    initY = _initY;
    initZ = _initZ;
    planeSize = _planeSize;
}

Quad::~Quad()
{
}

void Quad::setup()
{
    cout << "setup quad" << endl;
    this->initialComp = new ComputeShader("./shaders/initial_spectrum.comp");
    this->btComp = new ComputeShader("./shaders/butterfly_texture.comp");
    this->fcComp = new ComputeShader("./shaders/fourier_component.comp");
    this->copyImageComp = new ComputeShader("./shaders/copy_image.comp");
    this->ipComp = new ComputeShader("./shaders/inverse_and_permute.comp");
    this->butterflyComp = new ComputeShader("./shaders/butterfly.comp");

    this->shader = new Shader("./shaders/ocean.vert", "./shaders/ocean.frag");


    /* Textures for Compute Shader */

    glGenTextures(7, textures);
    for (int i = 0; i < 7; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glBindImageTexture(i, textures[i], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    }

    /* Set Uniforms */
    shader->use();
    shader->setInt("displacement", 6);
    shader->setVec3("material.ambient", 0.1f, 0.6f, 0.3f);
    shader->setVec3("material.diffuse", 0.1f, 0.6f, 0.3f);
    shader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
    shader->setFloat("material.shininess", 32.0f);

    /* Calculate Vertex Positions */
    float *vertices = new float[5 * planeSize * planeSize];

    for (int i = 0; i < planeSize; i++)
    {
        for (int j = 0; j < planeSize; j++)
        {
            float &vertX = vertices[i * 5 * planeSize + j * 5];
            float &vertY = vertices[i * 5 * planeSize + j * 5 + 1];
            float &vertZ = vertices[i * 5 * planeSize + j * 5 + 2];
            float &u = vertices[i * 5 * planeSize + j * 5 + 3];
            float &v = vertices[i * 5 * planeSize + j * 5 + 4];

            vertX = (float)i/(planeSize-1);
            vertY = 0;
            vertZ = (float)j/(planeSize-1);

            u = (float)i/(planeSize-1);
            v = (float)j/(planeSize-1);
        }
    }

    /* Calculate Indices */

    unsigned int *indices = new unsigned int[(planeSize - 1) * (planeSize - 1) * 2 * 3];
    int current = 0;
    for (int i = 0; i < planeSize - 1; i++)
    {
        for (int j = 0; j < planeSize - 1; j++)
        {
            indices[current++] = i * planeSize + j;
            indices[current++] = i * planeSize + j + planeSize;
            indices[current++] = i * planeSize + j + planeSize + 1;

            indices[current++] = i * planeSize + j;
            indices[current++] = i * planeSize + j + planeSize + 1;
            indices[current++] = i * planeSize + j + 1;
        }
    }

    /* VAO, VBO, EBO */
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 5 * planeSize * planeSize, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * (planeSize - 1) * (planeSize - 1) * 2 * 3, indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // normal attribute
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    // glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /* Dispatch Initial Spectrum Texture */
    initialComp->use();
    initialComp->setInt("N", 256);
    initialComp->setInt("L", 1000);
    initialComp->setFloat("A", 4);
    initialComp->setVec2("w", 1, 1);
    initialComp->setFloat("V", 40);
    glDispatchCompute((unsigned int)TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* Pass the bit reversed index to butterfly texture using SSBO */

    int n = 256;
    int log2n = static_cast<int>(log2(n));
    int *bit_reversed = new int[n + 1];
    for (int i = 0; i <= n; i++)
    {
        int reversed = 0;
        for (int j = 0; j < log2n; j++)
        {
            if (i & (1 << j))
                reversed |= 1 << (log2n - 1 - j);
        }
        bit_reversed[i] = reversed;
    }
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * n, bit_reversed, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssbo);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    delete[] bit_reversed;

    checkGLError();

    /* Dispatch Butterfly Texture */

    btComp->use();
    btComp->setInt("N", 256);
    glDispatchCompute(8, 16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    checkGLError();
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    // int *ptr = (int *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    // for (int i = 0; i < 256; i++)
    // {
    //     cout << "Data[" << i << "] = " << ptr[i] << endl;
    // }
    // glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void Quad::render()
{
    /* Fourier Components */
    fcComp->use();
    fcComp->setInt("N", 256);
    fcComp->setInt("L", 1000);
    fcComp->setFloat("t", float(glfwGetTime()));
    glDispatchCompute((unsigned int)16, (unsigned int)16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    copyImageComp->use();
    glDispatchCompute((unsigned int)16, (unsigned int)16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    pingpong = 0;

    for (int i = 0; i < int(log2(256)); i++)
    {
        butterflyComp->use();
        butterflyComp->setInt("direction", 0);
        butterflyComp->setInt("stage", i);
        butterflyComp->setInt("pingpong", pingpong);
        glDispatchCompute((unsigned int)16, (unsigned int)16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        pingpong = (pingpong + 1) % 2;
        // if (i == 7)
        // {
        //     checkTextureContent(textures[5], 256, 256);
        // }
    }
    for (int i = 0; i < int(log2(256)); i++)
    {
        butterflyComp->use();
        butterflyComp->setInt("direction", 1);
        butterflyComp->setInt("stage", i);
        butterflyComp->setInt("pingpong", pingpong);
        glDispatchCompute((unsigned int)16, (unsigned int)16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        pingpong = (pingpong + 1) % 2;
    }

    ipComp->use();
    ipComp->setInt("N", 256);
    ipComp->setInt("pingpong", pingpong);
    glDispatchCompute((unsigned int)16, (unsigned int)16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* Transform */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, textures[6]);
    shader->use();

    glm::mat4 model = glm::mat4(1.0f);

    // model = glm::scale(model, glm::vec3(10.0f, 0.5f, 0.5f));
    model = glm::translate(model, glm::vec3(initX, initY, initZ));

    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));

    glm::vec3 lightPos(1.1f, 3.0f, 2.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 objectColor(0.1f, 1.0f, 0.3f);
    shader->setVec3("lightPos", lightPos);
    shader->setVec3("viewPos", cameraPos);
    shader->setVec3("lightColor", lightColor);
    shader->setVec3("objectColor", objectColor);
    shader->setFloat("time", glfwGetTime());

    int modelLoc = glGetUniformLocation(shader->getID(), "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    int normalMatrixLoc = glGetUniformLocation(shader->getID(), "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    int viewLoc = glGetUniformLocation(shader->getID(), "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    int projectionLoc = glGetUniformLocation(shader->getID(), "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    /* Draw */
    glBindVertexArray(VAO);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, (planeSize - 1) * (planeSize - 1) * 2 * 3, GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

void Quad::checkGLError()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error: " << err << std::endl;
    }
}

void Quad::checkTextureContent(GLuint texture, int width, int height)
{
    std::vector<float> data(width * height * 4); // assuming RGBA32F
    glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data.data());

    // Print the first few values for debugging
    for (int i = 0; i < std::min(40000, width * height * 4); i += 4)
    {
        std::cout << "Texel " << i / 4 << ": ("
                  << data[i] << ", " << data[i + 1] << ", "
                  << data[i + 2] << ", " << data[i + 3] << ")" << std::endl;
        // cout << data[i] << ' ' << data[i + 1] << ' ';
    }
}