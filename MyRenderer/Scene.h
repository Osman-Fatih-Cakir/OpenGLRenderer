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
	std::vector<Skybox*> skyboxes;
	std::vector<Model*> all_models;
	std::vector<Model*> translucent_models;
	std::vector<DirectionalLight*> direct_lights;
	std::vector<PointLight*> point_lights;

	void add_model(Model* m);
	// TODO delete model function
	void add_point_light(PointLight* p);
	void add_direct_light(DirectionalLight* d);
	void add_skybox(const char* path, int id, bool ibl, float exposure);
	Skybox* get_render_skybox();
	void delete_skybox(int id);
	void render_skybox_id(int id);

private:
	Skybox* get_skybox(int id);

	int skybox_id = -1;
};
