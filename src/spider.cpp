#include "spider.h"

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

void renderBitmapString(float x, float y, void *font, char *string) 
{  
  char *c;
  glRasterPos2f(x,y);
  for (c=string; *c != '\0'; c++) 
  {
    glutBitmapCharacter(font, *c);
  }
}


void glThickLine(vec3 start, vec3 end, double thick){
	glBegin(GL_QUAD_STRIP);
		glVertex3f(start[0], start[1], start[2]);
		glVertex3f(end[0], end[1], end[2]);	
		glVertex3f(start[0] - thick, start[1], start[2]);
		glVertex3f(end[0] - thick, end[1], end[2]);
		glVertex3f(start[0] - thick, start[1] - thick, start[2]);
		glVertex3f(end[0] - thick, end[1] - thick, end[2]);
		glVertex3f(start[0], start[1] - thick, start[2]);
		glVertex3f(end[0], end[1] - thick, end[2]);
	glEnd();

}
Spider::Spider(face * f, double t_init, vec3 offset, string filename,  string vertProg, string fragProg){
	angle = (rand() % 90)/2;
	loc = offset;
	myFace = f;
	o = vec3(1, 0, 0);
	t = t_init;
	health = 100; alive = true;

    ifstream f2(filename.c_str());
    if (!f2) {
        UCBPrint("Sweep", "Couldn't load file " << filename);
        return;
    }
    string line;
    while (getline(f2,line)) {
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
    bumpMapEnabledUniform = glGetUniformLocationARB(program, "bumpMapEnabled");
    textureMapEnabledUniform = glGetUniformLocationARB(program, "textureMapEnabled");
    tangentAttrib = glGetAttribLocationARB(program, "tangent");
    bitangentAttrib = glGetAttribLocationARB(program, "bitangent");

}

Spider::~Spider(void){
}

bool Spider::render(bool showHealth, bullet * b, ccMap * cube, int num, GLuint depthTextureId) {
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

	glColor3f(1 - health / 100, 0, 0.5);
	if (!alive) return false;
	bool ret = false;
	double t_local = t;
	//Check for hit
	if (b!= NULL){
		vec3 d = vec3(loc[0] + cos(t_local/6), loc[1], loc[2] + 0.5 - sin(t) / 6)
		- b->current;
		if (d.length2() < 0.5){
			ret = true;
			health-= 120 * d.length2();
			if (health <= 0) alive= false;
		}
	}
	
	//render spider body
		//glTranslatef(loc[0], loc[1], loc[2]);
		//glTranslatef(myFace->centroid()[0] + cos(t_local/6), myFace->centroid()[1], myFace->centroid()[2] + 0.5 - sin(t) / 6);
		/*
		vec3 c = cube->faces[(num * (cube->faces.size() % 10)) % cube->faces.size()]->centroid();
		//glTranslatef(c[0] + cos(t_local/6), c[1], c[2] + .5 - sin(t) / 6);
		//glTexCoord2f(0, 0);
		
		glPushMatrix();
		glScalef(o[0]+1, o[1]+1, o[2]+1);
		glRotatef(7 * sin(t), 0, 0, 1);
		glutSolidSphere(.3, 6, 6);
		glPopMatrix();
		*/
		//render spider body

		glTranslatef(loc[0] + cos(t_local/6), loc[1], loc[2] + 0.5 - sin(t) / 6);
		//glRotatef((angle/2 - 35) * 2, 0, 1, 0);
		//glTexCoord2f(0, 0);
		glRotatef(((angle)/2 - 10), 0, 1, 0);
		glRotatef(sin(t/10)*5, 0, 0, 1);
		glRotatef(sin(t)*5, 0, 1, 0);
		glPushMatrix();
		glScalef(o[0]+1, o[1]+1, o[2]+1);
		glRotatef(7 * sin(t), 0, 0, 1);
		glutSolidSphere(.3, 6, 6);
		glPopMatrix();
	char s[5];
	itoa(health, s, 10);
	strcat(s, "%");
	glPushMatrix();
	glRotatef(90, 0, 0, 1);
	if (showHealth) renderBitmapString(0, 0, GLUT_BITMAP_TIMES_ROMAN_24, s);
	glPopMatrix();
	//render spider eyes
	glPushMatrix();
		glTranslatef(-0.5, 0.15, 0.06);
		glutSolidSphere(0.1, 4, 4);
		glTranslatef(0, - 0.3, 0);
		glutSolidSphere(0.1, 4, 4);
	glPopMatrix();
	//render spider legs
	int i=0;
	while(i < 4){
		glTranslatef( - 0.1 + 0.1 * i, 0, 0);
		glThickLine(vec3(0,0,0), vec3(0, 0.6, sin(t_local) / 4), 0.05);
		glThickLine(vec3(0, 0.6, sin(t_local) / 4), vec3(0, 0.75, sin(t_local) / 4 - 0.5), 0.05);
		glThickLine(vec3(0,0,0), vec3(0, - 0.6, sin(t_local) / 4), 0.05);
		glThickLine(vec3(0, - 0.6, sin(t_local) / 4), vec3(0, -0.75, sin(t_local) / 4 - 0.5), 0.05);
		i++;
		t_local+= .3;
	}
	
	//cout << myFace->().normalize();
	t+= (0.015);
	return ret;
	
}
