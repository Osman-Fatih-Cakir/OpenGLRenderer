#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <Model.h>
#include <gBuffer.h>
#include <MainFramebuffer.h>
#include <Scene.h>

typedef glm::mat4 mat4;
typedef glm::vec3 vec3;

class ForwardRender
{
public:

	ForwardRender();
	~ForwardRender();

	void start_program(gBuffer* _GBuffer, MainFramebuffer* fb);
	void change_viewport_resolution(unsigned int w, unsigned int h);
	GLuint get_shader_program();
	void set_projection_matrix(mat4 mat);
	void set_view_matrix(mat4 mat);
	void set_color(vec3 vec);
	void render(Scene* scene, MainFramebuffer* main_fb, gBuffer* GBuffer);

private:

	GLuint program;
	GLuint width = 1024;
	GLuint height = 1024;
	GLuint loc_projection_matrix;
	GLuint loc_view_matrix;
	GLuint loc_model_matrix;
	GLuint loc_color;

	gBuffer* GBuffer = nullptr;
	MainFramebuffer* main_framebuffer = nullptr;

	void init_shaders();
	void get_uniform_locations();
	void render_model(Camera* camera, Model* model);
};
