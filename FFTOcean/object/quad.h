#ifndef QUAD_H
#define QUAD_H

#include <FFTOcean/vector/vector.h>
#include <FFTOcean/renderer/compute_shader.h>
#include <FFTOcean/renderer/shader.h>
#include "object.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace FFTOcean
{
    class Quad : public Object
    {

    public:
        Quad();
        Quad(const char *_computeShaderSrc, float initX, float initY, float initZ, int OceanSize);
        ~Quad();
        void setup();
        void render();
        void clear();
        void updateView(glm::mat4 _view);
        void updateProjection(glm::mat4 _projection);
        void updateCameraPos(glm::vec3 _cameraPos);

    private:
        const unsigned int TEXTURE_WIDTH = 512, TEXTURE_HEIGHT = 512;
        unsigned int texture;
        int OceanSize;
        const char *computeShaderSrc;
    };

}

#endif