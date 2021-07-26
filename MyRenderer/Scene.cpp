
#include <Scene.h>

// Constructor
Scene::Scene()
{
}

// Destructor
Scene::~Scene()
{
	//
	//// Deallocate every pointer
	//
	
	// Mesh pointers
	for (unsigned int i = 0; i < all_models.size(); i++)
	{
		delete all_models[i];
	}
	all_models.clear();

	// Direct lights
	for (unsigned int i = 0; i < direct_lights.size(); i++)
	{
		delete direct_lights[i];
	}
	direct_lights.clear();

	// Point lights
	for (unsigned int i = 0; i < point_lights.size(); i++)
	{
		delete point_lights[i];
	}
	point_lights.clear();

	// Camera
	delete camera;
}

// Add mesh to scene
void Scene::add_model(Model* m)
{
	all_models.push_back(m);
}

// Add point light to scene
void Scene::add_point_light(PointLight* p)
{
	point_lights.push_back(p);
}

// Add direct light to scene
void Scene::add_direct_light(DirectionalLight* d)
{
	direct_lights.push_back(d);
}
