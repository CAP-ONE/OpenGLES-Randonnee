#version 300 es
uniform sampler2D myTexture;
uniform vec4 lumpos;
in vec2 vsoTexCoord;
in vec3 vsoNormal;
in vec4 vsoModPosition;

out vec4 fragColor;

void main(void) {
  vec3 lum = normalize(vsoModPosition.xyz - lumpos.xyz);
  fragColor = texture(myTexture, vsoTexCoord);
	
}
