
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <Window.h>

class Bloom
{
public:

	Bloom();
	~Bloom();
	
	void start(GLuint main_fb, GLuint texture);

private:


	void take_hdr_image(GLuint texture);
	void blur();
	void converge(GLuint main_fb);

	void create_framebuffers();
	void init_shader();
	void render_quad();
	void get_uniform_locations();

	void set_hdr_image(GLuint id);
	void set_blur_image(GLuint id);
	void set_horizontal(bool val);
	void set_conv_img1(GLuint id);
	void set_conv_img2(GLuint id);

	GLuint hdr_program;
	GLuint blur_program;
	GLuint converge_program;

	GLuint hdrFBO;
	GLuint color_buffers[2];
	GLuint pingpongFBO[2];
	GLuint pingpong_textures[2];

	GLuint loc_hdr_image;
	GLuint loc_horizontal;
	GLuint loc_blur_image;
	GLuint loc_conv_img1;
	GLuint loc_conv_img2;

	unsigned int width = WINDOW_WIDTH;
	unsigned int height = WINDOW_HEIGHT;
	unsigned int blur_amount = 10;
};