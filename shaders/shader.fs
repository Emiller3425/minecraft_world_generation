#version 330 core
out vec4 FragColor;
in vec3 ourColor;
in vec3 ourPosition;
uniform vec3 timeColor;
void main()
{
    vec3 finalColor = ourColor * timeColor;
    FragColor = vec4(ourPosition, 1.0);
}