#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <vector>
#include <gBuffer.h>
#include <Skybox.h>

typedef glm::mat4 mat4;
typedef glm::vec3 vec3;

class DeferredShading
{
public:

	DeferredShading();
	~DeferredShading();

	void start_program(gBuffer* _GBuffer);
	void change_viewport_resolution(unsigned int w, unsigned int h);
	void set_point_light(
		vec3 position,
		vec3 color,
		float radius,
		float linear,
		float quadratic,
		float _far,
		GLuint shadow_map,
		float intensity
	);
	void set_direct_light(
		vec3 color,
		vec3 direction,
		float intensity,
		GLuint shadow_map,
		mat4 light_space_matrix
	);
	void render(Camera* camera, Skybox* skybox);

private:

	void init_shaders();
	void get_uniform_locations();
	void init_quad();
	void draw_quad(GLuint shader_program);

	void set_viewer_pos(vec3 vec);
	void set_gPosition(GLuint id);
	void set_gNormal(GLuint id);
	void set_gAlbedoSpec(GLuint id);
	void set_gPbr_materials(GLuint id);
	void set_irradiance_map(GLuint id);
	void set_prefiltered_map(GLuint id);
	void set_brdf_lut(GLuint id);
	void set_max_reflection_lod(float val);
	void set_is_ibl_active(bool val);

	GLuint program;
	unsigned int width = 1024;
	unsigned int height = 1024;
	GLuint loc_viewer_pos;
	GLuint loc_gPosition;
	GLuint loc_gNormal;
	GLuint loc_gAlbedoSpec;
	GLuint loc_gPbr_materials;
	GLuint loc_irradiance_map;
	GLuint loc_prefiltered_map;
	GLuint loc_brdf_lut;
	GLuint loc_max_reflection_lod;
	GLuint loc_is_ibl_active;
	GLuint quad_VAO;
	int point_light_count = 0;
	int direct_light_count = 0;
	gBuffer* GBuffer = nullptr;
	unsigned int static_texture_uniform_count = -1;
};
