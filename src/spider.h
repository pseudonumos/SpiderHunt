#pragma once

#include <cmath>
#include <vector>

#include "algebra3.h"
#include "global.h"

#include "LoadImage.h"
#include "BSpline.h"
#include "algebra3.h"
#include "ccMap.h"

class bullet {
public:
	vec3 start; vec3 dir; vec3 current; int type;
	bullet(int t) {
		type =t;
	}
	void render(){
		glPushMatrix();
		if (type ==10){
		glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		glVertex3f(current[0], current[1], current[2]);
		current[0] = current[0] + dir[0]; current[1] = current[1] + dir[1]; current[2] = current[2] + dir[2];
		glVertex3f(current[0], current[1], current[2]);
		glEnd();
		} else{
		current[0] = current[0] + dir[0]; current[1] = current[1] + dir[1]; current[2] = current[2] + dir[2];
		current[0] = current[0] + dir[0]; current[1] = current[1] + dir[1]; current[2] = current[2] + dir[2];
		glTranslatef(current[0], current[1], current[2]);
		glutSolidSphere(0.05, 3, 3);
		//current[0] = current[0] + dir[0]; current[1] = current[1] + dir[1]; current[2] = current[2] + dir[2];
		}
		glPopMatrix();

	}
};

class Spider {
public:
	bool render(bool showHealth = false, bullet * b = NULL, ccMap * cube = NULL, int num = 0, GLuint depthTextureId = NULL);
	Spider(face * f, double t_init = 0, vec3 offset = vec3(0, 0, 0), string filename = "", string vertShader="", string fragmentShader="");
	~Spider(void);
	bool alive;
private:
	double angle;
	vec3 loc;
	int health;
	face * myFace;
	vec3 o;
	double t;

	GLuint texture, normalMap, heightMap; // texture data
    
    bool shadersFailed;
    GLhandleARB program; // shaders
    GLint tangentAttrib, bitangentAttrib; // tangent space for uploading to shaders
    GLint bumpMapEnabledUniform, textureMapEnabledUniform, shadowMapUniform;

    bool bumpMapEnabled, textureMapEnabled;
    bool shadersOn;


};