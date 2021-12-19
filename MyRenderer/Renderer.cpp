
#include <Renderer.h>
#include <gtc/matrix_transform.hpp>
#include <window.h>
#include <iostream>

// Constructor
Renderer::Renderer(Scene* _scene)
{
	scene = _scene;
	

	init_shader_programs();
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
}

// Renders the scene
void Renderer::render(float delta)
{
	// Total duration of the rendering
	time += (int)delta;
	
	//////////////////////////////////////////////////////
	for (int i = 0; i < 1; i++)
	{
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

	// Attach depth buffer to default framebuffer
	GBuffer->attach_depthbuffer_to_framebuffer(0);

	// Attach default framebuffer for no prevent further modifications
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
	//// 2. Lighting Pass: Calculate lighting pixel by pixel using gBuffer
	//

	deferredShading->start_program(GBuffer);

	// Point lights
	for (unsigned int i = 0; i < scene->point_lights.size(); i++)
	{
		PointLight* light = scene->point_lights[i];
		deferredShading->set_point_light(
			light->position,
			light->color,
			light->radius,
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
	//// 3. Pass: Draw light meshes (The meshes that are not lit but in the same scene with other meshes)
	//

	// Start forward rendering program
	forwardRender->start_program(GBuffer);
	forwardRender->change_viewport_resolution(WINDOW_WIDTH, WINDOW_HEIGHT);

	// Draw scene
	for (unsigned int i = 0; i < scene->point_lights.size(); i++)
	{
		if (scene->point_lights[i]->model != nullptr)
		{
			forwardRender->set_model_matrix(scene->point_lights[i]->get_model_matrix());
			forwardRender->set_color(scene->point_lights[i]->color);
			forwardRender->render(scene->camera, scene->point_lights[i]->model);
		}

		if (scene->point_lights[i]->is_debug_active())
		{
			forwardRender->set_model_matrix(scene->point_lights[i]->get_debug_model_matrix());
			forwardRender->set_color(scene->point_lights[i]->color);
			// Light radius renders wih wireframe
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			forwardRender->render(scene->camera, scene->point_lights[i]->debug_model);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	// Skybox rendering
	Skybox* skybox = scene->get_render_skybox();
	if (skybox != nullptr)
		scene->get_render_skybox()->render(scene->camera);
	
	// Error check
	GLuint err = glGetError(); if (err) fprintf(stderr, "ERROR: %s\n", gluErrorString(err));

	glutSwapBuffers();

	glutPostRedisplay(); // Render loop
}

// Initialize programs that are going to be used when rendering
void Renderer::init_shader_programs()
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
}
