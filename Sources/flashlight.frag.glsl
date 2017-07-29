#version 450

uniform sampler2D tex;
in vec2 texCoord;
in vec4 color;
out vec4 FragColor;

uniform float aspect;
//uniform float angle;
uniform vec2 player;
uniform vec2 mouse;

float easeOutQuart(float t) {
	return 1 - (--t) * t * t * t;
}

void main() {
	vec4 texcolor = texture(tex, texCoord) * color;
	texcolor.rgb *= color.a;
	
	float tx = (texCoord.x * 2 - 1) * aspect;
	float ty = texCoord.y * 2 - 1;
	tx = texCoord.x - player.x;
	ty = texCoord.y - player.y;

	float angle = atan(mouse.y - player.y, mouse.x - player.x);

	float start = angle - 0.4;
	float end = angle + 0.4;
	float tangle = atan(ty, tx);
	float tdistance = sqrt(tx * tx + ty * ty);

	float scale = 1.0 - clamp(tdistance * 1.5, 0.0, 1.0);
	
	scale *= 1.0 - clamp(abs(tangle - angle) * 1.5, 0.0, 1.0);

	scale = easeOutQuart(scale);

	texcolor.r *= scale;
	texcolor.g *= scale;
	//texcolor.b *= scale;
	
	FragColor = texcolor;
}
