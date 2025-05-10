#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal; 

// Per-instance Model Matrix
layout (location = 3) in mat4 aInstanceModel;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
    {
      // calculate world position for fragment shader
      gl_Position = projection * view * aInstanceModel * vec4(aPos, 1.0);
      FragPos = vec3(model * vec4(aPos, 1.0));

      // calculate world-space normal for fragment shader
      Normal = mat3(aInstanceModel) * aNormal;
      
      TexCoord = aTexCoord;
    }