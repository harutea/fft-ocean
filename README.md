# fft-ocean
Ocean Wave Simulation using FFT

## Clone with submodules
```bash
git clone --recursive https://github.com/harutea/fft-ocean.git
```
Or,
```bash
git clone https://github.com/harutea/fft-ocean.git
cd fft-ocean
git submodule update --recursive --init
```

## How to build
```bash
mkdir build
cd build
cmake .. -G "Unix Makefiles"
make
```

## References
- [Tessendorf, Jerry. 2001. Simulating Ocean Water. SIG-GRAPH'99 Course Note.](https://www.researchgate.net/publication/264839743_Simulating_Ocean_Water)
- [Flügge F.-J. 2017. Realtime GPGPU FFT Ocean Water Simulation, Institute of Embedded Systems.](https://tore.tuhh.de/bitstream/11420/1439/1/GPGPU_FFT_Ocean_Simulation.pdf)
