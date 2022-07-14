/**
 * ComputeRaytraceRenderer.cpp - Compute shader raytracer.
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

#include "ComputeRaytraceRenderer.hpp"

#include <fstream>
#include <sstream>


/* ===[ Utility ]=== */

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


/** Load a GLSL Shader from a file. */
Shader shader_from_file(std::string path, GLenum type)
{
    std::ifstream shaderfile{path.c_str()};
    std::stringstream srcstream{};
    srcstream << shaderfile.rdbuf();
    shaderfile.close();
    return Shader{type, srcstream.str(), path};
}


/* ===[ Renderer ]=== */

ComputeRaytraceRenderer::ComputeRaytraceRenderer(Scene const &scene, GLuint width, GLuint height)
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
    glViewport(0, 0, _width, _height);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_debug_message_callback, nullptr);
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

Texture const &ComputeRaytraceRenderer::getResult() const
{
    return _renderResult;
}

void ComputeRaytraceRenderer::setRenderDimensions(GLuint width, GLuint height)
{
    _width = width;
    _height = height;
    _renderResult.bind();
    glTexImage2D(
        _renderResult.type(), 0, GL_RGBA32F, _width, _height, 0, GL_RGBA,
        GL_FLOAT, nullptr);
    _renderResult.unbind();
}

void ComputeRaytraceRenderer::render() const
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


/* ===[ RenderResultDisplay ]=== */

RenderResultDisplay::RenderResultDisplay()
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

void RenderResultDisplay::draw(Texture const &result) const
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
