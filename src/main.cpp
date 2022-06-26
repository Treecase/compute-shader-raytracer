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
#include "ShaderStructs.hpp"

#include <GL/glew.h>
#include <SDL.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>


/**
 * The application.
 */
class App
{
private:
    std::unordered_map<
        Uint32,
        std::vector<std::function<void(SDL_Event)>>> _event_callbacks;
    SDL_Window *const _window;
    SDL_GLContext const _context;
public:
    int window_width,
        window_height;
    bool running;

    App(std::string title, int width, int height)
    :   _event_callbacks{}
    ,   _window{SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)}
    ,   _context{SDL_GL_CreateContext(_window)}
    ,   window_width{width}
    ,   window_height{height}
    ,   running{true}
    {
        glViewport(0, 0, window_width, window_height);
    }

    ~App()
    {
        SDL_GL_DeleteContext(_context);
        SDL_DestroyWindow(_window);
    }


    /** Set up an event callback. */
    void add_callback(
        SDL_EventType event, std::function<void(SDL_Event)> callback)
    {
        _event_callbacks.insert(
            {event, std::vector<std::function<void(SDL_Event)>>{}});
        _event_callbacks[event].push_back(callback);
    }


    /** Handle input events. */
    void input()
    {
        SDL_Event event{};
        while (SDL_PollEvent(&event))
        {
            // Built-in actions.
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
                }
                break;
            }

            // Run user-added callbacks.
            bool valid = true;
            try
            {
                auto _ = _event_callbacks.at(event.type);
            }
            catch (std::out_of_range const &)
            {
                valid = false;
            }
            if (valid)
            {
                for (auto callback : _event_callbacks[event.type])
                {
                    callback(event);
                }
            }
        }
    }

    /** Update the screen. */
    void render()
    {
        // Update the screen.
        SDL_GL_SwapWindow(_window);
    }
};


/** Load a GLSL Shader from a file. */
Shader shader_from_file(std::string path, GLenum type)
{
    std::ifstream shaderfile{path.c_str()};
    std::stringstream srcstream{};
    srcstream << shaderfile.rdbuf();
    shaderfile.close();
    return Shader{type, srcstream.str()};
}


/** SDL initialization. */
void init_SDL()
{
    SDL_Init(SDL_INIT_VIDEO);

    // Set OpenGL context version and profile (4.3 core).
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

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
}


/**
 * OpenGL initialization.
 * (Make sure to have an active OpenGL context before calling this!)
 */
void init_OpenGL()
{
    // Init GLEW and check/print version.
    GLenum error = glewInit();
    if (error != GLEW_OK)
    {
        throw std::runtime_error{
            "glewInit - " + std::string{(char *)glewGetErrorString(error)}};
    }
    if (glewIsSupported("GL_VERSION_4_3") == GL_FALSE)
    {
        throw std::runtime_error{"GLEW: OpenGL Version 4.3 not supported"};
    }
    std::cout << "GLEW Version " << glewGetString(GLEW_VERSION) << "\n";
}


#ifdef _WIN32
int SDL_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    /* ===[ Initialization ]=== */
    init_SDL();

    // Create the app after SDL is initialized, but BEFORE anything that needs
    // an OpenGL context.
    App app{"compute", 640, 480};

    init_OpenGL();


    /* ===[ Compute Shader ]=== */
    Program *compute_program = nullptr;
    try
    {
        compute_program = new Program{
            {shader_from_file("shaders/compute.comp", GL_COMPUTE_SHADER)}};
    }
    catch (std::runtime_error const &e)
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
    catch (std::runtime_error const &e)
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
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA32F, app.window_width, app.window_height, 0,
        GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindTexture(GL_TEXTURE_2D, 0);
    // Since the compute shader's output texture must match the window size, we
    // must resize it whenever the app's window size changes.
    app.add_callback(
        SDL_WINDOWEVENT,
        [&texture](SDL_Event event){
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(
                    GL_TEXTURE_2D, 0, GL_RGBA32F, event.window.data1,
                    event.window.data2, 0, GL_RGBA, GL_FLOAT, nullptr);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        });

    /* ===[ Compute Shader Input Data ]=== */
    // Define the scene.
    Material const default_mat{
        1.0f,
        1.0f,
        1.0f,
        15.0f,
        {1.0f, 1.0f, 1.0f}
    };
    Sphere const spheres[] = {
        {
            {0.0f, 0.0f, -2.0f},
            1.0f,
            default_mat,
        },
        {
            {1.55f, 0.0f, -2.0f},
            0.25f,
            default_mat,
        },
    };
    OmniLight const lights[] = {
        {
            {0.0f, 5.0f, -1.0f},
            {0.0f, 1.0f, 0.0f},
        },
    };

    // Create sphere SSBO.
    GLuint sphere_ssbo = 0;
    glGenBuffers(1, &sphere_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphere_ssbo);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER, sizeof(spheres), spheres,
        GL_STATIC_DRAW);
    auto spheres_buffer_binding = glGetProgramResourceIndex(
        compute_program->id(), GL_SHADER_STORAGE_BLOCK, "Spheres");
    if (spheres_buffer_binding == GL_INVALID_INDEX)
    {
        std::cerr << "glGetProgramResourceIndex - no shader storage block "
            "named 'Spheres'\n";
        return EXIT_FAILURE;
    }
    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER, spheres_buffer_binding, sphere_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Create lights SSBO.
    GLuint light_ssbo = 0;
    glGenBuffers(1, &light_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_ssbo);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER, sizeof(lights), lights,
        GL_STATIC_DRAW);
    auto lights_buffer_binding = glGetProgramResourceIndex(
        compute_program->id(), GL_SHADER_STORAGE_BLOCK, "Lights");
    if (lights_buffer_binding == GL_INVALID_INDEX)
    {
        std::cerr << "glGetProgramResourceIndex - no shader storage block "
            "named 'Lights'\n";
        return EXIT_FAILURE;
    }
    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER, lights_buffer_binding, light_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


    /* ===[ Main Loop ]=== */
    for (; app.running;)
    {
        // Handle user inputs.
        app.input();

        /* ===[ Execute Compute Shader ]=== */
        // Use the compute shader.
        compute_program->use();
        // Set output texture uniform.
        glActiveTexture(GL_TEXTURE0);
        try{
            compute_program->setUniformI("outputImg", 0);
        }
        catch (std::runtime_error &e){
            std::cerr << e.what() << "\n";
        }
        // Set ambient color uniform.
        try{
            compute_program->setUniformVec3(
                "ambientColor", glm::vec3{0.0f, 0.05f, 0.1f});
        }
        catch (std::runtime_error &e){
            std::cerr << e.what() << "\n";
        }
        // Set blank color.
        try{
            compute_program->setUniformVec3(
                "blankColor", glm::vec3{0.2f, 0.0f, 0.2f});
        }
        catch (std::runtime_error &e){
            std::cerr << e.what() << "\n";
        }
        // Bind the sphere SSBO.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphere_ssbo);
        // Bind the light SSBO.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_ssbo);
        // Run the compute shader.
        glDispatchCompute(
            (GLuint)app.window_width, (GLuint)app.window_height, 1);
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
        visual_program->setUniformI("tex", 0);
        // Render the screenquad.
        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices)/sizeof(*vertices));

        // Refresh the screen.
        app.render();
    }

    // Cleanup OpenGL data.
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &sphere_ssbo);
    glDeleteBuffers(1, &light_ssbo);

    // Shutdown SDL.
    SDL_Quit();

    return EXIT_SUCCESS;
}
