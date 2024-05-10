#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform float size;
layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    gl_PointSize = size;
}