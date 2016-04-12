#version 300 es

uniform sampler2D myTexture0;
uniform sampler2D myTexture1;
uniform sampler2D myTexture2;
uniform sampler2D myTexture3;
uniform sampler2D myTexture4;
//uniform sampler2D myTexture5;
uniform vec4 lumpos;
uniform vec4 hm;

in vec2 vsoTexCoord;
in vec2 vsoTexCoord2;
in vec3 vsoNormal;
in vec4 vsoModPosition;
in vec4 vsoPosition;

out vec4 fragColor;

void main(void) {

  float scale = vsoPosition.y;

  vec3 lum = normalize(vsoModPosition.xyz - lumpos.xyz);
  float intensity = dot(normalize(vsoNormal),lum);
  float factor = 0.1;

  if( intensity > 0.1  && intensity <= 0.3) factor = 1.0;
  else if ( intensity > 0.3 && intensity <= 0.4) factor = 0.8;
  else if ( intensity > 0.4 && intensity <= 0.7) factor = 0.6;
  else if ( intensity > 0.7 && intensity <= 0.9) factor = 0.4;

  else factor = 0.2;


    //à partir de -1.0 (eau)

   if(scale > 0.3){ //neige
  	fragColor = texture(myTexture4, vsoTexCoord);
  	fragColor *= vec4(factor, factor, factor, 1.0);
    }

  if(scale > -0.0 && scale <= 0.3){
    fragColor = texture(myTexture3, vsoTexCoord); //roche
    fragColor *= vec4(factor, factor, factor, 1.0);
  }

   if(scale > -0.8 && scale <= -0.0){
    fragColor = texture(myTexture2, vsoTexCoord); //herbe
    fragColor *= vec4(factor, factor, factor, 1.0);
  }

   if(scale > -0.9 && scale <= -0.8){ //sable
    fragColor = texture(myTexture1, vsoTexCoord);
    fragColor *= vec4(factor, factor, factor, 1.0);
  }

   if(scale <= -0.9){ //eau
    fragColor = texture(myTexture0, vsoTexCoord);
    fragColor *= vec4(factor, factor, factor, 0.5);
  }



}