// this is the texture map data we can sample
uniform sampler2D textureMap, heightMap, normalMap;
// Toon Shader Written Using Lighthouse 3d tutorial
varying vec3 normal;
uniform bool bumpMapEnabled, textureMapEnabled;
	void main()
	{
		float intensity;
		vec4 color;
		vec3 n = normalize(normal);
		
		intensity = dot(vec3(gl_LightSource[0].position),n);
		
		if (intensity > 0.95)
			color = vec4(0.9,0.8,0.7,1.0);
		else if (intensity > 0.5)
			color = vec4(0.7,0.4,0.4,1.0);
		else if (intensity > 0.25)
			color = vec4(0.4,0.2,0.2,1.0);
		else
			color = vec4(0.2,0.1,0.1,1.0);
		if (textureMapEnabled) {
			color = color * texture2D(textureMap,gl_TexCoord[0].st);
		}
		gl_FragColor = color;
	} 