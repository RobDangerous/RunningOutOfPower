#version 450

uniform sampler2D tex;
in vec2 texCoord;
in vec4 color;
out vec4 FragColor;

uniform float aspect;
uniform float angle;

void main() {
	vec4 texcolor = texture(tex, texCoord) * color;
	texcolor.rgb *= color.a;
	float tx = (texCoord.x * 2 - 1) * aspect;
	float ty = texCoord.y * 2 - 1;
	float start = angle - 0.4;
	float end = angle + 0.4;
	float tangle = atan(ty, tx);
	if (tx * tx + ty * ty > 0.3 || !(tangle >= start && tangle <= end)) {
		texcolor.r = 0;
		texcolor.g = 0;
	}
	FragColor = texcolor;
}
