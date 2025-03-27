#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D textureSide;
uniform sampler2D textureTop;
uniform sampler2D textureBottom;

void main()
{       
        vec4 texColor = texture(textureSide, TexCoord);

        if (texColor.a < 0.1)
                discard;

        FragColor = texColor;
}