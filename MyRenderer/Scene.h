#pragma once

#include <vector>
#include <Mesh.h>
#include <Camera.h>
#include <DirectionalLight.h>
#include <PointLight.h>

class Scene
{
public:

	Scene();
	~Scene();

	Camera* camera;
	std::vector<Mesh*> all_meshes;
	std::vector<DirectionalLight*> direct_lights;
	std::vector<PointLight*> point_lights;

	void add_mesh(Mesh* m);
	void add_point_light(PointLight* p);
	void add_direct_light(DirectionalLight* d);

private:

};
