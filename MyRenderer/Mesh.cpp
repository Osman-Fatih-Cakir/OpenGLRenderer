
#include "Mesh.h"

// Constructor
Mesh::Mesh(std::string path)
{
	VAO = load_monkey(path);
	Globals::Log("Loaded file: " + path);
}

GLuint Mesh::get_VAO()
{
	return VAO;
}

unsigned int Mesh::get_triangle_count()
{
	return triangle_count;
}


// Loads the mesh from specified path
GLuint Mesh::load_monkey(std::string path)
{
	std::vector<vec3> vertices;
	std::vector<vec2> tex_coords;
	std::vector<vec3> normals;

	std::vector<vec3> temp_vertices;
	std::vector<vec2> temp_tex_coords;
	std::vector<vec3> temp_normals;
	std::vector<unsigned int> vertex_indices;
	std::vector<unsigned int> tex_coord_indices;
	std::vector<unsigned int> normal_indices;

	// Load datas from file
	FILE* obj_file = fopen(path.c_str(), "r");
	if (obj_file == NULL)
	{
		Globals::Log("Error opening file.");
		Globals::Log(path);
		return -1;
	}

	while (true)
	{
		char head[128]; // First word should not exceed 128 characters
		// Read the first word
		int res = fscanf(obj_file, "%s", head);
		if (res == EOF) // End of file
			break;

		if (strcmp(head, "v") == 0) // Vertices
		{
			vec3 vertex;
			fscanf(obj_file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			// Converting float to GLfloat (Not sure if it is necessery)
			vertex.x = (GLfloat)vertex.x;
			vertex.y = (GLfloat)vertex.y;
			vertex.z = (GLfloat)vertex.z;
			temp_vertices.push_back(vertex);
		}
		else if(strcmp(head, "vn") == 0) // Normals
		{
			vec3 normal;
			fscanf(obj_file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			// Converting float to GLfloat (Not sure if it is necessery)
			normal.x = (GLfloat)normal.x;
			normal.y = (GLfloat)normal.y;
			normal.z = (GLfloat)normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(head, "vt") == 0) // Texture coordinates
		{
			vec2 uv;
			fscanf(obj_file, "%f %f\n", &uv.x, &uv.y);
			// Converting float to GLfloat (Not sure if it is necessery)
			uv.x = (GLfloat)uv.x;
			uv.y = (GLfloat)uv.y;
			temp_tex_coords.push_back(uv);
		}
		else if (strcmp(head, "f") == 0) // Faces
		{
			// Increase triangle count for each face
			triangle_count++;

			// Vertices, normals an tex coordinates should be loaded from this far
			// We arrange the data in the correct format and store
			unsigned int ver_index[3], tex_coord_index[3], norm_index[3];
			int matches = fscanf(obj_file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", 
				&ver_index[0], &tex_coord_index[0], &norm_index[0],
				&ver_index[1], &tex_coord_index[1], &norm_index[1],
				&ver_index[2], &tex_coord_index[2], &norm_index[2]
			);
			if (matches != 9) // Parsing error
			{
				Globals::Log("Error when parsing face data of mesh.");
			}

			// Set the indices
			vertex_indices.push_back(ver_index[0]);
			vertex_indices.push_back(ver_index[1]);
			vertex_indices.push_back(ver_index[2]);
			tex_coord_indices.push_back(tex_coord_index[0]);
			tex_coord_indices.push_back(tex_coord_index[1]);
			tex_coord_indices.push_back(tex_coord_index[2]);
			normal_indices.push_back(norm_index[0]);
			normal_indices.push_back(norm_index[1]);
			normal_indices.push_back(norm_index[2]);
		}
	}

	// After loading, process the data and set correct arrays
	for (unsigned int i = 0; i < vertex_indices.size(); i++)
	{
		vec3 vertex = temp_vertices[vertex_indices[i] - 1]; // Indices starts with 1, not 0 (zero)
		vec3 normal = temp_normals[normal_indices[i] - 1]; // Indices starts with 1, not 0 (zero)
		vec2 tex_coord = temp_tex_coords[tex_coord_indices[i] - 1]; // Indices starts with 1, not 0 (zero)
		vertices.push_back(vertex);
		normals.push_back(normal);
		tex_coords.push_back(tex_coord);
	}
	
	// Then, generate buffers using data
	GLuint vao = gen_monkey_buffers(vertices, normals, tex_coords);

	return vao;
}

// Generates buffers from mesh data
GLuint Mesh::gen_monkey_buffers(std::vector<vec3>& vertices, std::vector<vec3>& normals, std::vector<vec2>& tex_coords)
{
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Vertices buffer
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0); // Layout 0 for vertex in vertex shader
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	// Normals buffer
	GLuint NBO;
	glGenBuffers(1, &NBO);
	glBindBuffer(GL_ARRAY_BUFFER, NBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * normals.size(), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1); // Layout 1 for normal in vertex shader
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	// Texture coordinates buffer
	GLuint TBO;
	glGenBuffers(1, &TBO);
	glBindBuffer(GL_ARRAY_BUFFER, TBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * tex_coords.size(), &tex_coords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2); // Layout 2 for texture coordinates in vertex shader
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0); // Prevent further modifications

	return VAO;
}