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

#include <memory>
#include <string>
#include <vector>


/* ===[ Deleters ]=== */
/** Deleter for Shader objects. (For use with shared_ptr and co.) */
void _shader_delete(GLuint *shader);

/** Deleter for Program objects. (For use with shared_ptr and co.) */
void _program_delete(GLuint *program);

/** Deleter for Buffer objects. (For use with shared_ptr and co.) */
void _buffer_delete(GLuint *buffer);

/** Deleter for VertexArray objects. (For use with shared_ptr and co.) */
void _vertex_array_delete(GLuint *vertexarray);

/** Deleter for Texture objects. (For use with shared_ptr and co.) */
void _texture_delete(GLuint *texture);



/**
 * OpenGL shader object.
 */
class Shader
{
private:
    std::shared_ptr<GLuint> const _id;
public:
    Shader(GLenum type, std::string source);

    /** Get the shader's id. */
    GLuint id() const;
};

/**
 * OpenGL program object.
 */
class Program
{
private:
    std::shared_ptr<GLuint> const _id;
    GLint _getUniformLocation(std::string uniform) const;
public:
    Program(std::vector<Shader> shaders);

    /** Use the program. */
    void use() const;
    /** Get the program's id. */
    GLuint id() const;
    /** Set a floating-point uniform. */
    void setUniformF(std::string uniform, GLfloat value) const;
    /** Set an integer uniform. */
    void setUniformI(std::string uniform, GLint value) const;
    /** Set a vector3 uniform. */
    void setUniformVec3(std::string uniform, glm::vec3 value) const;
};

/**
 * OpenGL buffer object.
 */
class Buffer
{
private:
    std::shared_ptr<GLuint> const _id;
public:
    Buffer();

    /** Get the buffer's id. */
    GLuint id() const;

    /** Bind the buffer. */
    void bind(GLenum target) const;
};

/**
 * OpenGL vertex array object.
 */
class VertexArray
{
private:
    std::shared_ptr<GLuint> const _id;
public:
    VertexArray();

    /** Get the vertex array's id. */
    GLuint id() const;

    /** Bind the vertex array. */
    void bind() const;
};

/**
 * OpenGL texture object.
 */
class Texture
{
private:
    std::shared_ptr<GLuint> const _id;
public:
    Texture();

    /** Get the texture's id. */
    GLuint id() const;

    /** Bind the texture. */
    void bind(GLenum target) const;
};

#endif
