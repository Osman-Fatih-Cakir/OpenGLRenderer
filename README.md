# OpenGLRenderer
An OpenGL renderer.

### Note:
In this project, my goal is to build a renderer using OpenGL. I will try to learn and implement common realtime computer graphics techniques.
Since I am new to computer graphics, I would be happy to receive suggestions about the project and how I can improve myself :).

## Screenshots
<img src="./Outputs/pbr1.gif" width="800" height="450" />  
<img src="./Outputs/ibl3.gif" width="800" height="450" />  
<img src="./Outputs/ds3.PNG" width="800" height="450" />  
<img src="./Outputs/sponza2.png" width="800" height="450" />  

## Features:
- PBR (Cook-Torrance specular BRDF)
- IBL
	- Diffuse irradiance
	- Specular
- Deferred shading
- Point light
- Directional light
- Shadows for all light types (shadow map)
- Load objects with assimp (GLTF)
- Forward rendering
- Skybox (.hdr files)
- Emission
- Post Process
	- Bloom
- Transparent rendering

## Material files pipeline
- Albedo map
- Normal map (optional)
- Metallic Roughness map (Red: Metallic, Green: Roughness, Blue: None)
- AO map (optional)

## Prerequisites
- OpenGL 4.5
- Visual Studio 19 (I haven't tried old versions of Visual Studio)

## Resources
- https://learnopengl.com/
- https://www.youtube.com/c/TheChernoProject

## Models:
- https://www.artstation.com/artwork/battle-damaged-sci-fi-helmet-pbr


## HDR Files
- https://polyhaven.com/a/museum_of_ethnography
- https://polyhaven.com/a/entrance_hall
- https://polyhaven.com/a/veranda
- https://polyhaven.com/a/dikhololo_night
- https://polyhaven.com/a/dreifaltigkeitsberg

## Note:
There are memory leaks in project (which I assume caused by assimp library), I found no way to fix it. I will check that later since it has cost me so much time.
