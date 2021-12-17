
#include <Scene.h>
#include <iostream>

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

	// Skybox
	for (int i = 0; i < skyboxes.size(); i++)
	{
		delete skyboxes[i];
	}

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

// Add skybox to the scene
void Scene::add_skybox(const char* path, int id)
{
	Skybox* skybox = new Skybox(path, id);
	skyboxes.push_back(skybox);
}

// Returns the skybox that should be rendered
Skybox* Scene::get_render_skybox()
{
	if (skybox_id == -1)
	{
		std::cout << "Error: No skybox in the scene!" << "\n";
	}
	else
	{
		return get_skybox(skybox_id);
	}

	return nullptr;
}

// Sets the skybox of the scene with the skybox id
void Scene::render_skybox_id(int id)
{
	skybox_id = id;
}

// Returns the skybox with that id
Skybox* Scene::get_skybox(int id)
{
	for (int i = 0; i < skyboxes.size(); i++)
	{
		if (skyboxes[i]->get_id() == id)
		{
			return skyboxes[i];
		}
	}

	std::cout << "Error: No skybox with id: " << id << "\n";
	return nullptr;
}

// Deletes the skybox from the memory
void Scene::delete_skybox(int id)
{
	for (int i = 0; i < skyboxes.size(); i++)
	{
		if (skyboxes[i]->get_id() == id)
		{
			Skybox* temp = skyboxes[i];
			skyboxes.erase(skyboxes.begin() + i);
			delete temp;
		}
	}
}
