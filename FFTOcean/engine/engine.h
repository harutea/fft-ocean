#ifndef ENGINE_H
#define ENGINE_H

#include <FFTOcean/renderer/renderer.h>
#include <FFTOcean/object/object.h>

namespace FFTOcean
{

    class Engine
    {
    private:
        Renderer *renderer;

    public:
        Engine();
        ~Engine();
        void init();
        void run();
        void registerObject(Object *object);
    };

}

#endif ENGINE_H