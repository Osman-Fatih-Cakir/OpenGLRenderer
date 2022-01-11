
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
		//scene->all_models[i]->rotate(vec3(0.f, 1.f, 0.f), (float)(4 * (i + 1)), delta / 100.f);
		//scene->all_models[i]->translate(vec3(0.f, 0.f, 10.f), delta / 1000.f);
	}
	//////////////////////////////////////////////////////

	//
	//// 1. GBuffer Pass: Generate geometry/color data into gBuffers
	//

	GBuffer->start_program();

	// Draw models of the scene
	for (unsigned int i = 0; i < scene->all_models.size(); i++)
	{
		// Draw model
		GBuffer->render(scene->camera, scene->all_models[i]);
	}

	//
	//// Shadow pass: Get the depth map of the scene
	//

	// Directional shadows
	for (unsigned int i = 0; i < scene->direct_lights.size(); i++)
	{
		// Check if the light casts shadow
		if (!scene->direct_lights[i]->does_cast_shadow())
			continue;

		dirDepth->start_program(scene->direct_lights[i]);

		// Draw scene
		for (unsigned int ii = 0; ii < scene->all_models.size(); ii++)
		{
			// Draw
			dirDepth->render(scene->all_models[ii]);
		}
	}

	// Point light shadows
	for (unsigned int i = 0; i < scene->point_lights.size(); i++)
	{
		// Check if the light casts shadow
		if (!scene->point_lights[i]->does_cast_shadow())
			continue;

		pointDepth->start_program(scene->point_lights[i]);

		// Draw scene
		for (unsigned int ii = 0; ii < scene->all_models.size(); ii++)
		{
			// Draw
			pointDepth->render(scene->all_models[ii]);
		}
	}

	//
	//// 2: Calculate lighting pixel by pixel using gBuffer
	//

	deferredShading->start_program(GBuffer, main_fb);

	// Point lights
	for (unsigned int i = 0; i < scene->point_lights.size(); i++)
	{
		PointLight* light = scene->point_lights[i];
		deferredShading->set_point_light(
			light->position,
			light->color,
			light->radius,
			light->cutoff,
			light->half_radius,
			light->linear,
			light->quadratic,
			light->shadow_projection_far,
			light->depth_cubemap,
			light->intensity,
			light->does_cast_shadow()
		);
	}
	// Directional lights
	for (unsigned int i = 0; i < scene->direct_lights.size(); i++)
	{
		DirectionalLight* light = scene->direct_lights[i];
		deferredShading->set_direct_light(
			light->get_color(),
			light->get_direction(),
			light->intensity,
			light->get_depth_map(),
			light->get_space_matrix(),
			light->does_cast_shadow()
		);
	}
	
	// Render with deferred shading
	deferredShading->render(scene->camera, scene->get_render_skybox());

	//
	//// 3: Draw light meshes (The meshes that are not lit but in the same scene with other meshes)
	//

	// Start forward rendering program
	forwardRender->start_program(GBuffer, main_fb);
	forwardRender->change_viewport_resolution(WINDOW_WIDTH, WINDOW_HEIGHT);

	// Draw scene
	for (unsigned int i = 0; i < scene->point_lights.size(); i++)
	{
		if (scene->point_lights[i]->model != nullptr)
		{
			forwardRender->set_color(scene->point_lights[i]->color);
			forwardRender->render(scene->camera, scene->point_lights[i]->model);
		}

		if (scene->point_lights[i]->is_debug_active())
		{
			forwardRender->set_color(scene->point_lights[i]->color);
			// Light radius renders wih wireframe
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			forwardRender->render(scene->camera, scene->point_lights[i]->debug_model);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	//
	//// 4: Skybox rendering
	//

	Skybox* skybox = scene->get_render_skybox();
	if (skybox != nullptr)
		scene->get_render_skybox()->render(scene->camera);
	
	//
	//// 5: Post Processing
	//

	bloom->start(main_fb->get_FBO(),  main_fb->get_color_texture());

	//
	//// 6. Render the scene after post process
	//

	render_all(main_fb->get_color_texture());

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

	// Forward render program
	forwardRender = new ForwardRender();
	forwardRender->change_viewport_resolution(WINDOW_WIDTH, WINDOW_HEIGHT);

	// Main framebuffer
	main_fb = new MainFramebuffer();

	// Bloom
	bloom = new Bloom();
}

void Renderer::init_uniforms()
{
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/render_texture2D_vs.glsl");
	GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/render_texture2D_fs.glsl");
	render_program = initprogram(vertex_shader, fragment_shader);

	loc_texture = glGetUniformLocation(render_program, "image");
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