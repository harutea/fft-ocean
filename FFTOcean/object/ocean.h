#ifndef OCEAN_H
#define OCEAN_H

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
    class Ocean : public Object
    {

    public:
        Ocean();
        Ocean(float _initX, float _initY, float _initZ, int _planeSize);

        ~Ocean();
        void setup();
        void render();
        void clear();
        void updateView(glm::mat4 _view);
        void updateProjection(glm::mat4 _projection);
        void updateCameraPos(glm::vec3 _cameraPos);
        void checkGLError();
        void checkTextureContent(GLuint texture, int width, int height);

    private:
        const unsigned int TEXTURE_WIDTH = 256, TEXTURE_HEIGHT = 256;
        int pingpong;
        ComputeShader *initialComp;
        ComputeShader *fcComp;
        ComputeShader *btComp;
        ComputeShader *butterflyComp;
        ComputeShader *ipComp;
        ComputeShader *copyImageComp;
        int planeSize;
    };

}

#endif