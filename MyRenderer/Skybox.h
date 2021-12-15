#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

class Skybox
{
public:
	// Constrctor takes path to .hdr file
	Skybox(const char* path);

	// Getters
	GLuint get_equirectangular_map();
	GLuint get_skybox_map();
	GLuint get_irradiance_map();

private:

	void generate_skybox_map();
	void generate_irradiance_map();
	void load_hdr_file(const char* path);
	void render_cube();

	GLuint equirectangular_map;
	GLuint skybox_map;
	GLuint irradiance_map;
	GLuint cubeVAO = 0;
	GLuint cubeVBO = 0;
};
