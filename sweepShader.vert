attribute vec3 tangent;
attribute vec3 bitangent;

// NOTE:
// varying variables are set by the vertex shader
// then interpolated by the harder 
// and the interpolated values are passed to the fragment shader
// so the corresponding varying vec4 v in the fragment shader
// will be the linearly interpolated position of the spot you're shading.
varying vec4 v;

// NOTE:
// we'll pass the TBN matrix down
// in order to do the lighting in eye space
// many people do it in tangent space instead, for efficiency
// but for the sake of exercise we'll do it all in eye space
varying vec3 t;
varying vec3 b;
varying vec3 n;
varying mat3 transform;

varying vec4 ShadowCoord;

void main()
{

    ShadowCoord = gl_TextureMatrix[7] * gl_Vertex;
    // put everything varying in eye space
    v = gl_ModelViewMatrix * gl_Vertex;
    // @TODO: SET t,b,n appropriately
    // HINT: remember normals transform by the gl_NormalMatrix
	t = normalize((gl_ModelViewMatrix * vec4(t, 0)).xyz);
	b = normalize((gl_ModelViewMatrix * vec4(b, 0)).xyz);
	n = normalize(gl_NormalMatrix * gl_Normal);
    // pass down the texture coords
	gl_TexCoord[0] = gl_MultiTexCoord0;
    
    // position by the standard transform
    // as this shader doesn't change geometry
	gl_Position = ftransform();

	transform = mat3(t, b, n);

}