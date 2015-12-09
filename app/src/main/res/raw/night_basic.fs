#version 100

uniform sampler2D myTexture0;
uniform sampler2D myTexture1;
uniform sampler2D myTexture2;
uniform sampler2D myTexture3;
uniform sampler2D myTexture4;
uniform vec4 lumpos;
 
varying vec2 vsoTexCoord;
varying vec3 vsoNormal;
varying vec4 vsoModPosition;
varying vec4 vsoPosition;

mediump vec4 gl_FragColor;

uniform vec4 nColor;

void main(void){

  float scale = vsoPosition.y;
  float factor = 0.2;
  
  if(scale > -10.0 && scale <= 5.0){
  gl_FragColor = texture2D(myTexture0, vsoTexCoord);
  gl_FragColor *= vec4(factor, factor, factor, 1.0);
  }
  
  else if(scale > 5.0 && scale <= 10.0){
  gl_FragColor = texture2D(myTexture1, vsoTexCoord);
  gl_FragColor *= vec4(factor, factor, factor, 1.0);
  }

  else if(scale > 10.0 && scale <= 30.0){
  gl_FragColor = texture2D(myTexture2, vsoTexCoord);
  gl_FragColor *= vec4(factor, factor, factor, 1.0);
  }

  else if(scale > 30.0 && scale <= 50.0){
  gl_FragColor = texture2D(myTexture3, vsoTexCoord);
  gl_FragColor *= vec4(factor, factor, factor, 1.0);
  }
  else{
  gl_FragColor = texture2D(myTexture4, vsoTexCoord);
  gl_FragColor *= vec4(factor, factor, factor, 1.0);
  }

     
}
