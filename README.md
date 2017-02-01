Features
-

OpenCL renderer running on GPU or CPU  
C++ renderer multithreaded with OpenMP running on CPU

Building
-

This project is developped on Windows with [CLion](https://www.jetbrains.com/clion/), which uses CMake as its build system.  
The compiler used is MinGW-w64 from [Msys2](https://msys2.github.io/) using GCC 5.3.0

To build, you will need these libraries:

* SDL 2.0.3
* SDL_image 2.0.1
* Assimp 3.2
* An OpenCL SDK from any vendors (Amd, Nvidia, Intel)

Additionally, the awesomely tiny [ImGui](https://github.com/ocornut/imgui), which is directly embedded in src/gui/imgui.  

