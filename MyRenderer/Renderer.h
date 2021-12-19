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

class Renderer
{
public:

	Renderer() = delete;
	Renderer(Scene* _scene);
	~Renderer();

	void render(float delta);

private:

	Scene* scene;
	gBuffer* GBuffer = nullptr;
	DirectionalDepth* dirDepth = nullptr;
	PointDepth* pointDepth = nullptr;
	ForwardRender* forwardRender = nullptr;
	DeferredShading* deferredShading = nullptr;
	unsigned long time = 0;

	void init_shader_programs();
};