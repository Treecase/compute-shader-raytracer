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


Texture::Texture()
:   _id{new GLuint{0}, _texture_delete}
{
    glGenTextures(1, _id.get());
}

GLuint Texture::id() const
{
    return *_id;
}

void Texture::bind(GLenum target) const
{
    glBindTexture(target, *_id);
}
