#ifndef OBJECT_H
#define OBJECT_H

#include <FFTOcean/vector/vector.h>
#include <FFTOcean/renderer/compute_shader.h>
#include <FFTOcean/renderer/shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h>

namespace FFTOcean
{
    class Object
    {
    public:
        Object();
        ~Object();
        virtual void setup() = 0;
        virtual void render() = 0;
        virtual void clear() = 0;
        virtual void updateView(glm::mat4 _view) = 0;
        virtual void updateProjection(glm::mat4 _projection) = 0;
        virtual void updateCameraPos(glm::vec3 _cameraPos) = 0;

    protected:
        unsigned int VAO;
        unsigned int VBO;
        unsigned int EBO;
        GLuint textures[7];
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec3 cameraPos;

        float initX, initY, initZ;
        Vector3 position;
        Vector3 velocity;
        Vector3 force;
        double mass;
        Shader *shader;
        ComputeShader *compShader0;
        ComputeShader *compShader1;
    };

}

#endif