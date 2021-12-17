# OpenGLRenderer
An OpenGL renderer.

Note:
In this project, my goal is to build a renderer using OpenGL. I will try to learn and implement common realtime computer graphics techniques.
Since I am new to computer graphics, I would be happy to receive suggestions about the project and how I can improve myself :).

Techniques that I used so far:
- PBR (Cook-Torrance specular BRDF)
- IBL
	- Diffuse irradiance
- Deferred shading
- Point light
- Directional light
- Shadows for all light types (shadow map)
- Load objects with assimp
- Forward rendering
- Skybox (.hdr files)

## Material file read (Probably bad)
- map_Ka	: Metallic map
- map_Kd	: Albedo map
- map_bump: Normal map
- map_Ns 	: Roughness map
- map_d 	: AO map

## Resources
- https://learnopengl.com/
- https://www.youtube.com/c/TheChernoProject
## Models:
- https://www.cgtrader.com/free-3d-models/military/armor/mandalorian-helmet-e1903ae2-3218-43bc-a22c-a102284d230e
- https://www.sharetextures.com/textures/floor/tiling_46/
## HDR files
- https://polyhaven.com/a/museum_of_ethnography
- https://polyhaven.com/a/entrance_hall
- https://polyhaven.com/a/veranda

## Note:
There are memory leaks in project (which I assume caused by assimp library), I found no way to fix it. I will check that later for now since it cost me so much time