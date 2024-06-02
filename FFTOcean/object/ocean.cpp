#include "ocean.h"

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

Ocean::Ocean()
{
    initX = 0.0f;
    initY = 0.0f;
    initZ = 0.0f;
}

Ocean::Ocean(float _initX, float _initY, float _initZ, int _planeSize)
{
    initX = _initX;
    initY = _initY;
    initZ = _initZ;
    planeSize = _planeSize;
}

Ocean::~Ocean()
{
}

void Ocean::setup()
{
    cout << "Setup Ocean" << endl;
    
    // Initiailze Shaders
    this->initialComp = new ComputeShader("./shaders/initial_spectrum.comp");
    this->btComp = new ComputeShader("./shaders/butterfly_texture.comp");
    this->fcComp = new ComputeShader("./shaders/fourier_component.comp");
    this->copyImageComp = new ComputeShader("./shaders/copy_image.comp");
    this->ipComp = new ComputeShader("./shaders/inverse_and_permute.comp");
    this->butterflyComp = new ComputeShader("./shaders/butterfly.comp");

    this->shader = new Shader("./shaders/ocean.vert", "./shaders/ocean.frag");


    /* Initialize Textures for Compute Shader */
    glGenTextures(7, textures);
    for (int i = 0; i < 7; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, planeSize, planeSize, 0, GL_RGBA, GL_FLOAT, NULL);
        glBindImageTexture(i, textures[i], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    }

    /* Set Uniforms for Vertex Shader and Fragment Shader */
    shader->use();
    shader->setInt("heightMap", 6);
    shader->setFloat("texelSize", (float)1/(planeSize-1));
    shader->setVec3("material.ambient",0.0,0.231,0.38);
    shader->setVec3("material.diffuse", 0.,0.231,0.38);
    shader->setVec3("material.specular", 0.9f, 0.9f, 0.9f);
    shader->setFloat("material.shininess", 32.0f);

    /* Calculate Vertex Positions and Texture Coordinates */
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

    /*Vertex Position attribute */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    /* Texture Coordinate attribute */
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /* Dispatch Initial Spectrum Texture */
    initialComp->use();
    initialComp->setInt("N", planeSize);
    initialComp->setInt("L", 1000);
    initialComp->setFloat("A", 4);
    initialComp->setVec2("w", 1, 1);
    initialComp->setFloat("V", 40);
    glDispatchCompute((unsigned int)planeSize, (unsigned int)planeSize, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* Pass the bit reversed index to butterfly texture using SSBO */
    int n = planeSize;
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

    /* Check SSBO */
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    // int *ptr = (int *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    // for (int i = 0; i < 256; i++)
    // {
    //     cout << "Data[" << i << "] = " << ptr[i] << endl;
    // }
    // glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    checkGLError();

    /* Dispatch Butterfly Texture */
    btComp->use();
    btComp->setInt("N", planeSize);
    glDispatchCompute((unsigned int)log2(planeSize), (unsigned int)planeSize/16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    checkGLError();
}

void Ocean::render()
{
    /* Fourier Components */
    fcComp->use();
    fcComp->setInt("N", planeSize);
    fcComp->setInt("L", 1000);
    fcComp->setFloat("t", float(glfwGetTime()));
    glDispatchCompute((unsigned int)planeSize/16, (unsigned int)planeSize/16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* Copy Fourier Components to pingpong0 texture*/
    copyImageComp->use();
    glDispatchCompute((unsigned int)planeSize/16, (unsigned int)planeSize/16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* Dispatch Butterfly Shader */
    pingpong = 0;

    for (int i = 0; i < int(log2(planeSize)); i++)
    {
        butterflyComp->use();
        butterflyComp->setInt("direction", 0);
        butterflyComp->setInt("stage", i);
        butterflyComp->setInt("pingpong", pingpong);
        glDispatchCompute((unsigned int)planeSize/16, (unsigned int)planeSize/16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        pingpong = (pingpong + 1) % 2;
        // if (i == 7)
        // {
        //     checkTextureContent(textures[5], 256, 256);
        // }
    }
    for (int i = 0; i < int(log2(planeSize)); i++)
    {
        butterflyComp->use();
        butterflyComp->setInt("direction", 1);
        butterflyComp->setInt("stage", i);
        butterflyComp->setInt("pingpong", pingpong);
        glDispatchCompute((unsigned int)planeSize/16, (unsigned int)planeSize/16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        pingpong = (pingpong + 1) % 2;
    }

    /* Dispatch Inverse and Permute Shader */
    ipComp->use();
    ipComp->setInt("N", planeSize);
    ipComp->setInt("pingpong", pingpong);
    glDispatchCompute((unsigned int)planeSize/16, (unsigned int)planeSize/16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    /* Transforms */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, textures[6]);
    shader->use();

    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, glm::vec3(initX, initY, initZ));

    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    glm::vec3 lightPos(initX+0.5f, initY+0.5f, initZ+0.5f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    /* Set Uniforms */
    shader->setVec3("lightPos", lightPos);
    shader->setVec3("viewPos", cameraPos);
    shader->setVec3("lightColor", lightColor);
    shader->setFloat("time", glfwGetTime());

    shader->setMat4("model", model);
    shader->setMat3("normalMatrix", normalMatrix);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    /* Draw */
    glBindVertexArray(VAO);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, (planeSize - 1) * (planeSize - 1) * 2 * 3, GL_UNSIGNED_INT, 0);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Ocean::clear()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    // glDeleteBuffers(1, &EBO);
}

void Ocean::updateView(glm::mat4 _view)
{
    this->view = _view;
}

void Ocean::updateProjection(glm::mat4 _projection)
{
    this->projection = _projection;
}

void Ocean::updateCameraPos(glm::vec3 _cameraPos)
{
    this->cameraPos = _cameraPos;
}

void Ocean::checkGLError()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error: " << err << std::endl;
    }
}

void Ocean::checkTextureContent(GLuint texture, int width, int height)
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