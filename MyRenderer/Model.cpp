
#include <iostream>
#include <Model.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h> // Image loading library for load texture images
#include <gtc/matrix_transform.hpp>

typedef glm::vec2 vec2;

// Constructor
Model::Model(const char* path, bool _translucent)
{
	load_model(path);

    // Set normal matrix
    update_normal_matrix();

    translucent = _translucent;
}

// Destructor
Model::~Model()
{
    // Deallocate meshes
    for (int i = 0; i < meshes.size(); i++)
        delete meshes[i];
    for (int i = 0; i < translucent_meshes.size(); i++)
        delete translucent_meshes[i];

    // Deallocate textures
    for (int i = 0; i < textures_loaded.size(); i++)
        glDeleteTextures(1, &textures_loaded[i].id);
}

// Translate model
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

// Scale model
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

bool Model::is_translucent()
{
    return translucent;
}

// Draws every mesh
void Model::draw(GLuint shader_program, vec3 cam_pos)
{
    // Draw opaque meshes first
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        meshes[i]->draw(shader_program, has_normal_map, has_ao_map, has_emissive_map, 
            has_opacity_map, model_matrix);
    }
    // Draw translucent meshes in sorted order
    std::map<float, Mesh*> sorted = sort_translucent_meshes(cam_pos);
    
    for (std::map<float, Mesh*>::reverse_iterator it = sorted.rbegin();
        it != sorted.rend(); ++it)
    {
        it->second->draw(shader_program, has_normal_map, has_ao_map, has_emissive_map,
            has_opacity_map, model_matrix);
    }
}

// Load the model from the path
void Model::load_model(std::string path)
{
    std::cout << "Loading model: " << path << std::endl;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate
        | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    
    aiMatrix4x4* tr = new aiMatrix4x4();
    process_node(scene->mRootNode, scene, *tr);
}

// Process each node in model
void Model::process_node(aiNode* node, const aiScene* scene, aiMatrix4x4 tr)
{
    tr = tr * node->mTransformation;
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh* _mesh = process_mesh(mesh, scene, tr);
        if (_mesh->has_alpha()) // Store translucent meshes in another array
        {
            translucent_meshes.push_back(_mesh);
        }
        else
        {
            meshes.push_back(_mesh);
        }
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        process_node(node->mChildren[i], scene, tr);
    }
}

// Process mesh and store datas
Mesh* Model::process_mesh(aiMesh* mesh, const aiScene* scene, aiMatrix4x4 transformation)
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
    has_alpha = false;
    std::vector<Texture> albedo_maps = load_material_textures(material, aiTextureType_DIFFUSE, "albedo_map");
    textures.insert(textures.end(), albedo_maps.begin(), albedo_maps.end());
    // Normal map
    std::vector<Texture> normal_maps = load_material_textures(material, aiTextureType_NORMALS, "normal_map");
    if (normal_maps.size() > 0) has_normal_map = true;
    textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());
    // Roughness metallic map
    std::vector<Texture> metallic_maps = load_material_textures(material, aiTextureType_UNKNOWN, "metallic_roughness_map");
    textures.insert(textures.end(), metallic_maps.begin(), metallic_maps.end());
    // AO map
    std::vector<Texture> ao_maps = load_material_textures(material, aiTextureType_LIGHTMAP, "ao_map");
    if (ao_maps.size() > 0) has_ao_map = true;
    textures.insert(textures.end(), ao_maps.begin(), ao_maps.end());
    // Emmisive map
    std::vector<Texture> ems_maps = load_material_textures(material, aiTextureType_EMISSIVE, "emissive_map");
    if (ems_maps.size() > 0) has_emissive_map = true;
    textures.insert(textures.end(), ems_maps.begin(), ems_maps.end());
    // Opacity map // TODO implement
    has_opacity_map = false;
    
    mat4 _transformation = {
        transformation.a1, transformation.b1, transformation.c1, transformation.d1,
        transformation.a2, transformation.b2, transformation.c2, transformation.d2,
        transformation.a3, transformation.b3, transformation.c3, transformation.d3,
        transformation.a4, transformation.b4, transformation.c4, transformation.d4,
    };

    // Return a mesh object created from the extracted mesh data
    return new Mesh(vertices, indices, textures, _transformation, has_alpha);
}

// Load textures
std::vector<Texture> Model::load_material_textures(aiMaterial* mat, aiTextureType type, 
    std::string typeName)
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
                has_alpha = true;
                break;
            }
        }
        if (!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = texture_from_file(str.C_Str(), directory, typeName);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); // add to loaded textures
        }
    }
    return textures;
}

// Loads texture from image file
unsigned int Model::texture_from_file(const char* path, const std::string& directory,
        std::string& typeName)
{
    stbi_set_flip_vertically_on_load(false);
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
        {
            format = GL_RGBA;
            if (typeName.compare("albedo_map") == 0)
                has_alpha = true;
        }

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

// Sorts meshes via the first vertex distance to camera
// (This is not good and I will come back to it later)
std::map<float, Mesh*> Model::sort_translucent_meshes(vec3 cam_pos)
{
    std::map<float, Mesh*> tr_mesh_map;
    for (std::vector<Mesh*>::iterator itr = translucent_meshes.begin();
        itr != translucent_meshes.end(); itr++)
    {
        //float avg = 0.f;
        //for (std::vector<Vertex>::iterator it = (*itr)->vertices.begin();
        //    it != (*itr)->vertices.end(); it++)
        //{
        //    avg += glm::length(cam_pos - (*it).position);
        //}
        //avg = avg / (*itr)->vertices.size();
        //tr_mesh_map[avg] = (*itr);
        float avg = glm::length(cam_pos - (*itr)->vertices[0].position);
        tr_mesh_map[avg] = (*itr);
    }
    return tr_mesh_map;
}
