#pragma once

#include <cmath>
#include <vector>

#include "algebra3.h"
#include "global.h"

#include "LoadImage.h"
#include "BSpline.h"
#include "algebra3.h"


#define RENDER_WIDTH 600
#define RENDER_HEIGHT 600
#define SHADOW_MAP_RATIO 2

class edge;
class vertex;
class face;

class vertex {
public:
	vertex(vec3 v1) {
		v = v1;
		texCoord = vec2(0, 0);
	}

	vec3 v;
	vec2 texCoord;
	vector<edge*> edges;
	vector<edge*> new_edges;
	vector<face*> faces;
};

class edge {
public:
	vertex * a;
	vertex * b;
	vector<face*> faces;
	face * oldface;
	void swapVertices(){
		vertex * c = a;
		a = b;
		b = c;
	}
	edge(vertex * v1, vertex * v2) {
		a = v1;
		b = v2;
		bool isIn = false;
		for (unsigned int i =0; i < a->edges.size(); i++) {
			if ((*a->edges[i]) == (*this)) {
				isIn = true;
			}
		}
		if (!isIn) {
			a->edges.push_back(this);
		}
		for (unsigned int i =0; i < b->edges.size(); i++) {
			if ((*b->edges[i]) == (*this)) {
				isIn = true;
			}
		}
		if (!isIn) {
			b->edges.push_back(this);
		}
	}
	double length(){
		vec3 l = a->v - b->v;
		return l.length();
	}
	vec3 midpoint() {
		return (a->v + b->v)/2;
	}
	face * otherface(face * f) {
		if (faces[0] == f) {
			return faces[1];
		} else {
			return faces[0];
		}
	}
	vertex * othervertex(vec3 v) {
		if (a->v == v) return b;
		else return a;
	}
	bool operator==(const edge &other) {
		return ((a->v == other.a->v) && (b->v == other.b->v)) || ((a->v == other.b->v) && (b->v == other.a->v));
	}
};

class face {
public:
	int subdivideLevel;
	vector<edge*> edges;
	vector<edge*> new_edges;
	vec3 texA;
	vec3 texB;
	vec3 texC;
	vec3 texD;
	face(edge * a, edge * b, edge * c, edge * d) {
		edges.push_back(a);
		edges.push_back(b);
		edges.push_back(c);
		edges.push_back(d);

		a->faces.push_back(this);
		b->faces.push_back(this);
		c->faces.push_back(this);
		d->faces.push_back(this);

		subdivideLevel = 0;
	}
	vec3 centroid() {
		return (edges[0]->midpoint() + edges[1]->midpoint() + edges[2]->midpoint() + edges[3]->midpoint()) / 4;
	}
	void getState(vec3 pos, bool &s1, bool &s2, bool &s3, bool &s4) {
		vec3 n = centroid();
		char dir = 'a';
		if (ae(n[1], 4, -2)) dir = 'u'; //up
		else if (ae(n[1], 2, -4)) dir = 'd'; //down
		else if (ae(n[0], 4, -2)) dir = 'r'; //right
		else if (ae(n[0], 2, -4)) dir = 'l'; //left
		else if (ae(n[2], 4, -2)) dir = 'f'; //forward
		else if (ae(n[2], 2, -4)) dir = 'b'; //backward

		bool s[4];
		for (unsigned int i=0; i < edges.size(); i++){
			switch(dir){
				case 'u':
				case 'd':
					s[i] = (pos[0] - edges[i]->a->v[0]) / (pos[2] - edges[i]->a->v[2]) < 
						(edges[i]->midpoint()[0] - edges[i]->a->v[0]) / (edges[i]->midpoint()[2] - edges[i]->a->v[2]);
				break;
				case 'l':
				case 'r':
					s[i] = (pos[1] - edges[i]->a->v[1]) / (pos[2] - edges[i]->a->v[2]) < 
						(edges[i]->midpoint()[1] - edges[i]->a->v[1]) / (edges[i]->midpoint()[2] - edges[i]->a->v[2]);
					break;
				case 'f':
				case 'b':
					s[i] = (pos[1] - edges[i]->a->v[1]) / (pos[0] - edges[i]->a->v[0]) < 
						(edges[i]->midpoint()[1] - edges[i]->a->v[1]) / (edges[i]->midpoint()[0] - edges[i]->a->v[0]);
					break;
			}
		}
		s1 = s[1]; s2 = s[2]; s3 = s[3]; s4 = s[4];
	}
	bool ae(float a, float b1, float b2){ //approximately equal
		float tolerance = 0.5;
		return (abs(a-b1) < tolerance) || (abs(a-b2) < tolerance);
	}
	vec3 fix(float n, vec3 v){
		if (n > 0) return v;
		else return -v;
	}
	vec3 normal() {
		vec3 n = centroid();
		char dir = 'a';
		if (ae(n[1], 4, -2)) dir = 'u'; //up
		else if (ae(n[1], 2, -4)) dir = 'd'; //down
		else if (ae(n[0], 4, -2)) dir = 'r'; //right
		else if (ae(n[0], 2, -4)) dir = 'l'; //left
		else if (ae(n[2], 4, -2)) dir = 'f'; //forward
		else if (ae(n[2], 2, -4)) dir = 'b'; //backward
		n = ((edges[0]->b->v - edges[0]->a->v)^(edges[1]->b->v - edges[1]->a->v)).normalize();
		switch(dir){
			case 'u':
				return fix(n[1], n); break;
			case 'd':
				return fix(-n[1], n); break;
			case 'r':
				return fix(n[0], n); break;
			case 'l':
				return fix(-n[0], n); break;
			case 'f':
				return fix(n[2], n); break;
			case 'b':
				return fix(-n[2], n); break;
		}
		return n;
	}
	vec3 forward() { //return the vector resulting from the centroid to vec3(100,100,100) projected onto the plane
		vec3 projectedPoint = vec3(100, 100, 100) - normal() * ( (normal() * vec3(100,100,100) - normal() * centroid()) / normal().length2());
		return (projectedPoint - centroid());

	}
	void render() {
		
		double scale = 1.0;
		glColor3f(1, 1, 1);
			
		vec3 n = this->centroid();

		vector <edge *> temp_edges;
		temp_edges.push_back(edges[0]);
		temp_edges.push_back(edges[1]);
		temp_edges.push_back(edges[2]);
		temp_edges.push_back(edges[3]);

		for (int i = 1; i < 4; i++) {
			for (int j = i-1; j >= 0; j--) {
				if ((temp_edges[i]->a->v == temp_edges[j]->a->v || temp_edges[i]->b->v == temp_edges[j]->b->v)) {
					temp_edges[i]->swapVertices();
				}
			}
		}

		vec3 v1 = temp_edges[0]->a->v;
		vec3 v2 = temp_edges[1]->a->v;
		vec3 v3 = temp_edges[2]->a->v;
		vec3 v4 = temp_edges[3]->a->v;
		
		vec3 diff1 = v2 - v1;
		vec3 diff2 = v2 - v4;
		vec3 diff3 = v2 - v3;

		vertex * t1 = temp_edges[0]->a;
		vertex * t2 = temp_edges[1]->a;
		vertex * t3 = temp_edges[2]->a;
		vertex * t4 = temp_edges[3]->a;

		//cout << endl;

		glBegin(GL_QUAD_STRIP);
		double val = abs((v2 - v1) * (v2 - v4));
		if (val >= 0 && val <= .99) {
			//cout << "one: 1 2 3 4" << endl;
			glNormal3d(n[0], n[1], n[2]);
			if (subdivideLevel == 0) {
				glTexCoord2f(0, 0);
				t1->texCoord = vec2(0, 0);
			} else {
				glTexCoord2f(t1->texCoord[0], t1->texCoord[1]);										
				glTexCoord2f(0, 0);
			}
			glVertex3f(v1[0], v1[1], v1[2]);
			
			glNormal3d(n[0], n[1], n[2]);
			if (subdivideLevel == 0) {
				glTexCoord2f(0, 1);
				t2->texCoord = vec2(0, 1);
			} else {
				glTexCoord2f(t2->texCoord[0], t2->texCoord[1]);										
				glTexCoord2f(0, 1);
			}
			glVertex3f(v2[0], v2[1], v2[2]);			

			glNormal3d(n[0], n[1], n[2]);
			if (subdivideLevel == 0) {
				glTexCoord2f(1, 0);
				t3->texCoord = vec2(1, 0);
			} else {
				glTexCoord2f(t3->texCoord[0], t3->texCoord[1]);										
				glTexCoord2f(1, 0);
			}
			glVertex3f(v3[0], v3[1], v3[2]);

			glNormal3d(n[0], n[1], n[2]);
			if (subdivideLevel == 0) {
				glTexCoord2f(1, 1);
				t3->texCoord = vec2(1, 1);
			} else {
				glTexCoord2f(t4->texCoord[0], t4->texCoord[1]);										
				glTexCoord2f(1, 1);
			}
			glVertex3f(v4[0], v4[1], v4[2]);
		} else {
			val = abs((v2 - v1) * (v2 - v3));
			if (val >= 0 && val <= .99) {
				//cout << "two: 1 2 4 3" << endl;
				glNormal3d(n[0], n[1], n[2]);
				if (subdivideLevel == 0) {
					glTexCoord2f(0, 0);
					t1->texCoord = vec2(0, 0);
				} else {
					glTexCoord2f(t1->texCoord[0], t1->texCoord[1]);										
					glTexCoord2f(0, 0);
				}
				glVertex3f(v1[0], v1[1], v1[2]);
				
				glNormal3d(n[0], n[1], n[2]);
				if (subdivideLevel == 0) {
					glTexCoord2f(0, 1);
					t2->texCoord = vec2(0, 1);
				} else {
					glTexCoord2f(t2->texCoord[0], t2->texCoord[1]);										
					glTexCoord2f(0, 1);
				}
				glVertex3f(v2[0], v2[1], v2[2]);			

				glNormal3d(n[0], n[1], n[2]);
				if (subdivideLevel == 0) {
					glTexCoord2f(1, 0);
					t4->texCoord = vec2(1, 0);
				} else {
					glTexCoord2f(t4->texCoord[0], t4->texCoord[1]);										
					glTexCoord2f(1, 0);
				}
				glVertex3f(v4[0], v4[1], v4[2]);

				glNormal3d(n[0], n[1], n[2]);
				if (subdivideLevel == 0) {
					glTexCoord2f(1, 1);
					t3->texCoord = vec2(1, 1);
				} else {
					glTexCoord2f(t3->texCoord[0], t3->texCoord[1]);										
					glTexCoord2f(1, 1);
				}
				glVertex3f(v3[0], v3[1], v3[2]);
			} else {
				//cout << "three: 1 3 4 2" << endl;
				glNormal3d(n[0], n[1], n[2]);
				if (subdivideLevel == 0) {
					glTexCoord2f(0, 0);
					t1->texCoord = vec2(0, 0);
				} else {
					glTexCoord2f(t1->texCoord[0], t1->texCoord[1]);										
					glTexCoord2f(0, 0);
				}
				glVertex3f(v1[0], v1[1], v1[2]);
				
				glNormal3d(n[0], n[1], n[2]);
				if (subdivideLevel == 0) {
					glTexCoord2f(0, 1);
					t3->texCoord = vec2(0, 1);
				} else {
					glTexCoord2f(t3->texCoord[0], t3->texCoord[1]);										
					glTexCoord2f(0, 1);
				}
				glVertex3f(v3[0], v3[1], v3[2]);			

				glNormal3d(n[0], n[1], n[2]);
				if (subdivideLevel == 0) {
					glTexCoord2f(1, 0);
					t4->texCoord = vec2(1, 0);
				} else {
					glTexCoord2f(t4->texCoord[0], t4->texCoord[1]);										
					glTexCoord2f(1, 0);
				}
				glVertex3f(v4[0], v4[1], v4[2]);

				glNormal3d(n[0], n[1], n[2]);
				if (subdivideLevel == 0) {
					glTexCoord2f(1, 1);
					t2->texCoord = vec2(1, 1);
				} else {
					glTexCoord2f(t2->texCoord[0], t2->texCoord[1]);										
					glTexCoord2f(1, 1);
				}
				glVertex3f(v2[0], v2[1], v2[2]);
			}
		}
	glEnd();
	}
};

class ccMap 
{
public:
	void renderCube(GLuint depthTextureId);
	void makeFace(int v1, int v2, int v3, int v4);
	void makeTube(int v1, int v2, int v3, int v4);
	void connect(int v1, int v2, int v3, int v4);
	void subdivide();
	edge * push(edge * e);
	face * closestFace(vec3 pos);
	ccMap(int face, string filename, string vertShader, string fragmentShader);
	~ccMap(void);

	vector<vertex *> vertices;
	vector<edge*> edges;
	vector<face*> faces;

	bool shadersOn;
	int subdivLevel;

private:
	GLuint texture, normalMap, heightMap; // texture data
    int lengthRepeats, widthRepeats;
    
    bool shadersFailed;
    GLhandleARB program; // shaders
    GLint tangentAttrib, bitangentAttrib; // tangent space for uploading to shaders
    GLint bumpMapEnabledUniform, textureMapEnabledUniform, shadowMapUniform;

    bool bumpMapEnabled, textureMapEnabled;
    
};