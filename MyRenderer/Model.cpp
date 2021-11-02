
#include <iostream>
#include <Model.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h> // Image loading library for load texture images
#include <gtc/matrix_transform.hpp>

typedef glm::vec2 vec2;

// Constructor
Model::Model(const char* path)
{
	load_model(path);

    // Set normal matrix
    update_normal_matrix();
}

// Destructor
Model::~Model() {}

// Translate mesh
void Model::translate(vec3 vec, float delta)
{
    vec3 _vec = vec3(vec.x * delta, vec.y * delta, vec.z * delta);
    model_matrix = glm::translate(model_matrix, _vec);
}
void Model::translate(float x, float y, float z, float delta)
{
    model_matrix = glm::translate(model_matrix, vec3(x * delta, y * delta, z * delta));
}

// Rotate model
void Model::rotate(vec3 vec, float angle, float delta)
{
    model_matrix = glm::rotate(model_matrix, glm::radians(angle * delta), vec);
    update_normal_matrix();
}

// Scale mesh
void Model::scale(vec3 vec, float delta)
{
    vec3 _vec = vec3(vec.x * delta, vec.y * delta, vec.z * delta);
    model_matrix = glm::scale(model_matrix, _vec);
    update_normal_matrix();
}
void Model::scale(float x, float y, float z, float delta)
{
    model_matrix = glm::scale(model_matrix, vec3(x * delta, y * delta, z * delta));
    update_normal_matrix();
}

mat4 Model::get_model_matrix()
{
    return model_matrix;
}

mat4 Model::get_normal_matrix()
{
    return normal_matrix;
}

// Draws every mesh
void Model::draw(GLuint shader_program)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        meshes[i].draw(shader_program, has_normal_map);
    }
}

// Load the model from the path
void Model::load_model(std::string path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate 
        | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    process_node(scene->mRootNode, scene);
}

// Process each node in model
void Model::process_node(aiNode* node, const aiScene* scene)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(process_mesh(mesh, scene));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        process_node(node->mChildren[i], scene);
    }
}

// Process mesh and store datas
Mesh Model::process_mesh(aiMesh* mesh, const aiScene* scene)
{
    // Data to fill
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;
        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }
        // texture coordinates
        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            // A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0)
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoord = vec;
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.bitangent = vector;
        }
        else
        {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }
    // Now walk through each of the mesh's faces and retrieve the corresponding vertex indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // Process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    // Albedo map
    std::vector<Texture> albedo_maps = load_material_textures(material, aiTextureType_DIFFUSE, "albedo_map");
    textures.insert(textures.end(), albedo_maps.begin(), albedo_maps.end());
    // Normal map
    std::vector<Texture> normal_maps = load_material_textures(material, aiTextureType_HEIGHT, "normal_map");
    if (normal_maps.size() > 0) has_normal_map = true;
    textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());
    // Metallic map
    std::vector<Texture> metallic_maps = load_material_textures(material, aiTextureType_AMBIENT, "metallic_map");
    textures.insert(textures.end(), metallic_maps.begin(), metallic_maps.end());
    // Roughness map
    std::vector<Texture> roughness_maps = load_material_textures(material, aiTextureType_SHININESS, "roughness_map");
    textures.insert(textures.end(), roughness_maps.begin(), roughness_maps.end());
    // AO map
    std::vector<Texture> ao_maps = load_material_textures(material, aiTextureType_OPACITY, "ao_map");
    textures.insert(textures.end(), ao_maps.begin(), ao_maps.end());

    // return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, textures);
}

// Load textures
std::vector<Texture> Model::load_material_textures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = texture_from_file(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); // add to loaded textures
        }
    }
    return textures;
}

// Loads texture from image file
unsigned int Model::texture_from_file(const char* path, const std::string& directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// Updates normal matrix
void Model::update_normal_matrix()
{
    normal_matrix = glm::transpose(glm::inverse(model_matrix));
}
