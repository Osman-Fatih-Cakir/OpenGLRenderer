#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <vector>
#include <gBuffer.h>
#include <Skybox.h>
#include <Window.h>
#include <MainFramebuffer.h>
#include <Scene.h>

typedef glm::mat4 mat4;
typedef glm::vec3 vec3;

class DeferredShading
{
public:

	DeferredShading();
	~DeferredShading();

	void change_viewport_resolution(unsigned int w, unsigned int h);
	void render(gBuffer* _GBuffer, MainFramebuffer* fb, Scene* scene);

private:

	void init_shaders();
	void get_uniform_locations();
	void init_quad();
	void set_uniforms(Scene* scene, gBuffer* GBuffer);
	void set_light_uniforms(Scene* scene);
	void draw_quad(GLuint shader_program);

	void set_viewer_pos(vec3 vec);
	void set_gPosition(GLuint id);
	void set_gNormal(GLuint id);
	void set_gAlbedoSpec(GLuint id);
	void set_gPbr_materials(GLuint id);
	void set_irradiance_map(GLuint id);
	void set_prefiltered_map(GLuint id);
	void set_brdf_lut(GLuint id);
	void set_emissive(GLuint id);
	void set_max_reflection_lod(float val);
	void set_is_ibl_active(bool val);

	GLuint program;

	unsigned int width = WINDOW_WIDTH;
	unsigned int height = WINDOW_HEIGHT;

	GLuint loc_viewer_pos;
	GLuint loc_gPosition;
	GLuint loc_gNormal;
	GLuint loc_gAlbedoSpec;
	GLuint loc_gPbr_materials;
	GLuint loc_emissive;
	GLuint loc_irradiance_map;
	GLuint loc_prefiltered_map;
	GLuint loc_brdf_lut;
	GLuint loc_max_reflection_lod;
	GLuint loc_is_ibl_active;
	GLuint loc_point_light_count;
	GLuint loc_direct_light_count;

	GLuint quad_VAO;
	GLuint quadVBO;

	int point_light_count = 0;
	int direct_light_count = 0;

	unsigned int static_texture_uniform_count = -1;
	unsigned int texture_uniform_starting_point = 5;
	int max_plight_per_call = 32;
	int max_dlight_per_call = 4;
};
