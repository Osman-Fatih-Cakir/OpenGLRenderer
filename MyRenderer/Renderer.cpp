
#include <Renderer.h>
#include <gtc/matrix_transform.hpp>
#include <window.h>

// Constructor
Renderer::Renderer(Scene* _scene)
{
	scene = _scene;
	timer = new Timer();

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
	delete timer;
}

// Renders the scene
void Renderer::render()
{
	// Get delta time
	float delta = timer->get_delta_time();

	//
	//// 1. GBuffer Pass: Generate geometry/color data into gBuffers
	//

	//for (int i = 0; i < 2; i++)
	//	scene->all_models[i]->rotate(vec3(0.f, 1.f, 0.f), delta / (50*(i+1)));

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
		deferredShading->set_point_light(
			scene->point_lights[i]->position,
			scene->point_lights[i]->color,
			scene->point_lights[i]->radius,
			scene->point_lights[i]->linear,
			scene->point_lights[i]->quadratic,
			scene->point_lights[i]->shadow_projection_far,
			scene->point_lights[i]->depth_cubemap,
			scene->point_lights[i]->intensity
		);
	}
	// Directional lights
	for (unsigned int i = 0; i < scene->direct_lights.size(); i++)
	{
		deferredShading->set_direct_light(
			scene->direct_lights[i]->color,
			scene->direct_lights[i]->direction,
			scene->direct_lights[i]->intensity,
			scene->direct_lights[i]->depth_map,
			scene->direct_lights[i]->space_matrix
		);
	}
	
	// Render with deferred shading
	deferredShading->render(scene->camera);

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
			forwardRender->set_model_matrix(scene->point_lights[i]->model->get_model_matrix());
			forwardRender->set_color(scene->point_lights[i]->color);
			forwardRender->render(scene->camera, scene->point_lights[i]->model);
		}
	}
	
	// Error check
	GLuint err = glGetError(); if (err) fprintf(stderr, "%s\n", gluErrorString(err));

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
