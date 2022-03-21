#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <Model.h>
#include <DirectionalLight.h>
#include <Scene.h>

typedef glm::mat4 mat4;

class DirectionalDepth
{
public:

	DirectionalDepth();
	~DirectionalDepth();

	void start_program(DirectionalLight* _light);
	GLuint get_shader_program();
	void set_space_matrix(mat4 mat);
	void render(Scene* scene);

private:

	GLuint program;
	GLuint loc_space_matrix;
	GLuint loc_model_matrix;
	DirectionalLight* light = nullptr;

	void init_shaders();
	void get_uniform_locations();
	void render_model(Model* model, vec3 cam_pos);
};
