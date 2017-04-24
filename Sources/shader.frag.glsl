#version 450

uniform sampler2D texcoord;
in vec2 texCoord;
out vec4 FragColor;

void main() {
	FragColor = texture(texcoord, texCoord);
}
