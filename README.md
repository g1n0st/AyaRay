# AyaRay
Last update: 2020/4/16

**AyaRay** is a physically based Windows-platform renderer. AyaRay is written in modern C++ and integrates multi-threading and SIMD at the bottom to optimize high-performance computing. It includes many state of the art algorithms published in recent years in light transport simulation. The ultimate goal of AyaRay is to have a complete offline rendering procedure,  and provide the corresponding workflow for the artist.

Welcome to use any part of the code or the application in any place, but it should be warned that for now the code is just a demo. And it has no enough ability in any commercial occasions. If you have any issue, please post to the issue page and I will reply as early as I can.

## Demos

![san-miguel 2nd floor, 1280x800, 1024spp](https://img-blog.csdnimg.cn/20200411110204730.bmp)

![san-miguel 1-st floor, 1280x800, 1024spp](https://img-blog.csdnimg.cn/20200411110241784.bmp)

![point-light with bunny in cornell-box, 600x600, 200spp](https://img-blog.csdnimg.cn/20200411110446841.bmp)

## Build

Because the project is still building and need fast iteration, so it has not provide project file yet, you can include all files to build the current version.

## Compile switch

+ `AYA_DEBUG` debug option (off by default)
+ `AYA_USE_SIMD` Use SIMD / SEE instructions in the math library (on by default)
+ `AYA_SAMPLED_SPECTRUM`  Use Sampled Spectrum replace RGB Spectrum (off by default)
+ `AYA_USE_EMBREE` Replace default BVH to  Intel®  Embree BVH (default ver.2)
+ `AYA_USE_EMBREE_STATIC_LIB` Make Embree  provided as static lib (on by default)

## Features

### Integrators
+ Direct Lighting Integrator
+ Path Tracing
+ Bidirectional Path Tracing with MIS
+ Vertex Connection and Merging (Debugging)
+ Multiplex Metropolis Light Transport

### Materials
+ Bump Map
+ Texture Map
+ Alpha Test in texture
+ BSDFs
	+ Lambertian Diffuse
	+ Mirror (Smooth Conductor)
	+ Glass (Smooth Dielectric)
	+ Disney BRDF
	+ Rough Conductor/Dielectric


### Acceleration Structures
+ BVH
+ Intel®  Embree BVH (ver.2 / ver.3)


### Lights
+ Point Light
+ Area Light
+ Environmental Light
+ Spot Light
+ Directional Light

### Media
+ Homogenous

### Samplers
+ Independent Sampler
+ Sobol Sequence with Screen Space Index Enumeration

### Cameras
+ Perspective Camera
+ Motion blur
+ Custom lens shape
+ Vignette and Cat-eye effect

### Filters
+ Box Filter
+ Triangle Filter
+ Gaussian Filter
+ Mitchell Netravali Filter

### File Loaders
+ .Obj Model File Format
+ .Mtl File Format

### Math Library [AyaMath](https://github.com/g1n0st/AyaMath)

## Todo Lists (Planning schedule)

+ More Integrators
    + Stochastic Progressive Photon Mapping
	
+ Project Build based on CMake and Github Project dependency
+ IES Lighting support
+ More Cameras Model
	+ Environment Camera
	+ Orthographic Camera
+ BSSRDF
+ AyaGUI interface [AyaGUI](https://github.com/g1n0st/ayagui)
+ Preview interface based on GLSL Ray Tracing
+ OpenGL-based preview interface

+ User-defined scene import and export parser based on python syntax

+ Denoiser

+ Mesh Editor
+ Bezier rendering pipeline
