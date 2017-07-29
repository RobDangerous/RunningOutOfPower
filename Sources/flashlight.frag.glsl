#version 450

uniform sampler2D tex;
in vec2 texCoord;
in vec4 color;
out vec4 FragColor;

void main() {
	vec4 texcolor = texture(tex, texCoord) * color;
	texcolor.rgb *= color.a;
	texcolor.r = 0;
	texcolor.g = 0;
	FragColor = texcolor;
}
