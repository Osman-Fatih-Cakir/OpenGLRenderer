#include <Skybox.h>
#include "stb_image.h"
#include <iostream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <init_shaders.h>

typedef glm::vec3 vec3;
typedef glm::mat4 mat4;

Skybox::Skybox(const char* path)
{
	load_hdr_file(path);
	generate_skybox_map();
	//generate_irradiance_map();
}

void Skybox::generate_skybox_map()
{
    ///////// DEBUG PURPOSE /////////
    mat4 p = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 1000.f);
    mat4 v = glm::lookAt(vec3(1.5f, 1.5f, 1.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
    GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/skybox_debug_vs.glsl");
    GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/skybox_debug_fs.glsl");
    GLuint program = initprogram(vertex_shader, fragment_shader);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, &p[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, &v[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(program, "equirectangular_map"), 0);
    glBindTexture(GL_TEXTURE_2D, equirectangular_map);
    render_cube();
    GLuint err = glGetError(); if (err) fprintf(stderr, "%s\n", gluErrorString(err));
    glutSwapBuffers();
    glutPostRedisplay(); // Render loop
    ///////// DEBUG PURPOSE /////////
}

void Skybox::load_hdr_file(const char* path)
{
	// TODO check the flip thing with false ?!
	stbi_set_flip_vertically_on_load(true);

	int width, height, nrComponents;
	float* data = stbi_loadf(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		glGenTextures(1, &equirectangular_map);
		glBindTexture(GL_TEXTURE_2D, equirectangular_map);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load HDR image." << std::endl;
	}
}

void Skybox::render_cube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,         
             1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            // front face
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
            // left face
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            // right face
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,      
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,    
            // bottom face
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            // top face
            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f , 1.0f,
             1.0f,  1.0f, -1.0f,  
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f       
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

GLuint Skybox::get_equirectangular_map()
{
	return equirectangular_map;
}
GLuint Skybox::get_skybox_map()
{
	return skybox_map;
}
GLuint Skybox::get_irradiance_map()
{
	return irradiance_map;
}