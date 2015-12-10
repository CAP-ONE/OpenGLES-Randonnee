uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
//uniform mat4 mmat, vmat;
//uniform mat4 projmat;
attribute vec3 vsiPosition;
attribute vec3 vsiNormal;
attribute vec2 vsiTexCoord;

varying vec2 vsoTexCoord;
varying vec2 vsoTexCoord2;
varying vec3 vsoNormal;
varying vec4 vsoModPosition;
varying vec4 vsoPosition;

void main(void) {

    vsoModPosition = modelViewMatrix * vec4(vsiPosition.xyz, 1.0);
    vsoNormal = (transpose(inverse(modelViewMatrix)) * vec4(vsiNormal.xyz, 0.0)).xyz;
    vsoPosition = vec4(vsiPosition.xyz, 1.0)*2.0;
    //gl_Position = projmat * mmat * vec4(vsiPosition.xyz, 1.0);

    //projectionMatrix *= eyePerspective;

    gl_Position = projectionMatrix * modelViewMatrix * vec4(vsiPosition.xyz, 1.0);
    vsoTexCoord = vsiTexCoord;
    vsoTexCoord2 = vsiTexCoord;
  //gl_FogFragCoord = gl_Position.z;
}
