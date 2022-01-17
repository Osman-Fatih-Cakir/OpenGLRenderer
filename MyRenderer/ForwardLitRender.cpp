
#include <ForwardLitRender.h>
#include <init_shaders.h>

// Constructor
ForwardLitRender::ForwardLitRender()
{
	// Create program
	init_program();

	// Initialize uniforms
	init_uniforms();
}
// Destructor
ForwardLitRender::~ForwardLitRender()
{

}

// Renders the scene
void ForwardLitRender::render(gBuffer* GBuffer, MainFramebuffer* main_fb, Scene* scene)
{
	blit_depth_buffer(GBuffer, main_fb);

	glUseProgram(program);

	glViewport(0, 0, width, height);

	// Set camera attributes
	set_projection_matrix(scene->camera->get_projection_matrix());
	set_view_matrix(scene->camera->get_view_matrix());

	// Draw call for each object (forward rendering)
	std::vector<Model*>::iterator itr;
	for (itr = scene->translucent_models.begin(); itr != scene->translucent_models.end(); itr++)
	{
		(*itr)->draw(program);
	}

	glBindVertexArray(0);
}

// Change viewport resolution
void ForwardLitRender::change_viewport_resolution(unsigned int x, unsigned int y)
{
	width = x;
	height = y;
}

void ForwardLitRender::set_projection_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, &mat[0][0]);
}

void ForwardLitRender::set_view_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, &mat[0][0]);
}

// Getters and setters
GLuint ForwardLitRender::get_shader_program()
{
	return program;
}

// Compile shaders and create programs
void ForwardLitRender::init_program()
{
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/forward_lit_render_vs.glsl");
	GLint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/forward_lit_render_fs.glsl");
	program = initprogram(vertex_shader, fragment_shader);
}

// Initialize uniforms
void ForwardLitRender::init_uniforms()
{
	// Get uniform locaitons
	loc_projection_matrix = glGetUniformLocation(program, "projection_matrix");
	loc_view_matrix = glGetUniformLocation(program, "view_matrix");
	// TODO after fragment shader implementation
}

// Blit depth buffer of gBuffer
void ForwardLitRender::blit_depth_buffer(gBuffer* GBuffer, MainFramebuffer* fb)
{
	// Attach depth buffer to default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, GBuffer->get_fbo());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb->get_FBO()); // Write to default framebuffer
	glBlitFramebuffer(0, 0, GBuffer->get_width(), GBuffer->get_height(), 0, 0,
		GBuffer->get_width(), GBuffer->get_height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}
