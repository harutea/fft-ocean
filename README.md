# fft-ocean
Ocean Wave Simulation using FFT

## Clone
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
- [Flügge F.-J. 2017. Realtime GPGPU FFT Ocean Water Simulation, Institute of Embedded Systems.](https://tore.tuhh.de/bitstream/11420/1439/1/GPGPU_FFT_Ocean_Simulation.pdf)
