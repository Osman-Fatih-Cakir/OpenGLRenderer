# OpenGLRenderer
An OpenGL renderer.

Note:
In this project, my goal is to build a renderer using OpenGL. I will try to learn and implement common realtime computer graphics techniques.
Since I am new to computer graphics, I would be happy to receive suggestions about the project and how I can improve myself :).

Techniques that I used so far:
- PBR (Cook-Torrance specular BRDF)
- Deferred shading
- Point light
- Directional light
- Shadows for all light types (shadow map)
- Load objects with assimp
- Forward rendering

## Material file read (Probably bad)
map_Ka	: Metallic map
map_Kd	: Albedo map
map_bump: Normal map
map_Ns 	: Roughness map
map_d 	: AO map

## Resources
- https://learnopengl.com/
## Models:
- https://www.cgtrader.com/free-3d-models/military/armor/mandalorian-helmet-e1903ae2-3218-43bc-a22c-a102284d230e
- https://www.sharetextures.com/textures/floor/tiling_46/

## Note:
There are memory leaks in project (which I assume caused by assimp library), I will fix it somehow soon
