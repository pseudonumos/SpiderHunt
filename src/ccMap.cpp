#include "ccMap.h"

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

void ccMap::makeTube(int v1, int v2, int v3, int v4){ //Arguments must be closest (in-z) flat plane
	makeFace(v1,v2,v3,v4);
	makeFace(v1+16,v2+16,v3+16,v4+16);
	makeFace(v1,v2,v2+16,v1+16);
	makeFace(v3,v4,v4+16,v3+16);

}
void ccMap::connect(int v1, int v2, int v3, int v4){
	makeFace(v1+16, v1+32, v2+32, v2+16);
	makeFace(v2+16, v2+32, v3+32, v3+16);
	makeFace(v3+16, v3+32, v4+32, v4+16);
	makeFace(v4+16, v4+32, v1+32, v1+16);
}

void ccMap::makeFace(int v1, int v2, int v3, int v4){
	edge * newEdges[4];
	newEdges[0] = new edge(vertices[v1], vertices[v2]);
	newEdges[1] = new edge(vertices[v2], vertices[v3]);
	newEdges[2] = new edge(vertices[v3], vertices[v4]);
	newEdges[3] = new edge(vertices[v4], vertices[v1]);
	for (int i = 0; i < 4; i++) newEdges[i] = push(newEdges[i]);
	face * f = new face(newEdges[0], newEdges[1], newEdges[2], newEdges[3]);
	faces.push_back(f);
	vertices[v1]->faces.push_back(f);
	vertices[v2]->faces.push_back(f);
	vertices[v3]->faces.push_back(f);
	vertices[v4]->faces.push_back(f);
}
edge * ccMap::push(edge * e){
	for (int m = 0; m < edges.size(); m++) {
		
		if (*e == *edges[m]) {
//			cout << "duplicate edge " << endl;
			return edges[m];
		}
	}
	edges.push_back(e);
	return e;
}

ccMap::ccMap(int shape,  string filename,  string vertProg, string fragProg)
{
	switch(shape){
		case 1:
		//Z level = 4
			vertices.push_back(new vertex(vec3(-4, -4, 4))); //0: -4 bottom row
			vertices[0]->texCoord = vec2(0, 1);
			vertices.push_back(new vertex(vec3(-2, -4, 4))); 
			vertices.push_back(new vertex(vec3(2, -4, 4)));
			vertices.push_back(new vertex(vec3(4, -4, 4)));
			vertices.push_back(new vertex(vec3(-4, -2, 4))); //4: -2 row
			vertices.push_back(new vertex(vec3(-2, -2, 4))); 
			vertices.push_back(new vertex(vec3(2, -2, 4)));
			vertices.push_back(new vertex(vec3(4, -2, 4)));
			vertices.push_back(new vertex(vec3(-4, 2, 4))); //8: 2 row
			vertices.push_back(new vertex(vec3(-2, 2, 4))); 
			vertices.push_back(new vertex(vec3(2, 2, 4)));
			vertices.push_back(new vertex(vec3(4, 2, 4)));
			vertices.push_back(new vertex(vec3(-4, 4, 4))); //12: 4 row
			vertices.push_back(new vertex(vec3(-2, 4, 4))); 
			vertices.push_back(new vertex(vec3(2, 4, 4)));
			vertices.push_back(new vertex(vec3(4, 4, 4)));
			vertices[15]->texCoord = vec2(1, 1);
	//Z level = 2
			vertices.push_back(new vertex(vec3(-4, -4, 2))); //16: -4 bottom row
			vertices.push_back(new vertex(vec3(-2, -4, 2))); 
			vertices.push_back(new vertex(vec3(2, -4, 2)));
			vertices.push_back(new vertex(vec3(4, -4, 2)));
			vertices.push_back(new vertex(vec3(-4, -2, 2))); //20: -2 row
			vertices.push_back(new vertex(vec3(-2, -2, 2))); 
			vertices.push_back(new vertex(vec3(2, -2, 2)));
			vertices.push_back(new vertex(vec3(4, -2, 2)));
			vertices.push_back(new vertex(vec3(-4, 2, 2))); //24: 2 row
			vertices.push_back(new vertex(vec3(-2, 2, 2))); 
			vertices.push_back(new vertex(vec3(2, 2, 2)));
			vertices.push_back(new vertex(vec3(4, 2, 2)));
			vertices.push_back(new vertex(vec3(-4, 4, 2))); //28: 4 row
			vertices.push_back(new vertex(vec3(-2, 4, 2))); 
			vertices.push_back(new vertex(vec3(2, 4, 2)));
			vertices.push_back(new vertex(vec3(4, 4, 2)));
		//Z level = -2
			vertices.push_back(new vertex(vec3(-4, -4, -2))); //32: -4 bottom row
			vertices.push_back(new vertex(vec3(-2, -4, -2))); 
			vertices.push_back(new vertex(vec3(2, -4, -2)));
			vertices.push_back(new vertex(vec3(4, -4, -2)));
			vertices.push_back(new vertex(vec3(-4, -2, -2))); //36: -2 row
			vertices.push_back(new vertex(vec3(-2, -2, -2))); 
			vertices.push_back(new vertex(vec3(2, -2, -2)));
			vertices.push_back(new vertex(vec3(4, -2, -2)));
			vertices.push_back(new vertex(vec3(-4, 2, -2))); //40: 2 row
			vertices.push_back(new vertex(vec3(-2, 2, -2))); 
			vertices.push_back(new vertex(vec3(2, 2, -2)));
			vertices.push_back(new vertex(vec3(4, 2, -2)));
			vertices.push_back(new vertex(vec3(-4, 4, -2))); //44: 4 row
			vertices.push_back(new vertex(vec3(-2, 4, -2))); 
			vertices.push_back(new vertex(vec3(2, 4, -2)));
			vertices.push_back(new vertex(vec3(4, 4, -2)));
		//Z level = -4
			vertices.push_back(new vertex(vec3(-4, -4, -4))); //32: -4 bottom row
			vertices.push_back(new vertex(vec3(-2, -4, -4))); 
			vertices.push_back(new vertex(vec3(2, -4, -4)));
			vertices.push_back(new vertex(vec3(4, -4, -4)));
			vertices.push_back(new vertex(vec3(-4, -2, -4))); //36: -2 row
			vertices.push_back(new vertex(vec3(-2, -2, -4))); 
			vertices.push_back(new vertex(vec3(2, -2, -4)));
			vertices.push_back(new vertex(vec3(4, -2, -4)));
			vertices.push_back(new vertex(vec3(-4, 2, -4))); //40: 2 row
			vertices.push_back(new vertex(vec3(-2, 2, -4))); 
			vertices.push_back(new vertex(vec3(2, 2, -4)));
			vertices.push_back(new vertex(vec3(4, 2, -4)));
			vertices.push_back(new vertex(vec3(-4, 4, -4))); //44: 4 row
			vertices.push_back(new vertex(vec3(-2, 4, -4))); 
			vertices.push_back(new vertex(vec3(2, 4, -4)));
			vertices.push_back(new vertex(vec3(4, 4, -4)));
			
			makeFace(0,1,5,4); //bottom-left
			makeFace(0,1,17,16);
			makeFace(0, 4, 20, 16);

			makeTube(1,2,6,5); //bottom

			makeFace(2,3,7,6); // bottom-right
			//makeFace(18,19,23,22);
			makeFace(2,3,19,18);
			makeFace(3,19,23,7);

			makeTube(10,6,7,11);  //right
			
			makeFace(10,11,15,14); //top-right
			makeFace(11,15,15+16,11+16);
			makeFace(14,15,15+16,14+16);
			
			makeTube(9,10,14,13); // top

			makeFace(8,9,13,12); //top-left
			makeFace(12,8,8+16,12+16);
			makeFace(12,13,13+16,12+16);

			makeTube(8,4,5,9); //left

			

			//BACK FACE:
			makeFace(48+0,48+1,48+5,48+4); //bottom-left
			makeFace(32+0,32+1,32+17,32+16);
			makeFace(32+0, 32+4, 32+20, 32+16);
			makeTube(32+1,32+2,32+6,32+5); //bottom
			makeFace(48+2,48+3,48+7,48+6); // bottom-right
			makeFace(32+2,32+3,32+19,32+18);
			makeFace(32+3,32+19,32+23,32+7);
			//makeTube(32+10,32+6,32+7,32+11);  //right

			makeFace(48+10,48+11,48+15,48+14); //top-right
			makeFace(32+11,32+15,32+15+16,32+11+16);
			makeFace(32+14,32+15,32+15+16,32+14+16);

			makeTube(32+9,32+10,32+14,32+13); // top
			makeFace(48+8,48+9,48+13,48+12); //top-left
			makeFace(32+12,32+8,32+8+16,32+12+16);
			makeFace(32+12,32+13,32+13+16,32+12+16);

			makeTube(32+8,32+4,32+5,32+9);
			makeTube(32+10,32+6,32+7,32+11);



			//Connections:
			connect(12, 13, 9, 8);
			connect(14, 15, 11, 10);
			connect(4, 5, 1, 0);
			connect(6, 7, 3, 2);
			break;
			for (int i=0; i< vertices.size(); i++) 
				if (vertices[i]->faces.size() !=3) 
					cout << "ERROR: " << vertices[i]->faces.size() << endl;

		case 0:
			vertices.push_back(new vertex(vec3(-3, -3, -3))); // 0
			vertices.push_back(new vertex(vec3(-3, -3, 3))); // 1
			vertices.push_back(new vertex(vec3(-3, 3, -3))); // 2
			vertices.push_back(new vertex(vec3(-3, 3, 3))); // 3
			vertices.push_back(new vertex(vec3(3, -3, -3))); // 4
			vertices.push_back(new vertex(vec3(3, -3, 3))); // 5
			vertices.push_back(new vertex(vec3(3, 3, -3))); // 6
			vertices.push_back(new vertex(vec3(3, 3, 3))); // 7

			edges.push_back(new edge(vertices[0], vertices[1])); // (0, 0, 0) => (0, 0, 1) 0
			edges.push_back(new edge(vertices[0], vertices[2])); // (0, 0, 0) => (0, 1, 0) 1
			edges.push_back(new edge(vertices[0], vertices[4])); // (0, 0, 0) => (1, 0, 0) 2
			edges.push_back(new edge(vertices[1], vertices[3])); // (0, 0, 1) => (0, 1, 1) 3
			edges.push_back(new edge(vertices[1], vertices[5])); // (0, 0, 1) => (1, 0, 1) 4
			edges.push_back(new edge(vertices[2], vertices[3])); // (0, 1, 0) => (0, 1, 1) 5
			edges.push_back(new edge(vertices[2], vertices[6])); // (0, 1, 0) => (1, 1, 0) 6
			edges.push_back(new edge(vertices[3], vertices[7])); // (0, 1, 1) => (1, 1, 1) 7
			edges.push_back(new edge(vertices[4], vertices[5])); // (1, 0, 0) => (1, 0, 1) 8
			edges.push_back(new edge(vertices[4], vertices[6])); // (1, 0, 0) => (1, 1, 0) 9
			edges.push_back(new edge(vertices[5], vertices[7])); // (1, 0, 1) => (1, 1, 1) 10
			edges.push_back(new edge(vertices[6], vertices[7])); // (1, 1, 0) => (1, 1, 1) 11

			faces.push_back(new face(edges[2], edges[1], edges[9], edges[6]));
			faces.push_back(new face(edges[11], edges[8], edges[9], edges[10]));
			faces.push_back(new face(edges[6], edges[7], edges[5], edges[11]));
			faces.push_back(new face(edges[2], edges[0], edges[4], edges[8]));
			faces.push_back(new face(edges[1], edges[0], edges[5], edges[3]));
			faces.push_back(new face(edges[4], edges[10], edges[7], edges[3]));
			
			vertices[0]->faces.push_back(faces[3]);
			vertices[0]->faces.push_back(faces[4]);
			vertices[0]->faces.push_back(faces[0]);
			vertices[1]->faces.push_back(faces[5]);
			vertices[1]->faces.push_back(faces[4]);
			vertices[1]->faces.push_back(faces[3]);
			vertices[2]->faces.push_back(faces[0]);
			vertices[2]->faces.push_back(faces[2]);
			vertices[2]->faces.push_back(faces[4]);
			vertices[3]->faces.push_back(faces[2]);
			vertices[3]->faces.push_back(faces[4]);
			vertices[3]->faces.push_back(faces[5]);
			vertices[4]->faces.push_back(faces[0]);
			vertices[4]->faces.push_back(faces[1]);
			vertices[4]->faces.push_back(faces[3]);
			vertices[5]->faces.push_back(faces[1]);
			vertices[5]->faces.push_back(faces[3]);
			vertices[5]->faces.push_back(faces[5]);
			vertices[6]->faces.push_back(faces[0]);
			vertices[6]->faces.push_back(faces[1]);
			vertices[6]->faces.push_back(faces[2]);
			vertices[7]->faces.push_back(faces[1]);
			vertices[7]->faces.push_back(faces[2]);
			vertices[7]->faces.push_back(faces[5]);
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
            getValue(linestream, lengthRepeats);
            getValue(linestream, widthRepeats);
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
    bumpMapEnabled = true;
    textureMapEnabled = true;

    // set up variables for the shaders to use

    glUseProgramObjectARB(program);
    glUniform1iARB(glGetUniformLocationARB(program, "textureMap"), 0);
    glUniform1iARB(glGetUniformLocationARB(program, "heightMap"), 1);
    glUniform1iARB(glGetUniformLocationARB(program, "normalMap"), 2);
    bumpMapEnabledUniform = glGetUniformLocationARB(program, "bumpMapEnabled");
    textureMapEnabledUniform = glGetUniformLocationARB(program, "textureMapEnabled");
	shadowMapUniform = glGetUniformLocationARB(program,"shadowMap");

    tangentAttrib = glGetAttribLocationARB(program, "tangent");
    bitangentAttrib = glGetAttribLocationARB(program, "bitangent");

	glClearColor(0,0,0,1.0f);
	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	subdivLevel = 0;
}

ccMap::~ccMap(void)
{
}
void ccMap::renderCube(GLuint depthTextureId) {

	if (shadersOn) {
        glUseProgramObjectARB(program);
        glUniform1iARB(bumpMapEnabledUniform, bumpMapEnabled);
        glUniform1iARB(textureMapEnabledUniform, textureMapEnabled);
		glUniform1iARB(shadowMapUniform,7);
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
//	glCullFace(GL_BACK);

	for (int i = 0; i < faces.size(); i++) {
		faces[i]->subdivideLevel = subdivLevel;
		faces[i]->render();
	}
}

void ccMap::subdivide() {

	subdivLevel++;

	vector<vertex*> new_vertices;
	vector<face*> new_faces;
	vector<edge*> new_edges;

	for (int i = 0; i < faces.size(); i++) {
		
		// Encapsulate centroid to vertex
		vertex * centroid = new vertex(faces[i]->centroid());
		vec2 temp2(0, 0);
		for (int j = 0; j < faces[i]->edges.size(); j++) {
			temp2 += faces[i]->edges[j]->a->texCoord;
		}
		centroid->texCoord = temp2/4;

		// Add centroid to new_vertices
		new_vertices.push_back(centroid);

		// calculate edge points/edges for each face
		// #DEFINE edge point = average of neighboring face points (centroids) and the two original points of edges
		for (int j = 0; j < faces[i]->edges.size(); j++) {
			
			// Calculating edge point (from wikipedia)			
			vec3 temp = (faces[i]->centroid() + 
						faces[i]->edges[j]->otherface(faces[i])->centroid() + 
						faces[i]->edges[j]->a->v + 
						faces[i]->edges[j]->b->v) / 4;

			// Encapsulating edge point to new_vertices & adding new vertex to new_vertices
			vertex * new_vertex = new vertex(temp);
			new_vertex->texCoord = (faces[i]->edges[j]->a->texCoord + faces[i]->edges[j]->b->texCoord)/2;
			bool isIn = false;
			for (int k = 0; k < new_vertices.size(); k++) {
				if (new_vertices[k]->v == temp) {
					new_vertex = new_vertices[k];
					isIn = true;
				}
			}
			if (!isIn) {
				new_vertices.push_back(new_vertex);
			}

			// Creating new edges from new vertices
			edge * new_edge = new edge(centroid, new_vertex);
			new_edge->oldface = faces[i];

			// Linking the new edge to the vertices and faces
			faces[i]->edges[j]->a->new_edges.push_back(new_edge);
			faces[i]->edges[j]->b->new_edges.push_back(new_edge);
			faces[i]->new_edges.push_back(new_edge);
		}
	}
	
	// modify all vertices into new vertices
	for (int i = 0; i < vertices.size(); i++) {
		// new Vertex = (F + 2R + (n-3)P) / n
		vec3 facepoints_avg = vec3(0, 0, 0);
		vec3 edgepoints_avg = vec3(0, 0, 0);
		int n = vertices[i]->edges.size();

		// cout << "vertex: " << vertices[i]->v << " | edges_size: " << n << endl;

		// calculate F
		for (int j = 0; j < vertices[i]->faces.size(); j++) {
			facepoints_avg += vertices[i]->faces[j]->centroid();
		}
		facepoints_avg /= vertices[i]->faces.size();


		vec2 temp2(0, 0);
		// calculate R
		for (int j = 0; j < n; j++) {
			edgepoints_avg += vertices[i]->edges[j]->midpoint();
			
		}
		edgepoints_avg /= n;

		// calculate (F+2R+(n-3)P) / n => new vertex, added to vertices
		vec3 temp = (facepoints_avg + 2*edgepoints_avg + (n - 3) * vertices[i]->v) / n;
		vertex * v = new vertex(temp);
		new_vertices.push_back(v);

		// subdivide all faces
		face * currentFace;
		int j;

		int count = 0;
		for (j = 0; j < vertices[i]->new_edges.size(); j++) {
			currentFace = vertices[i]->new_edges[j]->oldface;
			for (int k = j+1; k < vertices[i]->new_edges.size(); k++) {
				if (vertices[i]->new_edges[k]->oldface->centroid() == currentFace->centroid()) {
					// You've found a pair of new_edges that share the same centroid/face
					//Construct two edges per face per vertex from these new_edges to v
					edge * myEdges[4];
					vertex * a = vertices[i]->new_edges[j]->othervertex(currentFace->centroid());
					vertex * b = vertices[i]->new_edges[k]->othervertex(currentFace->centroid());

					myEdges[0] = new edge(v, a);
					myEdges[1] = new edge(b, v);
					myEdges[2] = vertices[i]->new_edges[j];
					myEdges[3] = vertices[i]->new_edges[k];
					
					for (int l = 0; l < 4; l++) {
						bool isIn = false;
						for (int m = 0; m < new_edges.size(); m++) {
							if (*myEdges[l] == *new_edges[m]) {
								myEdges[l] = new_edges[m];
								isIn = true;
							}
						}
						if (!isIn) {
							new_edges.push_back(myEdges[l]);
//							v->edges.push_back(myEdges[l]);
						}
					}
					face * f = new face(myEdges[0], myEdges[1], myEdges[2], myEdges[3]);

					new_faces.push_back(f);
					v->faces.push_back(f);
					a->faces.push_back(f);
					b->faces.push_back(f);
					if (myEdges[2]->a->v == currentFace->centroid()) myEdges[2]->a->faces.push_back(f);
					else myEdges[2]->b->faces.push_back(f);
					count++;
				}
			}
		}
	}
	
	vertices.clear();
	vertices = new_vertices;
	faces.clear();
	faces = new_faces;	

	cout << "\n" <<endl;
	for (int i = 0; i < vertices.size(); i++) {
		cout << "vertex " << i << ": " << vertices[i]->v << endl;
	}

}
face * ccMap::closestFace(vec3 pos) {
	float min = 100000.0;
	face * closestFace = faces[0];
	for (int i = 0; i < faces.size(); i++) {
		/*
		// get Normal
		vec3 n = faces[i]->normal();
		n.normalize();
		// find n * d = -p
		double p = -1 * (n * faces[i]->centroid());
		double dist = abs(n * pos + p);
		if (dist < min) {
			double top = (faces[i]->normal() * faces[i]->centroid()) - ((pos - faces[i]->normal()) * faces[i]->normal());
			double bottom = (pos - (pos - faces[i]->normal())) * faces[i]->normal();
			double t = top / bottom;
			vec3 newpos = (pos - faces[i]->normal()) + t * faces[i]->normal();
			minPos = faces[i]->centroid();
			min = dist;
		}*/
		vec3 c = faces[i]->centroid();
		double dist = sqrt(pow(c[0] - pos[0], 2) + pow(c[1] - pos[1], 2) + pow(c[2] - pos[2], 2));
		if (dist < min) {
			closestFace = faces[i];
			min = dist;
		}
	}
	//cout << "minDistance: " << min << endl;
	return closestFace;
}

