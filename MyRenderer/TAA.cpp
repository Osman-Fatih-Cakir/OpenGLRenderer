
#include <TAA.h>
#include <init_shaders.h>

TAA::TAA()
{
	init_shaders();

	get_uniform_locations();

    init_quad();
}

TAA::~TAA()
{

}

// Render
void TAA::render(MainFramebuffer* main_fb, MainFramebuffer* prev_fb, gBuffer* GBuffer)
{
    glUseProgram(taa_program);
    glViewport(0, 0, width, height);

    set_uniforms(main_fb, prev_fb, GBuffer);
    draw_quad();
}

void TAA::set_uniforms(MainFramebuffer* main_fb, MainFramebuffer* prev_fb, gBuffer* GBuffer)
{
    // Bind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, main_fb->get_FBO());

    set_resolution();
    set_cur_depth_map(main_fb->get_depth_texture());
    set_prev_depth_map(prev_fb->get_depth_texture());
    set_cur_color_map(main_fb->get_color_texture());
    set_prev_color_map(prev_fb->get_color_texture());
    set_velocity_map(GBuffer->get_velocity());
}

void TAA::set_resolution()
{
    float ar[] = { (float)width, (float)height };
    glUniform2fv(loc_resolution, 1, &ar[0]);
}

void TAA::set_cur_depth_map(GLuint map)
{
    glUniform1i(loc_cur_depth_map, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, map);
}

void TAA::set_prev_depth_map(GLuint map)
{
    glUniform1i(loc_prev_depth_map, 1);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, map);
}

void TAA::set_cur_color_map(GLuint map)
{
    glUniform1i(loc_cur_color_map, 2);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, map);
}

void TAA::set_prev_color_map(GLuint map)
{
    glUniform1i(loc_prev_color_map, 3);
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, map);
}

void TAA::set_velocity_map(GLuint map)
{
    glUniform1i(loc_velocity_map, 4);
    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_2D, map);
}

// Compile shaders
void TAA::init_shaders()
{
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/taa_vs.glsl");
	GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/taa_fs.glsl");
	taa_program = initprogram(vertex_shader, fragment_shader);
}

void TAA::get_uniform_locations()
{
	loc_resolution = glGetUniformLocation(taa_program, "resolution");
	loc_cur_depth_map = glGetUniformLocation(taa_program, "cur_depth_map");
	loc_prev_depth_map = glGetUniformLocation(taa_program, "prev_depth_map");
	loc_cur_color_map = glGetUniformLocation(taa_program, "cur_color_map");
	loc_prev_color_map = glGetUniformLocation(taa_program, "prev_color_map");
	loc_velocity_map = glGetUniformLocation(taa_program, "velocity_map");
}


// Initialize a quad
void TAA::init_quad()
{
    float quadVertices[] = {

        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 2
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 4
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // 3

        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 2
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // 3
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f // 1
    };

    glGenVertexArrays(1, &quad_VAO);

    // Setup quad VAO
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quad_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

// Draw everything on a quad as a texture
void TAA::draw_quad()
{
    glBindVertexArray(quad_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
