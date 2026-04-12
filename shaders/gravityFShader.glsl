#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform vec3 col;
uniform sampler2D tex;

void main()
{
    FragColor = texture(tex, texCoord) * vec4(col, 1.0f);
}