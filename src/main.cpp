/**
 * main.cpp - Raytracer demo using SDL2.
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
#include "ComputeRaytraceRenderer.hpp"

#include <SDL.h>

#include <functional>
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
    ComputeRaytraceRenderer renderer{
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
    // Keybind to toggle dithering with the spacebar.
    app.add_callback(
        SDL_KEYDOWN,
        [&result_display](SDL_Event event){
            if (event.key.keysym.sym == SDLK_SPACE)
            {
                result_display.dithering = !result_display.dithering;
            }
        }
    );

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
