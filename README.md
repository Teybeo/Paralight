Paralight
===

Real-time Path-tracing Renderer

Written in C++11 as personal learning project

Build with CMake, tested on Windows and Mac

### Features
- C++ renderer with OpenMP for multithreading (CPU only)
- OpenCL renderer (GPU or CPU)
- Monte-Carlo estimator with BRDF Importance Sampling
- BVH partitionning with Surface Area Heuristic (SAH)
- PBR shading model based on Lambert/CookTorrance for diffuse/specular
- Progressive accumulation
- Freefly and orbit camera
- On-the-fly OpenCL kernels editing

### Libraries used
- ImGui for user interface
- SDL2 for windowing and input
- SDL2_image for texture loading
- Assimp for model loading
- OpenCL C++ Bindings

------

![Paralight](https://user-images.githubusercontent.com/1454762/64714289-ac3c0480-d4be-11e9-9063-ef8e527a7e6f.png)
