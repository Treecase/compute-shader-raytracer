/**
 * ComputeRaytraceRenderer.hpp - Compute shader raytracer.
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

#ifndef _COMPUTE_RAYTRACE_RENDERER_HPP
#define _COMPUTE_RAYTRACE_RENDERER_HPP

#include "glUtil.hpp"
#include "ShaderStructs.hpp"

#include <vector>


/**
 * Data needed to render something using a Renderer.
 */
struct Scene
{
    std::vector<Material> materials;
    std::vector<Sphere> spheres;
    std::vector<OmniLight> lights;
};


/**
 * Renders Scenes using OpenGL compute shaders.
 */
class ComputeRaytraceRenderer
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

    ComputeRaytraceRenderer(Scene const &scene, GLuint width, GLuint height);

    /** Get the render result. */
    Texture const &getResult() const;

    /** Set the render output dimensions. */
    void setRenderDimensions(GLuint width, GLuint height);

    /** Render the scene. */
    void render() const;
};


/**
 * Displays textures to the screen.
 */
class RenderResultDisplay
{
private:
    static std::vector<GLfloat> const _screenQuadVertices;
    Program const _display;
    VertexArray const _screenQuadVAO;
public:
    bool dithering;

    RenderResultDisplay();

    /** Draw the result to the screen. */
    void draw(Texture const &result) const;
};


#endif
