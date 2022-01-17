#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <glm.hpp>
#include <Mesh.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

typedef glm::vec3 vec3;
typedef glm::mat4 mat4;

class Model
{
public:
    // Constructor and destructor
    Model(const char* path, bool _translucent);
    ~Model();

    void translate(vec3 vec, float delta);
    void translate(float x, float y, float z, float delta);
    void rotate(vec3 vec, float angle, float delta);
    void scale(vec3 vec, float delta);
    void scale(float x, float y, float z, float delta);
    mat4 get_model_matrix();
    mat4 get_normal_matrix();
    bool is_translucent();
    void draw(GLuint shader_program);

private:
    // Model data
    std::vector<Mesh*> meshes;
    std::vector<Texture> textures_loaded;
    std::string directory;
    mat4 model_matrix = mat4(1.0f);
    mat4 normal_matrix = mat4(1.0f);

    bool has_normal_map = false;
    bool has_ao_map = false;
    bool has_emissive_map = false;
    bool has_opacity_map = false;
    bool translucent = false;

    void load_model(std::string path);
    void process_node(aiNode* node, const aiScene* scene, aiMatrix4x4 tr);
    Mesh* process_mesh(aiMesh* mesh, const aiScene* scene, aiMatrix4x4 transformation);
    std::vector<Texture> load_material_textures(aiMaterial* mat, aiTextureType type, std::string typeName);
    unsigned int texture_from_file(const char* path, const std::string& directory);
    void update_normal_matrix();
};
