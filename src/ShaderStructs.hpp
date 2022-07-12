/**
 * ShaderStructs.cpp - Compute shader structs.
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

#ifndef _SHADERSTRUCTS_HPP
#define _SHADERSTRUCTS_HPP

#include <GL/glew.h>


/** RGB color. */
typedef GLfloat Color[3];

/** XYZ position. */
typedef GLfloat Position[3];

/**
 * Material.
 *  specular - Specular reflection constant.
 *  diffuse - Diffuse reflection constant.
 *  ambient - Ambient reflection constant.
 *  shininess - How shiny the material is.
 *            (Higher values = smaller specular highlight)
 */
struct Material
{
    GLfloat specular;
    GLfloat diffuse;
    GLfloat ambient;
    GLfloat shininess;
    Color color;
};

/**
 * A Sphere.
 *  position - Center of the sphere.
 *  r - Radius of the sphere.
 *  material - The sphere's material.
 */
struct Sphere
{
    Position position;
    GLfloat r;
    GLint material;
};

/**
 * An omnidirectional light.
 *  position - Center of the light.
 *  color - Color of the light.
 */
struct OmniLight
{
    Position position;
    Color color;
};


#endif
