#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <Camera.h>

class Skybox
{
public:
	// Constrctor takes path to .hdr file
	Skybox(const char* path, int id, bool _ibl);

	void render(Camera* camera);
	void toggle_IBL();

	// Getters
	GLuint get_equirectangular_map();
	GLuint get_skybox_map();
	GLuint get_irradiance_map();
	GLuint get_prefiltered_map();
	GLuint get_brdf_lut();
	GLuint get_width();
	GLuint get_height();
	bool is_ibl_active();
	unsigned int get_max_mip_level();
	int get_id();
	
private:
	Skybox();

	void generate_skybox_map();
	void generate_irradiance_map();
	void generate_prefilter_map();
	void generate_skybox_mipmaps();
	void generate_brdf_lut();

	void get_uniform_location();
	void load_hdr_file(const char* path);
	void render_cube();
	void render_quad();
	void init_shader();

	void create_skybox_map();
	void create_irradiance_map();
	void create_prefilter_map();
	void create_brdf_lut();
	void create_framebuffer();

	void set_view_matrix(mat4 mat);
	void set_projection_matrix(mat4 mat);
	void set_skybox_map();

	int id = -1;
	GLuint equirectangular_map;
	GLuint skybox_map;
	GLuint irradiance_map;
	GLuint prefilter_map;
	GLuint brdf_lut;

	GLuint program; // TODO rename program->skybox_program
	GLuint render_program;
	GLuint irradiance_program;
	GLuint prefilter_program;
	GLuint brdf_program;

	GLuint captureFBO;
	GLuint captureRBO;

	GLuint loc_projection_matrix;
	GLuint loc_view_matrix;
	GLuint loc_skybox_map;
	
	bool IBL = true;
	unsigned int max_mip_level = 5;

	GLuint width = 512;
	GLuint height = 512;
	GLuint irradiance_width = 32;
	GLuint irradiance_height = 32;
	GLuint prefilter_width = 128;
	GLuint prefilter_height = 128;
};
