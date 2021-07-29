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
    Model(const char* path);
    ~Model();

    bool has_normal_map = false;

    void translate(vec3 vec);
    // TODO rotate
    // TODO scale // TODO update_normal_matrix
    mat4 get_model_matrix();
    mat4 get_normal_matrix();
    void draw(GLuint shader_program);

private:
    // Model data
    std::vector<Mesh> meshes;
    std::vector<Texture> textures_loaded;
    std::string directory;
    mat4 model_matrix = mat4(1.0f);
    mat4 normal_matrix = mat4(1.0f);

    void load_model(std::string path);
    void process_node(aiNode* node, const aiScene* scene);
    Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> load_material_textures(aiMaterial* mat, aiTextureType type, std::string typeName);
    unsigned int texture_from_file(const char* path, const std::string& directory);
    void update_normal_matrix();
};