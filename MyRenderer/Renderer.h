#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <Scene.h>
#include <gBuffer.h>
#include <DirectionalDepth.h>
#include <PointDepth.h>
#include <ForwardRender.h>
#include <DeferredShading.h>
#include <Timer.h>
#include <MainFramebuffer.h>
#include <Bloom.h>
#include <ForwardLitRender.h>
#include <TAA.h>

class Renderer
{
public:

	Renderer() = delete;
	Renderer(Scene* _scene);
	~Renderer();

	void render(float delta);

private:

	void init();
	void init_uniforms();
	void store_previous_framebuffer(GLuint depth_fbo);
	void render_all(MainFramebuffer* main_fb);
	void set_texture(GLuint id);
	void render_quad();

	Scene* scene;
	gBuffer* GBuffer = nullptr;
	DirectionalDepth* dirDepth = nullptr;
	PointDepth* pointDepth = nullptr;
	ForwardRender* forwardRender = nullptr;
	DeferredShading* deferredShading = nullptr;
	MainFramebuffer* main_fb = nullptr;
	MainFramebuffer* prev_fb = nullptr;
	Bloom* bloom = nullptr;
	ForwardLitRender* forwardLitRender = nullptr;
	TAA* taa = nullptr;
	unsigned long time = 0;
	unsigned int total_frames = 0;

	GLuint render_program;
	GLuint loc_texture;
};
