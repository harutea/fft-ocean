#include <FFTOcean/engine/engine.h>
#include <FFTOcean/object/ocean.h>
#include <FFTOcean/object/quad.h>
#include <string>
using namespace std;

int main()
{
    FFTOcean::Engine engine;

    FFTOcean::Quad quad(0.0f, 0.0f, 0.0f);
    engine.registerObject(&quad);

    engine.run();

    return 0;
}