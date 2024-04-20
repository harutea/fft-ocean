#include <FFTOcean/engine/engine.h>
#include <FFTOcean/object/ocean.h>
#include <FFTOcean/object/quad.h>
#include <string>
using namespace std;

int main()
{
    FFTOcean::Engine engine;

    FFTOcean::Quad quad("./shaders/initial_spectrum.comp", 0.0f, 0.0f, 0.0f, 64);
    engine.registerObject(&quad);

    engine.run();

    return 0;
}