#include <FFTOcean/engine/engine.h>
#include <FFTOcean/object/ocean.h>
#include <string>
using namespace std;

int main()
{
    FFTOcean::Engine engine;

    FFTOcean::Ocean ocean(-0.5f, 0.0f, -0.5f, 512);
    engine.registerObject(&ocean);

    engine.run();

    return 0;
}