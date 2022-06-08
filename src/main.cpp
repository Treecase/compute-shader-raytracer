/**
 * main.cpp - Compute shaders demo.
 * Copyright (C) 2022 Trevor Last
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "glUtil.hpp"

#include <GL/glew.h>
#include <SDL.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


/** Load a GLSL Shader from a file. */
Shader shader_from_file(std::string path, GLenum type)
{
    std::ifstream shaderfile{path.c_str()};
    std::stringstream srcstream{};
    srcstream << shaderfile.rdbuf();
    shaderfile.close();
    auto source = srcstream.str();
    return Shader{type, source};
}


int main(int argc, char *argv[])
{
    int window_width = 640,
        window_height = 480;

    /* ===[ Initialization ]=== */
    SDL_Init(SDL_INIT_VIDEO);

    // Set OpenGL context version and profile (4.3 core).
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create window and OpenGL context.
    auto window = SDL_CreateWindow(
        "compute",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_width, window_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    auto context = SDL_GL_CreateContext(window);

    // Print the received OpenGL version.
    int major = 0,
        minor = 0,
        profile = 0;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);
    std::cout << "OpenGl Version " << major << "." << minor << " ";
    switch (profile)
    {
    case SDL_GL_CONTEXT_PROFILE_CORE:
        std::cout << "core";
        break;
    case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY:
        std::cout << "compatibility";
        break;
    case SDL_GL_CONTEXT_PROFILE_ES:
        std::cout << "ES";
        break;
    default:
        std::cout << "Unrecognized Profile (" << profile << ")";
        break;
    }
    std::cout << "\n";

    // Init GLEW and check/print version.
    GLenum error = glewInit();
    if (error != GLEW_OK)
    {
        std::cerr << "glewInit - " << glewGetErrorString(error) << "\n";
        return EXIT_FAILURE;
    }
    if (glewIsSupported("GL_VERSION_4_3") == GL_FALSE)
    {
        std::cerr << "GLEW: OpenGL Version 4.3 not supported\n";
        return EXIT_FAILURE;
    }
    std::cout << "GLEW Version " << glewGetString(GLEW_VERSION) << "\n";

    // Set OpenGL viewport.
    glViewport(0, 0, window_width, window_height);


    /* ===[ Compute Shader ]=== */
    Program *compute_program = nullptr;
    try
    {
        compute_program = new Program{
            {shader_from_file("shaders/compute.comp", GL_COMPUTE_SHADER)}};
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    /* ===[ Visual Shader ]=== */
    Program *visual_program = nullptr;
    try
    {
        visual_program = new Program{{
            shader_from_file("shaders/vertex.vert", GL_VERTEX_SHADER),
            shader_from_file("shaders/fragment.frag", GL_FRAGMENT_SHADER)}};
    }
    catch(std::runtime_error const &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    /* ===[ Screen Quad ]=== */
    GLfloat const vertices[] = {
        // Positions        Texcoords
        // Top right tri
       -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, // tl
        1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // tr
        1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // br
        // Bottom left tri
        1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // br
       -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // bl
       -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, // tl
    };
    // Create screen quad VAO.
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // Create vertex data buffer.
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Link position vertex attribute.
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);
    // Link texcoord vertex attribute.
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat),
        (void *)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // Unbind VAO.
    glBindVertexArray(0);

    /* ===[ Compute Shader Output Texture ]=== */
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA32F, window_width, window_height, 0, GL_RGBA,
        GL_FLOAT, nullptr);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    /* ===[ Compute Shader Input Data ]=== */
    // Define the scene.
    struct
    {
        GLfloat x, y, z;
        GLfloat r;
    } spheres[2];
    spheres[0] = {0.0f, 0.0f, -5.0f, 1.0f};
    spheres[1] = {2.0f, 0.0f, -5.0f, 0.25f};

    // Create sphere SSBO.
    GLuint ssbo = 0;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER, sizeof(spheres), spheres,
        GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);

    /* ===[ Main Loop ]=== */
    for (bool running = true; running;)
    {
        // Check events.
        SDL_Event event{};
        while (SDL_PollEvent(&event) && running)
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    // Viewport size and compute shader output texture depend
                    // on window size, so if it changes they have to be updated.
                    window_width = event.window.data1;
                    window_height = event.window.data2;
                    glViewport(0, 0, window_width, window_height);
                    glTexImage2D(
                        GL_TEXTURE_2D, 0, GL_RGBA32F, window_width,
                        window_height, 0, GL_RGBA, GL_FLOAT, nullptr);
                }
                break;
            }
        }

        /* ===[ Execute Compute Shader ]=== */
        // Set output texture uniform.
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(
            glGetUniformLocation(compute_program->id(), "outputImg"), 0);
        // Bind the sphere SSBO.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        // Use the compute shader.
        compute_program->use();
        // Run the compute shader.
        glDispatchCompute((GLuint)window_width, (GLuint)window_height, 1);
        // Wait for the shader to finish writing to the image.
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        /* ===[ Draw the Screenquad ]=== */
        // Clear the screen.
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Use the screenquad shader.
        visual_program->use();
        // Bind screenquad VAO.
        glBindVertexArray(vao);
        // Use the compute output texture as the input texture.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(visual_program->id(), "tex"), 0);
        // Render the screenquad.
        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices)/sizeof(*vertices));

        // Update the screen.
        SDL_GL_SwapWindow(window);
    }

    // Cleanup OpenGL data.
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &ssbo);

    // Cleanup SDL data.
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
