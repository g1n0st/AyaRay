# AyaRay
Last update: 2020/3/20

g1n0st

**AyaRay** is a physically based cross-platform renderer  developing by [Chang Yu]( https://github.com/g1n0st ),  for the purpose of learning global illumination and Ray Tracing. AyaRay is written in modern C ++ and integrates multi-threading and SIMD at the bottom to optimize high-performance computing. The ultimate goal of AyaRay is to have a complete offline rendering procedure,  and provide the corresponding workflow for the artist.

Welcome to use any part of the code or the application in any place, but it should be warned that for now the code is just a demo. And it has no enough ability in any commercial occasions.



## Build

Because the project is still building and need fast iteration, so it has not provide project file yet, you can include all files to build the current version.

## Compile switch

+ **AYA_DEBUG** debug option (off by default)
+ **AYA_USE_SIMD** Use SIMD / SEE instructions in the math library (on by default)
+ **AYA_SAMPLED_SPECTRUM**  Use Sampled Spectrum replace RGB Spectrum (off by default)

## Features

### Integrators
+ Direct Lighting Integrator
+ Path Tracing

### Materials
+ Bump Map
+ Texture Map
+ BSDFs
	+ Lambertian Diffuse
	+ Mirror (Smooth Conductor)
	+ Glass (Smooth Dielectric)


### Acceleration Structures
+ BVH


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
    + Bidirectional Path Tracing
	+ Photon Mapping
	+ Stochastic Progressive Photon Mapping
	+ Metropolis Light Transport

+ Project Build based on CMake and Github Project dependency
+ Cross-platform Test

+ IES Lighting support
+ More Cameras Model
	+ Environment Camera
	+ Orthographic Camera
+ Rough Conductor/Dielectric
+ Disney BSDF
+ BSSRDF
+ Alpha Test

+ AyaGUI interface [AyaGUI](https://github.com/g1n0st/ayagui)
+ Preview interface based on GLSL Ray Tracing
+ OpenGL-based preview interface

+ User-defined scene import and export parser based on python syntax

+ Denoiser

+ Mesh Editor
+ Bezier rendering pipeline