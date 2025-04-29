#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D textureSide;
uniform sampler2D textureTop;
uniform sampler2D textureBottom;

uniform vec3 lightColor;
uniform vec3 lightPos;

void main()
{       
        vec4 texColor = texture(textureSide, TexCoord);

        if (texColor.a < 0.1)
                discard;
        // ambient
        float ambientStrength = 0.6f;
        vec3 ambient = ambientStrength * lightColor;

        // diffuse
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        vec3 lighting = ambient + diffuse;

        vec4 result = vec4(lighting * texColor.rgb, texColor.a);

        FragColor = result;
}