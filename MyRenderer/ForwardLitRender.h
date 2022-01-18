#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <Window.h>
#include <gBuffer.h>
#include <MainFramebuffer.h>
#include <Scene.h>
#include <PointLight.h>
#include <DirectionalLight.h>

class ForwardLitRender
{
public:
	ForwardLitRender();
	~ForwardLitRender();

	void render(gBuffer* _GBuffer, MainFramebuffer* main_fb, Scene* scene);

	void change_viewport_resolution(unsigned int x, unsigned int y);
	GLuint get_shader_program();

private:

	void init_program();
	void init_uniforms();
	void blit_depth_buffer(gBuffer* GBuffer, MainFramebuffer* fb);
	void set_uniforms(Scene* scene);
	void set_light_uniforms(Scene* scene);
	void set_projection_matrix(mat4 mat);
	void set_view_matrix(mat4 mat);
	void set_irradiance_map(GLuint id);
	void set_prefiltered_map(GLuint id);
	void set_brdf_lut(GLuint id);
	void set_max_reflection_lod(float id);
	void set_is_ibl_active(bool id);
	void set_viewer_position(vec3 vec);

	GLuint program;
	GLuint loc_viewer_pos;
	GLuint loc_projection_matrix;
	GLuint loc_view_matrix;
	GLuint loc_model_matrix;
	GLuint loc_irradiance_map;
	GLuint loc_prefiltered_map;
	GLuint loc_brdf_lut;
	GLuint loc_is_ibl_active;
	GLuint loc_max_reflection_lod;

	unsigned int width = WINDOW_WIDTH;
	unsigned int height = WINDOW_HEIGHT;

	unsigned int static_texture_uniform_count = -1;
	unsigned int texture_uniform_starting_point = 6;
	int point_light_count = 0;
	int direct_light_count = 0;
};
