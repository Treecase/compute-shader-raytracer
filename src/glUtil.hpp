/**
 * glUtil.hpp - OpenGL utilities.
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

#ifndef _GLUTIL_HPP
#define _GLUTIL_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>


/**
 * OpenGL shader object.
 */
class Shader
{
private:
    GLuint _id;
public:
    Shader(GLenum type, std::string source);
    ~Shader();

    /** Get the shader's id. */
    GLuint id() const;
};

/**
 * OpenGL program object.
 */
class Program
{
private:
    GLuint _id;
    GLint _getUniformLocation(std::string uniform) const;
public:
    Program(std::vector<Shader> shaders);
    ~Program();

    /** Use the program. */
    void use();
    /** Get the program's id. */
    GLuint id() const;
    /** Set a floating-point uniform. */
    void setUniformF(std::string uniform, GLfloat value) const;
    /** Set an integer uniform. */
    void setUniformI(std::string uniform, GLint value) const;
    /** Set a vector3 uniform. */
    void setUniformVec3(std::string uniform, glm::vec3 value) const;
};

#endif
