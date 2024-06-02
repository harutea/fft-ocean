#include <FFTOcean/engine/engine.h>
#include <FFTOcean/object/ocean.h>
#include <FFTOcean/object/quad.h>
#include <FFTOcean/object/plane.h>
#include <string>
using namespace std;

int main()
{
    FFTOcean::Engine engine;

    FFTOcean::Quad quad(0.0f, -0.5f, 0.0f, 512);
    // FFTOcean::Plane plane(0, 0, 0, 512);
    engine.registerObject(&quad);
    // engine.registerObject(&plane);

    engine.run();

    return 0;
}