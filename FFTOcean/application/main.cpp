#include <FFTOcean/engine/engine.h>
#include <FFTOcean/object/ocean.h>
#include <FFTOcean/object/quad.h>
#include <string>
using namespace std;

int main()
{
    FFTOcean::Engine engine;

    FFTOcean::Quad quad(0.0f, 0.0f, 0.0f);
    // FFTOcean::Ocean ocean(0, 0, 0, 512);
    engine.registerObject(&quad);
    // engine.registerObject(&ocean);

    engine.run();

    return 0;
}