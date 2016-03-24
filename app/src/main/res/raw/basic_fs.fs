#version 100

uniform sampler2D myTexture;
uniform vec4 lumpos;
varying vec2 vsoTexCoord;
varying vec3 vsoNormal;
varying vec4 vsoModPosition;
varying vec4 vsoPosition;

//mediump vec4 gl_FragColor;
precision mediump float;

void main(void) {

  vec3 lum = normalize(vsoModPosition.xyz - lumpos.xyz);
  gl_FragColor = texture2D(myTexture, vsoTexCoord);
  

}
