
#include <Renderer.h>
#include <gtc/matrix_transform.hpp>
#include <window.h>
#include <iostream>
#include <init_shaders.h>

// Constructor
Renderer::Renderer(Scene* _scene)
{
	scene = _scene;
	
	init();
	init_uniforms();
}

// Destructor
Renderer::~Renderer()
{
	// Deallocate memory allocations
	delete scene;
	delete GBuffer;
	delete dirDepth;
	delete pointDepth;
	delete forwardRender;
	delete deferredShading;
	delete main_fb;
	delete prev_fb;
	delete bloom;
}

// Renders the scene
void Renderer::render(float delta)
{
	// Total duration of the rendering
	time += (int)delta;
	
	//////////////////////////////////////////////////////
	for (int i = 0; i < 1; i++)
	{
		//scene->all_models[i]->rotate(vec3(0.f, 0.f, 1.f), (float)(4 * (i + 1)), delta / 100.f);
		//scene->all_models[i]->rotate(vec3(0.f, 1.f, 0.f), (float)(4 * (i + 1)), delta / 800.f);
		//scene->all_models[i]->translate(vec3(0.f, 0.f, 10.f), delta / 1000.f);
	}
	//////////////////////////////////////////////////////

	// TODO make sorting here instead of every draw call !!
	// Sort meshes of the models inside function
	// Make a distance ordered map of the models, call draws in order
	//sort_transparent(scene);

	//
	//// 1. GBuffer Pass: Generate geometry/color data into gBuffers
	//

	GBuffer->render(scene, total_frames);

	//
	//// Shadow pass: Get the depth map of the scene
	//

	// Directional light shadows
	dirDepth->render(scene);

	// Point light shadows
	pointDepth->render(scene);
	
	//
	//// 2: Calculate lighting pixel by pixel using gBuffer
	//

	deferredShading->render(GBuffer, main_fb, scene);

	//
	//// 2.1: Draw lit translucent meshes
	//

	// Draw scene
	forwardLitRender->render(GBuffer, main_fb, scene);

	//
	//// 3: Draw light meshes (The meshes that are not lit but in the same scene with other meshes)
	//

	forwardRender->render(scene, main_fb, GBuffer);

	//
	//// 4: Skybox rendering
	//

	// If scene has a skybox, render
	Skybox* skybox = scene->get_render_skybox();
	if (skybox != nullptr)
		scene->get_render_skybox()->render(scene->camera);
	
	//
	//// 5: Anti-aliasing
	//

	taa->render(main_fb, prev_fb, GBuffer);

	//
	//// 6: Post Processing
	//

	//bloom->render(main_fb);

	//
	//// 6. Render the scene after post process
	//

	render_all(main_fb->get_color_texture());

	// Store previous framebuffer
	store_previous_framebuffer();

	total_frames++;
}

// Initialize programs that are going to be used when rendering
void Renderer::init()
{
	// Initialize g-buffer program
	GBuffer = new gBuffer();
	// Set resolution
	GBuffer->set_gBuffer_resolution(WINDOW_WIDTH, WINDOW_HEIGHT);

	// Initialize depth map program
	dirDepth = new DirectionalDepth();

	// Initialize point depth
	pointDepth = new PointDepth();

	// Deferred shading program
	deferredShading = new DeferredShading();
	deferredShading->change_viewport_resolution(WINDOW_WIDTH, WINDOW_HEIGHT);

	// Forward Lit Renderer
	forwardLitRender = new ForwardLitRender();
	forwardLitRender->change_viewport_resolution(WINDOW_WIDTH, WINDOW_HEIGHT);

	// Forward render program
	forwardRender = new ForwardRender();
	forwardRender->change_viewport_resolution(WINDOW_WIDTH, WINDOW_HEIGHT);

	// Main framebuffer
	main_fb = new MainFramebuffer();
	prev_fb = new MainFramebuffer();

	// Bloom
	bloom = new Bloom();

	// Temporal anti aliasing
	taa = new TAA();
}

void Renderer::init_uniforms()
{
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/render_texture2D_vs.glsl");
	GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/render_texture2D_fs.glsl");
	render_program = initprogram(vertex_shader, fragment_shader);

	loc_texture = glGetUniformLocation(render_program, "image");
}

void Renderer::store_previous_framebuffer()
{
	// Clear previous framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, prev_fb->get_FBO());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Copy the depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, main_fb->get_FBO());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prev_fb->get_FBO());
	glBindFramebuffer(GL_READ_FRAMEBUFFER, main_fb->get_FBO());
	glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0,
		WINDOW_WIDTH, WINDOW_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0,
		WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// Clear new framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, main_fb->get_FBO());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Render the texture
void Renderer::render_all(GLuint texture)
{
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glUseProgram(render_program);
	// Write to default framebuffer to visualize the sceen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	set_texture(texture);

	render_quad();
}

void Renderer::set_texture(GLuint id)
{
	glUniform1i(loc_texture, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, id);
}

void Renderer::render_quad()
{
	unsigned int quadVAO = 0;
	unsigned int quadVBO;
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}