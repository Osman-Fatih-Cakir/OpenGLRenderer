
#include <Bloom.h>
#include <init_shaders.h>

Bloom::Bloom()
{
    create_framebuffers();
    init_shader();
    get_uniform_locations();

}

Bloom::~Bloom()
{

}

void Bloom::start(GLuint main_fb, GLuint texture)
{
    // Take hdr image and bright image
    take_hdr_image(texture);

    // Blur the bright image
    blur();

    // Converge blurred image to hdr image
    converge(main_fb);
}

void Bloom::take_hdr_image(GLuint texture)
{
    // Take the hdr image with rendering to color buffers of hdr framebuffer

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_buffers[0], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, color_buffers[1], 0);
    
    glUseProgram(hdr_program);
    glViewport(0, 0, width, height);

    // Set uniforms
    set_hdr_image(texture);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render_quad();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Bloom::blur()
{
    bool horizontal = true, first_iteration = true;
    glUseProgram(blur_program);

    for (unsigned int i = 0; i < blur_amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[(int)horizontal]);
        set_horizontal(horizontal);

        set_blur_image(first_iteration ? color_buffers[1] : pingpong_textures[!horizontal]);

        render_quad();

        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Bloom::converge(GLuint main_fb)
{
    // Converge blurred image to hdr image
    glUseProgram(converge_program);
    glBindFramebuffer(GL_FRAMEBUFFER, main_fb);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    set_conv_img1(pingpong_textures[1]);
    set_conv_img2(color_buffers[0]);

    render_quad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Bloom::create_framebuffers()
{
    // Hdr framebuffer holds the real hdr image
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // Create 2 color buffers (1 for normal rendering, other for brightness threshold values)
    glGenTextures(2, color_buffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, color_buffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // Attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
            GL_TEXTURE_2D, color_buffers[i], 0);
    }

    // Create and attach depth buffer (renderbuffer)
    GLuint hdrRBO;
    glGenRenderbuffers(1, &hdrRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, hdrRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrRBO);

    // Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR: Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Ping-pong-framebuffer for blurring
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpong_textures);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpong_textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // We clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            pingpong_textures[i], 0);
        // Check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR: Framebuffer not complete!" << std::endl;
    }
}

void Bloom::set_hdr_image(GLuint id)
{
    glUniform1i(loc_hdr_image, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Bloom::set_blur_image(GLuint id)
{
    glUniform1i(loc_blur_image, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Bloom::set_horizontal(bool val)
{
    glUniform1i(loc_horizontal, (int)val);
}

void Bloom::set_conv_img1(GLuint id)
{
    glUniform1i(loc_conv_img1, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Bloom::set_conv_img2(GLuint id)
{
    glUniform1i(loc_conv_img2, 1);
    glActiveTexture(GL_TEXTURE1 + 0);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Bloom::init_shader()
{
    GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/bloom_hdr_vs.glsl");
    GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/bloom_hdr_fs.glsl");
    hdr_program = initprogram(vertex_shader, fragment_shader);

    vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/blur_vs.glsl");
    fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/blur_fs.glsl");
    blur_program = initprogram(vertex_shader, fragment_shader);

    vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/converge_2_image_vs.glsl");
    fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/converge_2_image_fs.glsl");
    converge_program = initprogram(vertex_shader, fragment_shader);
}

void Bloom::get_uniform_locations()
{
    loc_hdr_image = glGetUniformLocation(hdr_program, "image");

    loc_horizontal = glGetUniformLocation(blur_program, "horizontal");
    loc_blur_image = glGetUniformLocation(blur_program, "image");

    loc_conv_img1 = glGetUniformLocation(converge_program, "image1");
    loc_conv_img2 = glGetUniformLocation(converge_program, "image2");
}

void Bloom::render_quad()
{
    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}