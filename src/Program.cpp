/**
 * Program.cpp - OpenGL program object.
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

#include <stdexcept>
#include <string>
#include <vector>


Program::Program(std::vector<Shader> shaders)
:   _id{glCreateProgram()}
{
    for (auto shader : shaders)
    {
        glAttachShader(_id, shader.id());
    }
    glLinkProgram(_id);
    for (auto shader : shaders)
    {
        glDetachShader(_id, shader.id());
    }
    GLint success = 0;
    glGetProgramiv(_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLint infolog_size = 0;
        glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &infolog_size);
        GLchar infolog[infolog_size];
        glGetProgramInfoLog(_id, infolog_size, nullptr, infolog);
        throw std::runtime_error{
            "Program link failed:\n" + std::string{infolog} + "\n"};
    }
}

Program::~Program()
{
    glDeleteProgram(_id);
}

void Program::use()
{
    glUseProgram(_id);
}

GLuint Program::id() const
{
    return _id;
}
