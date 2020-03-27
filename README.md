# About

This is the Dynon fork oy the lpc21isp project the original project is no longer maintained

## Requirements
  - cmake
  - gcc
  - git

## Gotchas

If you intend to use lpc21isp on en embedded device e.p. raspberry pi 3 compute module it needs to be compiled on that platform or cross compiled for that platform.

## Building lpc21isp on D30 platform

1. install all the required tools on the D30 `sudo apt-get update && sudo apt-get install cmake gcc git`
2. clone the project repository `git clone https://github.com/DynonAvionics/lpc21isp.git`
3. move into the project `cd lp21isp`
4. create an out of source build directory `mkdir _build`
5. move into that directory `cd _build`
6. invoke cmake with GPIO_SUPPORT `cmake .. -DGPIO_SUPPORT=ON`
7. build lpc21isp `make all -j5`
