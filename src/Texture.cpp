/**
 * Texture.cpp - OpenGL texture object.
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


void _texture_delete(GLuint *texture)
{
    glDeleteBuffers(1, texture);
    delete texture;
}


Texture::Texture(GLenum type, std::string label)
:   _id{new GLuint{0}, _texture_delete}
,   _type{type}
{
    glGenTextures(1, _id.get());
    glBindTexture(type, *_id);
    if (!label.empty())
    {
        glObjectLabel(GL_TEXTURE, *_id, (GLsizei)label.size(), label.c_str());
    }
}

GLuint Texture::id() const
{
    return *_id;
}

GLuint Texture::type() const
{
    return _type;
}

void Texture::bind() const
{
    glBindTexture(_type, *_id);
}

void Texture::unbind() const
{
    glBindTexture(_type, 0);
}

void Texture::setParameter(GLenum pname, GLint param)
{
    glTexParameteri(_type, pname, param);
}
