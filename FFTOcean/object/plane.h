#ifndef PLANE_H
#define PLANE_H

#include <FFTOcean/vector/vector.h>
#include <FFTOcean/renderer/shader.h>
#include "object.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace FFTOcean
{
    class Plane : public Object
    {

    public:
        Plane();
        Plane(float initX, float initY, float initZ, int planeSize);
        ~Plane();
        void setup();
        void render();
        void clear();
        void updateView(glm::mat4 _view);
        void updateProjection(glm::mat4 _projection);
        void updateCameraPos(glm::vec3 _cameraPos);

    private:
        int planeSize;
    };

}

#endif