#include "Sweep.h"

#include "global.h"

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

Sweep::Sweep(string filename, string vertProg, string fragProg) : globalAzimuth(0), globalTwist(0), lengthRepeats(1), widthRepeats(1) {
	//cubeVerts.push_back(vec3(
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
            vec2 v(0);
            linestream >> v;
            crossSectionControlPts.push_back(new CrossSectionPoint(v));
        } else if (op == "v") { // v marks bspline control points with optional azimuth info
            vec3 v(0);
            linestream >> v;
            double az;
            if (linestream >> az) {
                sweepControlPts.push_back(new SweepPoint(v, az));
            } else {
                sweepControlPts.push_back(new SweepPoint(v));
            }
        } else if (op == "twist") {
            linestream >> globalTwist;
        } else if (op == "azimuth") {
            linestream >> globalAzimuth;
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

// Used to advance the rotation minimizing frame forward
vec3 Sweep::advanceFrame(const vec3 &xi, const vec3 &xi1,
                            const vec3 &ti, const vec3 &si, const vec3 &ri,
                              const vec3 &ti1) {
    vec3 v1 = xi1 - xi;
    double c1 = v1 * v1;
    vec3 riL = ri - (2/c1)*(v1*ri)*v1;
    vec3 tiL = ti - (2/c1)*(v1*ti)*v1;
    vec3 v2 = ti1 - tiL;
    double c2 = v2 * v2;
    return riL - (2/c2)*(v2*riL)*v2;
}

// send a vertex to opengl with normal v-orig
void Sweep::sendVertex(vec3 &v) {
    glVertex3d(v[0],v[1],v[2]);
}

// clean up memory (helper for the big render function)
void Sweep::freePolyline(vector<SplinePoint*> &pts) {
    for(vector<SplinePoint*>::iterator it = pts.begin(); it != pts.end(); ++it)
        delete *it;
}

// create a polyline that samples the curve (helper for the big render function)
void Sweep::createPolyline(vector<SplinePoint*> &ctrlPts, vector<SplinePoint*> &polyline, int totalSamples, int extra) {
    if (totalSamples == 0)
        return; // ... no samples is easy!

    SplinePoint *lastGood = NULL;
    for (int i = 0; i < totalSamples + extra; i++) {
        int loc = i % totalSamples;
        double t = loc / double(totalSamples);
        SplinePoint *sp = SplinePoint::sampleBSpline(ctrlPts, t);
        if (!polyline.empty() && sp->dist2(lastGood) < .0001) {
            delete sp;
            continue; // wait for the samples to get a bit further apart ... !
        } else {
            polyline.push_back(sp);
            lastGood = sp;
        }
    }
}

// rotates a vector according to the global azimuth, local azimuth, twist, direction, and location on curve
void Sweep::orientVectorInFrame(const vec3 &dir, double percent, double localAz, vec3 &inFrame) {
    double rot = globalAzimuth + globalTwist * percent + localAz;
    inFrame = rotation3D(dir, rot) * inFrame;
}

namespace { // an anonymous helper for finding a tangent direction from three points
    vec3 findTan(vec3 &a, vec3 &b, vec3 &c) {
        vec3 dir1 = (b-a).normalize();
        vec3 dir2 = (c-b).normalize();
        vec3 dir = dir1+dir2;
        if (dir.length2() < .0001) {
            dir = dir2;
        }
        return dir.normalize();
    }
}

// sweep the cross section along the curve (helper for the big render function)
void Sweep::renderSweep(vector<SplinePoint*> &polyline, vector<SplinePoint*> &crossSection, double crossSectionScale) {
    // enable or disable shaders
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
    

    // set up variables we will use while marching around the sweep
    SweepPoint* pts[3]; // pts[1] is us, pts[0] and pts[3] surround us
    int size = (int) polyline.size();
    vec3 * newSlice = new vec3[crossSection.size()];
    vec3 * oldSlice = new vec3[crossSection.size()];
    vec3 oldDir, dir(0,0,1);
    bool firstDir = true;
    vec3 up, right;

    // March over the sweep, filling in quad strips
    for (int i = 1; i < size-1; i++) {
        // compute the percent we've travel around the sweep
        double percent = double(i % size) / (double(size-3));
        double deltaPercent = 1.0 / (double(size-3));
        for (int c = -1; c <= 1; c++) { // populate local pts
            pts[c+1] = (SweepPoint*)polyline[ (i + size + c) %  size ];
        }

        // look at the two line segments connected to the current point
		vec3 leg1 = (pts[0]->point - pts[1]->point).normalize();
        vec3 leg2 = (pts[2]->point - pts[1]->point).normalize();
        oldDir = dir; // store the previous tangent before computing the new one
        dir = findTan(pts[0]->point, pts[1]->point, pts[2]->point);

        // find the frame orientation
        if (firstDir) {
            up = leg1+leg2; // start with the frenet frame
            if (up.length2() < .0001) { // if that doesn't work, try something else
                up = dir ^ vec3(0,0,1);
                if (up.length2() < .0001) {
                    up = dir ^ vec3(1,0,0);
                }
            }
            firstDir = false;
        }
        else { // after the first frame, advance with the rotation minimizing frame
            up = advanceFrame(pts[1]->point, pts[2]->point,
                        oldDir, right, up, dir);
        }
        right = dir ^ up;

        // normalize our frame
        dir.normalize(); up.normalize(); right.normalize();

        // compute local rotation
        double rot = globalAzimuth + globalTwist * percent + pts[1]->azimuth;

        // compute scaling due to turning (to miter joints)
        vec3 bisect = leg1 + leg2;
        double len = bisect.length();
        bool scaleSect = false;
        double scaleTrans = 0;
        if (len > .0001) { // only scale if not going straight already
            scaleSect = true;
            bisect = bisect/len;
            double dot = -leg1*leg2;
            double angle = acos(CLAMP(dot,-1.0,1.0));
            double scale = 1.0 / MAX(cos(.5*angle ),.1);
            scaleTrans = scale - 1.0;
		}

        // apply scale, rotations, to cross section
        double s = crossSectionScale;
        int ind = 0;
        for (vector<SplinePoint*>::iterator it = crossSection.begin(); it != crossSection.end(); ++it, ++ind) {
            vec2 pos2d = rotation2D(vec2(0,0),rot) * ((CrossSectionPoint*) (*it))->point;
            vec3 pt = right * pos2d[0] * s + up * pos2d[1] * s;
            if (scaleSect) {
                pt = pt + scaleTrans * (pt * bisect) * bisect;
            }
            newSlice[ind] = pts[1]->point + pt;
        }

        // once we've buffered an 'oldSlice', fill in one loop of quads with a quad_strip
        if (i > 1) {
            glBegin(GL_QUAD_STRIP);
            int csize = (int)crossSection.size();
            for (int v = 0; v <= csize; v++) {
                int vn = v % csize;
                vec3 tan0 = findTan(oldSlice[(vn+csize-1)%csize], oldSlice[vn], oldSlice[(vn+1)%csize]);
                vec3 tan1 = findTan(newSlice[(vn+csize-1)%csize], newSlice[vn], newSlice[(vn+1)%csize]);

                double percentAround = v / double(crossSection.size()); // the percent around the cross section
                glColor3f(1,1,1);

                // @TODO: SET TEXTURE COORDINATE
				/*
				cout << "percent: " << percent << endl << "percentAround: " << percentAround << endl 
					<< "deltaPercent: " << deltaPercent << endl;
				
				cout << "lengthRepeats: " << lengthRepeats << "   widthRepeats: " << widthRepeats << endl;
				*/
				glTexCoord2f(percent * lengthRepeats, percentAround * widthRepeats);
				// HINT: use percent, deltaPercent, percentAround to determine where on the curves you are
				// HINT: use lengthRepeats and widthRepeats to determine how much to repeat
				vec3 n = (tan0^oldDir).normalize();
				glNormal3d(n[0],n[1],n[2]);
				if (shadersOn) {
					// @TODO: SET TANGENT AND BITANGENT
					// HINT: Use the glVertexAttrib3fARB, like:
					glVertexAttrib3fARB(tangentAttrib, oldDir[0], oldDir[1], oldDir[2]);
					glVertexAttrib3fARB(bitangentAttrib, tan0[0], tan0[1], tan0[2]);
				}
                sendVertex(oldSlice[vn]);


                // @TODO: SET TEXTURE COORDINATE
                // HINT: see above
				vec3 n2 = (tan1^dir).normalize();
				glNormal3d(n2[0],n2[1],n2[2]);
				glTexCoord2f((percent + deltaPercent) * lengthRepeats, (percentAround) * widthRepeats);
				if (shadersOn) {
					// @TODO: SET TANGENT AND BITANGENT
					// HINT: see above
					glVertexAttrib3fARB(tangentAttrib, dir[0], dir[1], dir[2]);
					glVertexAttrib3fARB(bitangentAttrib, tan1[0], tan1[1], tan1[2]);
				}
                sendVertex(newSlice[vn]);
            }
            glEnd();
        }

        // swap new and old lists
        vec3 *temp = newSlice;
        newSlice = oldSlice;
        oldSlice = temp;
    }
    delete [] newSlice;
    delete [] oldSlice;
}

// the big render function
void Sweep::Render(int samplesPerPt, int samplesPerPtOnCrossSection, double crossSectionScale) {
    int totalSamples = (int) sweepControlPts.size() * samplesPerPt;
    int totalSamplesXSect = (int) crossSectionControlPts.size() * samplesPerPtOnCrossSection;

    vector<SplinePoint*> polyline;
    createPolyline(sweepControlPts, polyline, totalSamples, 3);

    vector<SplinePoint*> crossSect;
    createPolyline(crossSectionControlPts, crossSect, totalSamplesXSect, 0);

    int size = (int) polyline.size();
    if (size < 2) { // a polyline with only one point is pretty lame!
        freePolyline(polyline);
        cout << "not enough curve to sweep ..." << endl;
        return;
    }

    renderSweep(polyline, crossSect, crossSectionScale);

    freePolyline(polyline);
    freePolyline(crossSect);
}

