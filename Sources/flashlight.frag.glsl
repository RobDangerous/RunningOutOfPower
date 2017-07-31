#version 450

uniform sampler2D tex;
in vec2 texCoord;
in vec4 color;
out vec4 FragColor;

uniform float aspect;
//uniform float angle;
uniform vec2 player;
uniform vec2 mouse;
uniform int anim;
uniform vec2 lights[8];
uniform float energy;
uniform float top;
uniform float bottom;

// noise from https://www.shadertoy.com/view/Msf3WH

vec2 hash( vec2 p ) // replace this by something better
{
	p = vec2( dot(p,vec2(127.1,311.7)),
			  dot(p,vec2(269.5,183.3)) );

	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

float noise(vec2 p) {
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;

	vec2 i = floor( p + (p.x+p.y)*K1 );
	
    vec2 a = p - i + (i.x+i.y)*K2;
    vec2 o = step(a.yx,a.xy);    
    vec2 b = a - o + K2;
	vec2 c = a - 1.0 + 2.0*K2;

    vec3 h = max( 0.5-vec3(dot(a,a), dot(b,b), dot(c,c) ), 0.0 );

	vec3 n = h*h*h*h*vec3( dot(a,hash(i+0.0)), dot(b,hash(i+o)), dot(c,hash(i+1.0)));

    return dot( n, vec3(70.0) );
}

float fractalNoise(vec2 uv) {
	float f = 0.0;
	uv *= 5.0;

    mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 );
	f  = 0.5000*noise( uv ); uv = m*uv;
	f += 0.2500*noise( uv ); uv = m*uv;
	f += 0.1250*noise( uv ); uv = m*uv;
	f += 0.0625*noise( uv ); uv = m*uv;

	f = 0.5 + 0.5*f;
	
    //f *= smoothstep( 0.0, 0.005, abs(p.x-0.6) );

    return f;
}

float easeOutQuad(float t) {
	return t*(2-t);
}

float easeOutQuart(float t) {
	return 1 - (--t) * t * t * t;
}

float noEase(float t) {
	return t > 0.2 ? 1.0 : 0.0;
}

#define PI 3.14159265359

void main() {
	float tx = (texCoord.x * 2 - 1) * aspect;
	float ty = texCoord.y * 2 - 1;
	tx = texCoord.x - player.x;
	ty = texCoord.y - player.y;

	float angle = atan(mouse.y - player.y, mouse.x - player.x) - PI / 2.0;

	float start = angle - 0.4;
	float end = angle + 0.4;
	float tangle = atan(ty, tx);
	float tdistance = sqrt(tx * aspect * tx * aspect + ty * 0.8 * ty * 0.8);

	float scale = 1.0;// - clamp(tdistance * 1.5, 0.0, 1.0);
	
	float angledif = atan(cos(tangle - angle), sin(tangle - angle));

	scale *= 1.0 - clamp(abs(angledif) * 1.5 / energy, 0.0, 1.0);

	if (texCoord.y < top || texCoord.y > bottom) {
		scale = 0.0;
	}

	scale = easeOutQuart(clamp(scale, 0.0, 1.0));

	scale += easeOutQuart(1.0 - clamp(tdistance * 5.0, 0.0, 1.0));

	for (int i = 0; i < 8; ++i) {
		float difx = (texCoord.x - lights[i].x) * aspect;
		float dify = texCoord.y - lights[i].y;
		float dist = sqrt(difx * difx + dify * dify);
		scale += easeOutQuad(1.0 - clamp(dist * 7.5, 0.0, 1.0));
	}

	scale = clamp(scale, 0.0, 1.0);

	vec4 texcolor = texture(tex, texCoord + vec2(sin(anim / 20.0 + texCoord.y * 8) * 0.02 * (1.0 - scale), 0.0)) * color;
	texcolor.rgb *= color.a;
	
	texcolor.r *= scale;
	texcolor.g *= scale;
	texcolor.b *= clamp(fractalNoise(texCoord - anim / 500.0) * fractalNoise(texCoord + anim / 500.0) * 1.5 + scale, 0.0, 1.0);
	
	/*float mx = texCoord.x - mouse.x;
	float my = texCoord.y - mouse.y;
	float mdistance = sqrt(mx * mx + my * my);
	if (mdistance < 0.01) {
		texcolor = vec4(1.0, 0.0, 0.0, 1.0);
	}*/

	FragColor = texcolor;
}
