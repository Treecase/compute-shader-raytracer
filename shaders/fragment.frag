#version 430 core
// fragment.frag - ScreenQuad fragment shader.
// Copyright (C) 2022 Trevor Last

in vec2 fTexCoords;

out vec4 FragColor;

uniform sampler2D tex;


/**
 * Calculate the 4x4 Bayer dithered value of the given pixel.
 *
 * IN
 * | value: brightness of the pixel.
 * | pos: the position of of the pixel in the image.
 */
float bayerDither(in float value, in ivec2 pos)
{
    // Algorithm from: https://en.wikipedia.org/wiki/Ordered_dithering
    // 4x4 threshold map.
    const mat4 M = 0.0625 * mat4(
         0,  8,  2, 10,
        12,  4, 14,  6,
         3, 11,  1,  9,
        15,  7, 13,  5);
    const float threshold = M[pos.x % 4][pos.y % 4];
    // return value + (threshold - 0.5) + 0.5;
    return float(value >= threshold);
}


void main()
{
    const ivec2 pixelPos = ivec2(gl_FragCoord.xy);
    const vec4 color = texture(tex, fTexCoords);

    // Dithering effect (for fun :]).
    const float brightness = (color.r + color.g + color.b) / 3.0;
    const vec4 dithered = vec4(color.rgb * bayerDither(brightness, pixelPos), 1.0);

    FragColor = dithered;
    // FragColor = color;
}
