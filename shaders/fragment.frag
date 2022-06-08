#version 430 core
// fragment.frag - ScreenQuad fragment shader.
// Copyright (C) 2022 Trevor Last

in vec2 fTexCoords;

out vec4 FragColor;

uniform sampler2D tex;


void main()
{
    FragColor = texture(tex, fTexCoords);
}
