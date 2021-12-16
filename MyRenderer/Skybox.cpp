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
    init_shader();
    get_uniform_location();
	generate_skybox_map();
	generate_irradiance_map();
}

void Skybox::generate_skybox_map()
{
    create_skybox_map();
    create_framebuffer();
    
    // TODO try sending space matrices rather than projection and view seperately
    // A view matrix for each face
    mat4 p = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
    mat4 views[] =
    {
        // TODO check these views are correct
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f))
    };

    glUseProgram(program);

    // Viewport
    glViewport(0, 0, width, height);

    // Bind framebuffer to get texture from color attachemnts
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    
    // Set the uniforms
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, &p[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(program, "equirectangular_map"), 0);
    glBindTexture(GL_TEXTURE_2D, equirectangular_map);

    // Render 6 times for each face of the cube
    for (int i = 0; i < 6; i++)
    {
        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, &views[i][0][0]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skybox_map, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_cube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    GLuint err = glGetError(); if (err) fprintf(stderr, "Generating skybox: %s\n", gluErrorString(err));
}

void Skybox::generate_irradiance_map()
{
    create_irradiance_map();

    mat4 p = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
    mat4 views[] =
    {
        // TODO check these views are correct
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f))
    };

    glUseProgram(irradiance_program);

    glViewport(0, 0, irradiance_width, irradiance_height);

    // Bind framebuffer and render buffer with irradiance map sizes
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irradiance_width, irradiance_height);

    // Set the uniforms
    glUniformMatrix4fv(glGetUniformLocation(irradiance_program, "projection"), 1, GL_FALSE, &p[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(irradiance_program, "skybox_map"), 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_map);
    
    // Render 6 times for each face of the cube
    for (int i = 0; i < 6; i++)
    {
        glUniformMatrix4fv(glGetUniformLocation(irradiance_program, "view"), 1, GL_FALSE, &views[i][0][0]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_map, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_cube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint err = glGetError(); if (err) fprintf(stderr, "Generating irradiance map: %s\n",
        gluErrorString(err));
}

void Skybox::create_irradiance_map()
{
    glGenTextures(1, &irradiance_map);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_map);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
            irradiance_width, irradiance_height, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Skybox::create_framebuffer()
{
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
}

void Skybox::create_skybox_map()
{
    glGenTextures(1, &skybox_map);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_map);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Skybox::init_shader()
{
    GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/equirect_to_cube_vs.glsl");
    GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/equirect_to_cube_fs.glsl");
    program = initprogram(vertex_shader, fragment_shader);

    vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/skybox_vs.glsl");
    fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/skybox_fs.glsl");
    render_program = initprogram(vertex_shader, fragment_shader);

    vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/irradiance_map_vs.glsl");
    fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/irradiance_map_fs.glsl");
    irradiance_program = initprogram(vertex_shader, fragment_shader);
}

void Skybox::get_uniform_location()
{
    // TODO space matrix?
    loc_projection_matrix = glGetUniformLocation(render_program, "projection");
    loc_view_matrix = glGetUniformLocation(render_program, "view");
    loc_skybox_map = glGetUniformLocation(render_program, "skybox_map");
}

void Skybox::set_view_matrix(mat4 mat)
{
    glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, &mat[0][0]);
}
void Skybox::set_projection_matrix(mat4 mat)
{
    glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, &mat[0][0]);
}
void Skybox::set_skybox_map()
{
    glUniform1i(loc_skybox_map, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_map);
}

void Skybox::load_hdr_file(const char* path)
{
	// why flip?
	//stbi_set_flip_vertically_on_load(true);

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
    GLuint cubeVAO = 0;
    GLuint cubeVBO = 0;
    // initialize
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
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void Skybox::render(Camera* camera)
{
    glUseProgram(render_program);
    glDepthFunc(GL_LEQUAL);
    set_projection_matrix(camera->get_projection_matrix());
    set_view_matrix(camera->get_view_matrix());
    set_skybox_map();
    render_cube();
    
    glDepthFunc(GL_LESS); // Return to default depth test
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
GLuint Skybox::get_width()
{
    return width;
}
GLuint Skybox::get_height()
{
    return height;
}
