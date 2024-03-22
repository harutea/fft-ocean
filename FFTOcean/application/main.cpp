#include <FFTOcean/engine/engine.h>
#include <FFTOcean/object/ocean.h>

int main()
{
    FFTOcean::Engine engine;

    FFTOcean::Ocean ocean(0.0f, -3.0f, 0.0f, 64);
    engine.registerObject(&ocean);

    engine.run();

    return 0;
}