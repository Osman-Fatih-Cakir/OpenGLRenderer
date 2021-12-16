#pragma once

#include <vector>
#include <Model.h>
#include <Camera.h>
#include <DirectionalLight.h>
#include <PointLight.h>
#include <Skybox.h>

class Scene
{
public:

	Scene();
	~Scene();

	Camera* camera;
	Skybox* skybox = nullptr;
	std::vector<Model*> all_models;
	std::vector<DirectionalLight*> direct_lights;
	std::vector<PointLight*> point_lights;

	void add_model(Model* m);
	void add_point_light(PointLight* p);
	void add_direct_light(DirectionalLight* d);

private:
	
};
