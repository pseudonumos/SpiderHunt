// this is the texture map data we can sample
uniform sampler2D textureMap, heightMap, normalMap, shadowMap;

// these are the texture and bump mapping settings we can toggle
uniform bool bumpMapEnabled, textureMapEnabled;

// These varying values are interpolated over the surface
varying vec4 v;
varying vec3 t;
varying vec3 b;
varying vec3 n;
varying mat3 transform;
varying vec4 ShadowCoord;

void main()
{
    // sample from a texture map
    vec4 texcolor;
    if (textureMapEnabled) {
        texcolor = texture2D(textureMap,gl_TexCoord[0].st);
    } else {
        texcolor = vec4(1,1,1,1);
    }
    // HINT: you can also sample from the height map for parallax bump mapping (extra credit option)
    
    // sample from a normal map
    vec3 normal;
    if (bumpMapEnabled) {
	normal = normalize((transform * normalize(texture2D(normalMap, gl_TexCoord[0].xy).xyz)) * 2.0 - 1.0);
    } else {
	normal = n;
    }
    
    // normalize the eye, light, and reflection vectors
    vec3 e = normalize(v).xyz; // in eye space, eye is at origin

    // shadow
	vec4 shadowCoordinateWdivide = ShadowCoord / ShadowCoord.w ;
	
	// Used to lower moiré pattern and self-shadowing
	shadowCoordinateWdivide.z += 0.0005;
	
	
	float distanceFromLight = texture2D(shadowMap,shadowCoordinateWdivide.st).z;
	
	
 	float shadow = 1.0;
 	if (ShadowCoord.w > 0.0)
 		shadow = distanceFromLight < shadowCoordinateWdivide.z ? 0.5 : 1.0 ;

    // light sources are in eye space; it is okay to assume directional light
    vec3 l = normalize(gl_LightSource[0].position.xyz);
    vec3 r = reflect(-l,normal); // reflect function assumes vectors are normalized
    
    // setting up your light parameters in the shader is fine
    // don't worry too much about matching the opengl lighting settings
    // the fixed function uses the blinn-phong half-angle formula anyway
    // while we've asked you to use the standard phong model
    float p = 20.0;
    vec4 cr = texcolor; //Color of the object
    vec4 cl = gl_LightSource[0].diffuse; //Diffuse
    vec4 cs = gl_LightSource[0].specular; //Specular
    //vec4 ca = vec4(.2,.2,.2,1.0); //Ambient
    vec4 ca = gl_LightSource[0].ambient; //Ambient


    // the below is fine for a lighting equation
    vec4 color;
    color = cr * (ca + 
	cl * max(0.0,dot(normal,l))) + 
	cl * pow(max(0.0,dot(r,normal)),p);


    vec3 l2 = normalize(gl_LightSource[1].position.xyz);
    vec3 r2 = reflect(-l, normal);
    
    vec4 c2r = texcolor; //Color of the object
    vec4 c2l = gl_LightSource[1].diffuse; //Diffuse
    vec4 c2s = gl_LightSource[1].specular; //Specular
    vec4 c2a = gl_LightSource[1].ambient; //Ambient

    vec4 color2;
    color2 = c2r * (c2a +
        c2l * max(0.0, dot(normal, l2))) +
        c2l * pow(max(0.0, dot (r2, normal)), p); 

    // set the output color to what we've computed
    gl_FragColor = (color/2 + color2/2) * shadow;
	
}
