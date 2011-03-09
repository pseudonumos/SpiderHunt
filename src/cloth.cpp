#include "cloth.h"

#include "global.h"

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Shaders.h"

namespace { // some anonymous helpers for file reading
    // get a quoted string, unless there's no quote at the start
    // in that case we just get whatever is there.
    string getQuoted(istream &str) {
        string ret;
        char temp;
        str >> temp;
        if (temp != '"') {
            str.putback(temp);
            str >> ret;
        } else {
            getline(str, ret, '"');
        }
        return ret;
    }

    void getValue(istream &str, int &val) {
        int v;
        if (str >> v)
            val = v;
    }
}

Cloth::Cloth(string filename,  string vertProg, string fragProg) {
	// define external forces
	// gravity = 1/MASS * vec3(0, -9.8, 0);
	gravity = 1/MASS * vec3(0, 0, -9.8);
	wind = vec3(.2, .2, 4.2);

	x = -WIDTH/2 * STEP_SIZE;
	y = 3;
	z = -HEIGHT/2 * STEP_SIZE;

	for (int i = 0; i < height; i++ ) {
		for (int j = 0; j < width; j++ ) {
			nodes[i][j] = new node(vec3(x + j*STEP_SIZE, y, z + i*STEP_SIZE));
		}
	}

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			// Structural Springs
			// set adjacent nodes
			if (i != 0) {
				nodes[i][j]->adjNodes.push_back(nodes[i-1][j]);		
			}
			if (j != 0) {
				nodes[i][j]->adjNodes.push_back(nodes[i][j-1]);
			}
			if (i != height - 1) {
				nodes[i][j]->adjNodes.push_back(nodes[i+1][j]);
			}
			if (j != width - 1) {
				nodes[i][j]->adjNodes.push_back(nodes[i][j+1]);
			}

			// Shear Springs
			// set diagonal nodes
			if (i != 0 && j != 0) {
				nodes[i][j]->adjNodes.push_back(nodes[i-1][j-1]);		
			}
			if (i != 0 && j != width - 1) {
				nodes[i][j]->adjNodes.push_back(nodes[i-1][j+1]);
			}
			if (i != height - 1 && j != 0) {
				nodes[i][j]->adjNodes.push_back(nodes[i+1][j-1]);
			}
			if (i != height - 1 && j != width - 1) {
				nodes[i][j]->adjNodes.push_back(nodes[i+1][j+1]);
			}
			
			// Flexion Springs
			// set diagonal and adjacent nodes of length 2
			if (i < height - 2 && j < width - 2) {
				nodes[i][j]->flexNodes.push_back(nodes[i+2][j+2]);
			}
			if (i < height - 2 && j > 1) {
				nodes[i][j]->flexNodes.push_back(nodes[i+2][j-2]);
			}
			if (j < width - 2 && i > 1) {
				nodes[i][j]->flexNodes.push_back(nodes[i-2][j+2]);
			}
			if (i < height - 2) {
				nodes[i][j]->flexNodes.push_back(nodes[i+2][j]);
			}
			if (j < width - 2) {
				nodes[i][j]->flexNodes.push_back(nodes[i][j+2]);
			}
			if (i > 1 && j > 1) {
				nodes[i][j]->flexNodes.push_back(nodes[i-2][j-2]);
			}
			if (i > 1) {
				nodes[i][j]->flexNodes.push_back(nodes[i-2][j]);
			}
			if (j > 1) {
				nodes[i][j]->flexNodes.push_back(nodes[i][j-2]);
			}
			
		}
	}

    // Load the track file
    ifstream f(filename.c_str());
    if (!f) {
        UCBPrint("Sweep", "Couldn't load file " << filename);
        return;
    }
    string line;
    while (getline(f,line)) {
        stringstream linestream(line);
        string op;
        linestream >> op;
        if (op[0] == '#') // comments are marked by # at the start of a line
            continue;
        if (op == "p") { // p marks profile points (2d cross section vertices)
        } else if (op == "v") { // v marks bspline control points with optional azimuth info
        } else if (op == "twist") {
        } else if (op == "azimuth") {
        } else if (op == "texture") {
            string textureFile = getQuoted(linestream);
            loadTexture(textureFile, texture);
        } else if (op == "bump") {
            string bumpFile = getQuoted(linestream);
            loadHeightAndNormalMaps(bumpFile, heightMap, normalMap, .2);
        }
    }

    // compile link and validate shader programs
    shadersFailed = false;
    program = glCreateProgramObjectARB();
    if (!setShader(program, vertProg.c_str(), GL_VERTEX_SHADER_ARB))
        shadersFailed = true;
    if (!setShader(program, fragProg.c_str(), GL_FRAGMENT_SHADER_ARB))
        shadersFailed = true;
    if (!linkAndValidateShader(program))
        shadersFailed = true;

    if (shadersFailed) {
        cout << "Shaders failed to initialize correctly" << endl;
    }

    // start with all the nice settings on
    shadersOn = true;
    bumpMapEnabled = false;
    textureMapEnabled = true;

    // set up variables for the shaders to use
    glUseProgramObjectARB(program);
    glUniform1iARB(glGetUniformLocationARB(program, "textureMap"), 0);
    glUniform1iARB(glGetUniformLocationARB(program, "heightMap"), 1);
    glUniform1iARB(glGetUniformLocationARB(program, "normalMap"), 2);
//    glUniform1iARB(glGetUniformLocationARB(program, "alphaMAp"), 3);
//	alpha = glGetUniformLocationARB(program, "alphaMapEnabled");
    bumpMapEnabledUniform = glGetUniformLocationARB(program, "bumpMapEnabled");
    textureMapEnabledUniform = glGetUniformLocationARB(program, "textureMapEnabled");
    tangentAttrib = glGetAttribLocationARB(program, "tangent");
    bitangentAttrib = glGetAttribLocationARB(program, "bitangent");

}


Cloth::~Cloth() {
}

void Cloth::setWindForce() {
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			nodes[i][j]->windForce = vec3(0, 0, 0);
		}
	}


	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			vec3 force = vec3(0, 0, 0);
			vec3 p = nodes[i][j]->pos;
			if (i != height - 1 && j != width - 1) {
				vec3 v1 = nodes[i+1][j]->pos;
				vec3 v2 = nodes[i][j+1]->pos;
				vec3 n = (v1 - p) ^ (v2 - p);
				double dot = wind * n;
				
				force = n.normalize() * dot;
				nodes[i][j]->windForce += force;
				nodes[i+1][j]->windForce += force;
				nodes[i][j+1]->windForce += force;
				
				nodes[i][j]->normal = n;
			}
			if (i != 0 && j != 0) {
				vec3 v1 = nodes[i][j-1]->pos;
				vec3 v2 = nodes[i-1][j]->pos;
				vec3 n = (v2 - p) ^ (v1 - p);
				double dot = wind * n;				
				
				force = n.normalize() * dot;
				nodes[i][j]->windForce += force;
				nodes[i][j-1]->windForce += force;
				nodes[i-1][j]->windForce += force;
				
				nodes[i][j]->normal = n;
			}
		}
	}

}

void Cloth::renderCloth(GLuint depthTextureId) {
    if (shadersOn) {
        glUseProgramObjectARB(program);
        glUniform1iARB(bumpMapEnabledUniform, bumpMapEnabled);
        glUniform1iARB(textureMapEnabledUniform, textureMapEnabled);
    } else {
        glUseProgramObjectARB(0);
    }

    // load textures
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, heightMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
	glActiveTextureARB(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D,depthTextureId);
	//glCullFace(GL_BACK);

	setWindForce();

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			// TOP LEFT AND BOTTOM LEFT CORNER REMAIN STATIONARY in a FLAG
			if (j == 0) {
				if (i == 0) {
					top = nodes[i][j]->pos;
				}
				continue;
			}

			// APPLY GRAVITY
			vec3 gravityForce = gravity;
			nodes[i][j]->applyForce(gravityForce);

			// APPLY WINDFORCE
			vec3 windForce = nodes[i][j]->windForce;
//			cout << "windForce: " << windForce << endl;
			nodes[i][j]->applyForce(windForce);

			// APPLY SPRINGFORCE
			vec3 springForce = nodes[i][j]->getSpringForce();
			nodes[i][j]->applyForce(springForce);

		}
	}
	for (int i = 0; i < height - 1; i++) {
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j < width; j++) {
			vec3 p = nodes[i][j]->pos;
			vec3 pn = nodes[i][j]->normal;
			glNormal3d(pn[0], pn[1], pn[2]);
			glTexCoord2f(((double) j) / width, ((double)i) / height);
			glVertex3f(p[0], p[1], p[2]);

			vec3 q = nodes[i+1][j]->pos;
			vec3 qn = nodes[i+1][j]->normal;
			glNormal3d(qn[0], qn[1], qn[2]);
			glTexCoord2f(((double) j) / width,  ((double) (i + 1)) / height);
			glVertex3f(q[0], q[1], q[2]);
		}
		glEnd();
	}
	glPushMatrix();
		glTranslatef(top[0], top[1], -1.0 * HEIGHT*STEP_SIZE + top[2]);
		GLUquadricObj * quadratic;
		quadratic = gluNewQuadric();
		gluQuadricDrawStyle(quadratic, GLU_FILL);
		gluCylinder(quadratic, .05f, .05f, 3.5f, 32, 32);
	glPopMatrix();
}