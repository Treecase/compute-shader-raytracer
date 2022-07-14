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


/** Callback to print OpenGL debug messages. */
void opengl_debug_message_callback(
    GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length, GLchar const *message,
    void const *userParam)
{
    std::cout << "OpenGL: ";
    if (type == GL_DEBUG_TYPE_ERROR)
    {
        std::cout << "** GL ERROR ** ";
    }
    std::cout << message << "\n";
}


/**
 * The application.
 */
class App
{
private:
    std::unordered_map<
        Uint32,
        std::vector<std::function<void(SDL_Event)>>
    > _event_callbacks;
    SDL_Window *const _window;
    SDL_GLContext const _context;
public:
    int window_width,
        window_height;
    bool running;

    /** NOTE: SDL must be initialized before an App can be created! */
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
        init_OpenGL();
        glViewport(0, 0, window_width, window_height);
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(opengl_debug_message_callback, nullptr);
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
    void updateScreen() const
    {
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
    return Shader{type, srcstream.str(), path};
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
    // Enable VSync. First try adaptive, if that's not available, use regular.
    if (SDL_GL_SetSwapInterval(-1)  == -1)
    {
        SDL_GL_SetSwapInterval(1);
    }
}


/**
 * Holds all the data needed to render an image with the raytracer.
 */
struct Scene
{
    std::vector<Material> materials;
    std::vector<Sphere> spheres;
    std::vector<OmniLight> lights;
};


/** Renderers for SceneRenderables. */
class Renderer
{
private:
    Program _compute;
    Texture _renderResult;

    Buffer _spheres;
    Buffer _materials;
    Buffer _lights;

    GLuint _width, _height;

    template<typename T>
    void _initComputeBuffer(
        Buffer &buffer, std::string const &buffer_name,
        std::vector<T> const &data) const
    {
        buffer.bind();
        buffer.buffer(GL_STATIC_DRAW, data);
        // Set the SSBO's binding point.
        auto binding = glGetProgramResourceIndex(
            _compute.id(), GL_SHADER_STORAGE_BLOCK, buffer_name.c_str());
        if (binding == GL_INVALID_INDEX)
        {
            throw std::runtime_error{
                "glGetProgramResourceIndex - no shader storage block named "
                "'Spheres'"};
        }
        glBindBufferBase(buffer.target, binding, buffer.id());
        buffer.unbind();
    }

public:
    glm::vec3 ambientColor;
    glm::vec3 blankColor;
    glm::vec3 eyePosition;
    glm::vec3 eyeForward;
    glm::vec3 eyeUp;
    GLfloat fov;

    Renderer(Scene const &scene, GLuint width, GLuint height)
    :   _compute{
            {shader_from_file("shaders/compute.comp", GL_COMPUTE_SHADER)},
            "ComputeShader"}
    ,   _renderResult{GL_TEXTURE_2D, "RenderResult"}
    ,   _spheres{GL_SHADER_STORAGE_BUFFER, "SphereSSBO"}
    ,   _materials{GL_SHADER_STORAGE_BUFFER, "MaterialSSBO"}
    ,   _lights{GL_SHADER_STORAGE_BUFFER, "LightSSBO"}
    ,   _width{width}
    ,   _height{height}
    {
        /* ===[ Output Texture ]=== */
        _renderResult.bind();
        _renderResult.setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        _renderResult.setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        _renderResult.setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        _renderResult.setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(
            _renderResult.type(), 0, GL_RGBA32F, _width, _height, 0, GL_RGBA,
            GL_FLOAT, nullptr);
        glBindImageTexture(
            0, _renderResult.id(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        _renderResult.unbind();
        /* ===[ Create Scene Data Buffers ]=== */
        // Init Spheres SSBO.
        _initComputeBuffer(_spheres, "Spheres", scene.spheres);
        // Init Materials SSBO.
        _initComputeBuffer(_materials, "Materials", scene.materials);
        // Init Lights SSBO.
        _initComputeBuffer(_lights, "Lights", scene.lights);
    }

    /** Get the render result. */
    Texture const &getResult() const
    {
        return _renderResult;
    }

    /** Set the render output dimensions. */
    void setRenderDimensions(GLuint width, GLuint height)
    {
        _width = width;
        _height = height;
        _renderResult.bind();
        glTexImage2D(
            _renderResult.type(), 0, GL_RGBA32F, _width, _height, 0, GL_RGBA,
            GL_FLOAT, nullptr);
        _renderResult.unbind();
    }

    /** Render the scene. */
    void render() const
    {
        // Use the compute shader.
        _compute.use();
        // Set output texture uniform.
        glActiveTexture(GL_TEXTURE0);
        _renderResult.bind();
        _compute.setUniformS("outputImg", 0);
        // Set ambient color uniform.
        _compute.setUniformS("ambientColor", ambientColor);
        // Set blank color.
        _compute.setUniformS("blankColor", blankColor);
        // Set eye position.
        _compute.setUniformS("eyePosition", eyePosition);
        // Set camera up vector.
        _compute.setUniformS("eyeUp", eyeUp);
        // Set camera forward vector.
        _compute.setUniformS("eyeForward", eyeForward);
        // Set FOV.
        _compute.setUniformS("fov", fov);
        // Run the compute shader.
        glDispatchCompute(_width, _height, 1);
        // Wait for the shader to finish writing to the image.
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
};


/** Displays a render result texture to the screen. */
class RenderResultDisplay
{
private:
    static std::vector<GLfloat> const _screenQuadVertices;
    Program const _display;
    VertexArray const _screenQuadVAO;
public:
    RenderResultDisplay()
    :   _display{
            {   shader_from_file("shaders/vertex.vert", GL_VERTEX_SHADER),
                shader_from_file("shaders/fragment.frag", GL_FRAGMENT_SHADER)},
            "RenderDisplayShader"}
    ,   _screenQuadVAO{"ScreenQuadVAO"}
    {
        /* ===[ Create ScreenQuad ]=== */
        _screenQuadVAO.bind();
        // Create vertex data buffer.
        Buffer vbo{GL_ARRAY_BUFFER, "ScreenQuadVBO"};
        vbo.bind();
        vbo.buffer(GL_STATIC_DRAW, _screenQuadVertices);
        // Link position vertex attribute.
        _screenQuadVAO.enableVertexAttribArray(
            0, 3, GL_FLOAT, 5*sizeof(GLfloat));
        // Link texcoord vertex attribute.
        _screenQuadVAO.enableVertexAttribArray(
            1, 2, GL_FLOAT, 5*sizeof(GLfloat), (3*sizeof(GLfloat)));
        // Unbind vertex data buffer.
        vbo.unbind();
        // Unbind screen quad VAO.
        _screenQuadVAO.unbind();
    }

    /** Draw the result to the screen. */
    void draw(Texture const &result) const
    {
        // Clear the screen.
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Use the screenquad shader.
        _display.use();
        // Bind screenquad VAO.
        _screenQuadVAO.bind();
        // Use the compute output texture as the input texture.
        glActiveTexture(GL_TEXTURE0);
        result.bind();
        _display.setUniformS("tex", 0);
        // Render the screenquad.
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(_screenQuadVertices.size()/5));
    }
};

std::vector<GLfloat> const RenderResultDisplay::_screenQuadVertices{
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


/** Main program body. */
int run(int argc, char *argv[])
{
    /* ===[ Initialization ]=== */
    init_SDL();
    App app{"compute", 640, 480};
    RenderResultDisplay result_display{};

    /* ===[ Scene Definition ]=== */
    Scene const scene{
        {   // materials
            {
                1.0f,
                1.0f,
                1.0f,
                15.0f,
                {1.0f, 1.0f, 1.0f}
            },
        },
        {   // spheres
            {
                {-0.4f, 0.0f, -2.0f},
                1.0f,
                0,
            },
            {
                {1.4f, 0.0f, -2.0f},
                0.25f,
                0,
            },
        },
        {   // lights
            {
                {0.0f, 1.0f, 0.0f},
                {0.9f, 1.0f, 0.9f},
            },
        },
    };

    /* ===[ Create Renderer ]=== */
    Renderer renderer{
        scene, (GLuint)app.window_width, (GLuint)app.window_height};
    renderer.ambientColor = glm::vec3{0.0f, 0.05f, 0.1f};
    renderer.blankColor = glm::vec3{0.2f, 0.0f, 0.2f};
    renderer.eyePosition = glm::vec3{0.0f, 0.0f, 0.0f};
    renderer.eyeForward = glm::vec3{0.0f, 0.0f, -1.0f};
    renderer.eyeUp = glm::vec3{0.0f, 1.0f, 0.0f};
    renderer.fov = glm::radians(90.0f);
    // Since we want the Renderer's output size to match the window's size, we
    // must resize it whenever the app's window size changes.
    app.add_callback(
        SDL_WINDOWEVENT,
        [&renderer](SDL_Event event){
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                renderer.setRenderDimensions(
                    event.window.data1, event.window.data2);
            }
        });

    /* ===[ Main Loop ]=== */
    for (; app.running;)
    {
        // Handle user inputs.
        app.input();

        // Render the scene.
        renderer.render();
        result_display.draw(renderer.getResult());
        app.updateScreen();
    }
    return EXIT_SUCCESS;
}


/**
 * Program entry point. Wraps the whole program in a try-catch so error
 * messages can be displayed.
 */
#ifdef _WIN32
int SDL_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    int r = EXIT_FAILURE;

    try
    {
        r = run(argc, argv);
    }
    catch (std::exception const &e)
    {
        int err = SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR, "Error", e.what(), nullptr);
        if (err != 0)
        {
            std::cerr << e.what() << "\n";
        }
    }

    // Shutdown SDL.
    SDL_Quit();
    return r;
}
