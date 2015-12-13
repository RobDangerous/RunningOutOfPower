#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D texcoord;
varying vec2 texCoord;

void kore() {
	gl_FragColor = texture2D(texcoord, texCoord);
}
