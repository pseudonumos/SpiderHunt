// Toon Shader Written Using Lighthouse 3d tutorial

varying vec3 normal;
	
	void main()
	{

		normal = gl_NormalMatrix * gl_Normal;
	
		gl_Position = ftransform();
	} 