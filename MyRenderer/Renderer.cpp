
#include <Renderer.h>

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

	// Start gBuffer program
	GBuffer->start_program();

	scene->camera->camera_rotate(vec3(0.f, 1.f, 0.f), delta / 10000); // Camera rotation smoothly
	// Set camera attributes
	GBuffer->set_projection_matrix(scene->camera->get_projection_matrix());
	GBuffer->set_view_matrix(scene->camera->get_view_matrix());

	// Draw scene
	for (int i = 0; i < scene->all_meshes.size(); i++)
	{
		// Set model attributes
		GBuffer->set_model_matrix(scene->all_meshes[i]->get_model_matrix());
		GBuffer->set_normal_matrix(scene->all_meshes[i]->get_normal_matrix());
		GBuffer->set_diffuse_texture(scene->all_meshes[i]->get_texture_id());
		// Draw
		GBuffer->render(scene->all_meshes[i]->get_VAO(), scene->all_meshes[i]->get_triangle_count() * 3);
	}

	// Attach depth buffer to default framebuffer
	GBuffer->attach_depthbuffer_to_framebuffer(0);

	// Attach default framebuffer for no prevent further modifications
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//
	//// Shadow pass: Get the depth map of the scene
	//

	// Directional shadows
	dirDepth->start_program();
	for (int i = 0; i < scene->direct_lights.size(); i++)
	{
		glViewport(0, 0, scene->direct_lights[i]->depth_map_width, scene->direct_lights[i]->depth_map_height);
		glBindFramebuffer(GL_FRAMEBUFFER, scene->direct_lights[i]->depth_map_fbo);
		glClear(GL_DEPTH_BUFFER_BIT);

		// Set space matrix
		dirDepth->set_space_matrix(scene->direct_lights[i]->space_matrix);
		// Draw scene
		for (int ii = 0; ii < scene->all_meshes.size(); ii++)
		{
			// Set model matrix
			dirDepth->set_model_matrix(scene->all_meshes[ii]->get_model_matrix());
			// Draw
			dirDepth->render(scene->all_meshes[ii]->get_VAO(), scene->all_meshes[ii]->get_triangle_count() * 3);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Point light shadows
	pointDepth->start_program();
	for (int i = 0; i < scene->point_lights.size(); i++)
	{
		glViewport(0, 0, scene->point_lights[i]->depth_map_width, scene->point_lights[i]->depth_map_height); // Use shadow resolutions
		glBindFramebuffer(GL_FRAMEBUFFER, scene->point_lights[i]->depth_cubemap_fbo);
		glClear(GL_DEPTH_BUFFER_BIT);

		// Draw scene
		for (int ii = 0; ii < scene->all_meshes.size(); ii++)
		{
			// Set model matrix
			pointDepth->set_model_matrix(scene->all_meshes[ii]->get_model_matrix());
			// Set space matrices
			pointDepth->set_space_matrices(scene->point_lights[i]->space_matrices);
			// Set far
			pointDepth->set_far(scene->point_lights[i]->shadow_projection_far);
			// Set position
			pointDepth->set_position(scene->point_lights[i]->position);
			// Draw
			pointDepth->render(scene->all_meshes[ii]->get_VAO(), scene->all_meshes[ii]->get_triangle_count() * 3);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	//
	//// 2. Lighting Pass: Calculate lighting pixel by pixel using gBuffer
	//

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	deferredShading->start_program();

	deferredShading->set_viewer_pos(scene->camera->get_eye());
	// Set g-buffer color attachments
	deferredShading->set_gPosition(GBuffer->get_gPosition());
	deferredShading->set_gNormal(GBuffer->get_gNormal());
	deferredShading->set_gAlbedoSpec(GBuffer->get_gAlbedoSpec());

	// Point lights
	for (int i = 0; i < scene->point_lights.size(); i++)
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
	for (int i = 0; i < scene->direct_lights.size(); i++)
	{
		deferredShading->set_direct_light(
			scene->direct_lights[i]->color,
			scene->direct_lights[i]->direction,
			scene->direct_lights[i]->intensity,
			scene->direct_lights[i]->depth_map,
			scene->direct_lights[i]->space_matrix
		);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Render to quad
	deferredShading->render(quad_VAO, 6);

	//
	//// 3. Pass: Draw light meshes (The meshes that are not lit but in the same scene with other meshes)
	//

	// Attach depth buffer to default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, GBuffer->get_fbo());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
	glBlitFramebuffer(0, 0, GBuffer->get_width(), GBuffer->get_height(), 0, 0,
		GBuffer->get_width(), GBuffer->get_height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Start forward rendering program
	forwardRender->start_program();

	// Set camera attributes
	forwardRender->set_projection_matrix(scene->camera->get_projection_matrix());
	forwardRender->set_view_matrix(scene->camera->get_view_matrix());

	// Draw scene
	for (int i = 0; i < scene->point_lights.size(); i++)
	{
		forwardRender->set_model_matrix(scene->point_lights[i]->mesh->get_model_matrix());
		forwardRender->set_color(scene->point_lights[i]->color);
		forwardRender->render(scene->point_lights[i]->mesh->get_VAO(), scene->point_lights[i]->mesh->get_triangle_count() * 3);
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
	GBuffer->set_gBuffer_resolution(Globals::WIDTH, Globals::HEIGHT);
	// Initialize quad that will be rendered with texture (The scene texture)
	init_quad();

	// Initialize depth map program
	dirDepth = new DirectionalDepth();

	// Initialize point depth
	pointDepth = new PointDepth();

	// Deferred shading program
	deferredShading = new DeferredShading();
	deferredShading->change_viewport_resolution(Globals::WIDTH, Globals::HEIGHT);

	// Forward render program
	forwardRender = new ForwardRender();
	forwardRender->change_viewport_resolution(Globals::WIDTH, Globals::HEIGHT);
}

// Initialize a quad
void Renderer::init_quad()
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
	GLuint quadVBO;
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