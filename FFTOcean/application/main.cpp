#include <FFTOcean/engine/engine.h>
#include <FFTOcean/object/ocean.h>
#include <FFTOcean/object/quad.h>

int main()
{
    FFTOcean::Engine engine;

    FFTOcean::Quad quad(0.0f, -2.0f, 0.0f, 64);
    engine.registerObject(&quad);

    engine.run();

    return 0;
}