#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <Model.h>
#include <Camera.h>
#include <Window.h>
#include <Scene.h>

typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec2 vec2;

class gBuffer
{
public:

	gBuffer();
	~gBuffer();

	void attach_depthbuffer_to_framebuffer(GLuint framebuffer);
	void set_gBuffer_resolution(int w, int h);

	GLuint get_gPosition();
	GLuint get_gNormal();
	GLuint get_gAlbedoSpec();
	GLuint get_gPbr_materials();
	GLuint get_emissive();
	GLuint get_velocity();
	GLuint get_fbo();
	GLuint get_shader_program();

	unsigned int get_width();
	unsigned int get_height();
	void render(Scene* scene, unsigned int total_frames);

private:

	int gBuffer_width = WINDOW_WIDTH;
	int gBuffer_height = WINDOW_HEIGHT;
	GLuint program;
	GLuint gBuffer_fbo;
	GLuint loc_projection_matrix;
	GLuint loc_view_matrix;
	GLuint loc_prev_view_matrix;
	GLuint loc_model_matrix;
	GLuint loc_normal_matrix;
	GLuint loc_halton_sequence;
	GLuint loc_resolution;
	GLuint loc_total_frames;
	GLuint gPosition;
	GLuint gNormal;
	GLuint gAlbedoSpec;
	GLuint gPbr_materials;
	GLuint gEmissive;
	GLuint gVelocity;
	GLuint rbo_depth;
	vec2 halton_sequence[6];

	void init_shaders();
	void create_framebuffer();
	void get_uniform_locations();
	void init_halton_sequence();
	float create_halton_sequence(unsigned int index, int base);
	void render_model(Camera* camera, Model* model, unsigned int total_frames);
	void start_program();
	void set_projection_matrix(mat4 mat);
	void set_view_matrix(mat4 mat);
	void set_prev_view_matrix(mat4 mat);
	void set_halton_sequence();
	void set_resolution(int x, int y);
	void set_total_frames(unsigned int val);
};
