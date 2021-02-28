#version 330 core

out vec4 color;

in vec4 vertColour;
in vec2 vertUV;

uniform sampler2D textureSampler;

void main()
{
	//color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	//color = vertColour;

	color = texture(textureSampler, vertUV);
	color.a = 0.4f;
}