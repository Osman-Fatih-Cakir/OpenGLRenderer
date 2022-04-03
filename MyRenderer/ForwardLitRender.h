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

	void render(gBuffer* _GBuffer, MainFramebuffer* main_fb, Scene* scene, unsigned int total_frames);

	void change_viewport_resolution(unsigned int x, unsigned int y);
	GLuint get_shader_program();

private:

	void init_program();
	void init_uniforms();
	void blit_depth_buffer(gBuffer* GBuffer, MainFramebuffer* fb);
	void set_uniforms(Scene* scene, unsigned int total_frames);
	void set_light_uniforms(Scene* scene);
	void set_projection_matrix(mat4 mat);
	void set_view_matrix(mat4 mat);
	void set_irradiance_map(GLuint id);
	void set_prefiltered_map(GLuint id);
	void set_brdf_lut(GLuint id);
	void set_max_reflection_lod(float id);
	void set_is_ibl_active(bool id);
	void set_viewer_position(vec3 vec);
	void set_prev_view_matrix(mat4 mat);
	void set_halton_sequence();
	void set_resolution(int w, int h);
	void set_total_frames(unsigned int val);
	void init_halton_sequence();
	float create_halton_sequence(unsigned int index, int base);

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
	GLuint loc_point_light_count;
	GLuint loc_direct_light_count;
	GLuint loc_prev_view_matrix;
	GLuint loc_halton_sequence;
	GLuint loc_resolution;
	GLuint loc_total_frames;

	unsigned int width = WINDOW_WIDTH;
	unsigned int height = WINDOW_HEIGHT;

	unsigned int static_texture_uniform_count = -1;
	unsigned int texture_uniform_starting_point = 5;
	int point_light_count = 0;
	int direct_light_count = 0;
	int max_plight_per_call = 32;
	int max_dlight_per_call = 4;

	vec2 halton_sequence[6];
};
