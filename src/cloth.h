#pragma once

#include <cmath>
#include <vector>

#include "algebra3.h"
#include "global.h"

#include "LoadImage.h"
#include "BSpline.h"
#include "algebra3.h"

#define K_S 30
#define K_D 1
#define dt .005
#define MASS 1
#define HEIGHT 4
#define WIDTH 4
#define STEP_SIZE .5

class node;

class node {
public:
	
	node(vec3 v) {
		pos = v;
		vel = vec3(0,0,0);
		accel = vec3(0,0,0);
	}

	vector<node *> adjNodes;
	vector<node *> diagNodes;
	vector<node *> flexNodes;

	vec3 getPos() {
		return pos;
	}

	vec3 getSpringForce() {

		vec3 netForce = vec3(0, 0, 0);

		// damper spring forces
		// Structural Spring forces
		for (unsigned int i = 0; i < adjNodes.size(); i++) {
			vec3 delta_x = pos - adjNodes[i]->pos;
			double l = delta_x.length();
			vec3 e = delta_x / l;
			vec3 v1 = vel;
			vec3 v2 = adjNodes[i]->vel;
//			netForce += e*( -1 * K_S * (1 - l) - K_D * (v2 - v1));
			vec3 force = -1 * (K_S * (l - 1) + K_D * ((v1 - v2) * delta_x) / l) * e;
			//cout << "force 1: " << force << endl;
			netForce += force;
		}
		// Shear Spring forces
		for (unsigned int i = 0; i < diagNodes.size(); i++) {
			vec3 delta_x = pos - diagNodes[i]->pos;
			double l = delta_x.length();
			vec3 e = delta_x / l;
			vec3 v1 = vel;
			vec3 v2 = diagNodes[i]->vel;
			vec3 force = -1 * (K_S * (l - 1.414) + K_D * ((v1 - v2) * delta_x) / l) * e;
			// cout << "force 2: " << force << endl;
			netForce += force;
		}
		

		// Flexion Spring forces
		for (unsigned int i = 0; i < flexNodes.size(); i++) {
			vec3 delta_x = pos - flexNodes[i]->pos;
			double l = delta_x.length();
			vec3 e = delta_x / l;
			vec3 v1 = vel;
			vec3 v2 = flexNodes[i]->vel;
			vec3 force = -1 * (K_S * (l - 2.4) + K_D * ((v1 - v2) * delta_x) / l) * e;
			// cout << "force 3: " << force << endl;
			netForce += force;
		}


		return netForce;
	}

	void applyForce(vec3 net) {
		accel = net;
		vel += accel * dt;
		pos += vel * dt;
	}

	vec3 accel;
	vec3 vel;
	vec3 pos;
	vec3 windForce;
	vec3 normal;
};

class Cloth {
public:
	Cloth(string filename, string vertShader, string fragmentShader);
	~Cloth();
	
	void renderCloth(GLuint depthTextureId);

	int getWidth() {
		return width;
	}

	int getHeight() {
		return height;
	}

	void setWindForce();

	vec3 gravity;
	vec3 wind;
	vec3 top;

private:
	node * nodes[HEIGHT][WIDTH];
	static const int height = HEIGHT;
	static const int width = WIDTH;
	double x;
	double y;
	double z;

	GLuint texture, normalMap, heightMap; // texture data
    
    bool shadersFailed;
    GLhandleARB program; // shaders
    GLint tangentAttrib, bitangentAttrib; // tangent space for uploading to shaders
    GLint bumpMapEnabledUniform, textureMapEnabledUniform, shadowMapUniform;

	GLint alpha;

    bool bumpMapEnabled, textureMapEnabled;
    bool shadersOn;


};
