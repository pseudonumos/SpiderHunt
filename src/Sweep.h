/*
 * Sweep.h
 *
 *  Created on: March 19, 2009
 *      Author: jima 
 */

#ifndef SWEEP_H_
#define SWEEP_H_


#include <cmath>
#include <vector>

#include "algebra3.h"
#include "global.h"

#include "LoadImage.h"
#include "BSpline.h"

//#include "ccMap.h"

using namespace std;


class Sweep {
public:
    Sweep(string filename, string vertShader, string fragmentShader);
    ~Sweep() {
        glDeleteObjectARB(program);
    }

    // renders the coaster
    void Render(int samplesPerPt, int samplesPerPtOnCrossSection, double crossSectionScale=.2);

    // if there are no control points, that's bad
    bool bad() {
        if (sweepControlPts.empty())
            return true;
        if (crossSectionControlPts.empty())
            return true;
        return false;
    }

    void toggleShader() { shadersOn = !shadersOn; }
    void toggleBumpMap() { bumpMapEnabled = !bumpMapEnabled; }
    void toggleTextureMap() { textureMapEnabled = !textureMapEnabled; }

private:
	vector<vec3> cubeVerts;
    vector<SplinePoint*> sweepControlPts; // control points for the sweep
    vector<SplinePoint*> crossSectionControlPts; // control points for the cross section
    double globalAzimuth; // global azimuth rotates the whole frame
    double globalTwist; // twists the whole frame

    GLuint texture, normalMap, heightMap; // texture data
    int lengthRepeats, widthRepeats;
    
    bool shadersFailed;
    GLhandleARB program; // shaders
    GLint tangentAttrib, bitangentAttrib; // tangent space for uploading to shaders
    GLint bumpMapEnabledUniform, textureMapEnabledUniform;
    bool bumpMapEnabled, textureMapEnabled;
    bool shadersOn;

    // internal helper functions
    void sendVertex(vec3 &v);
    void createPolyline(vector<SplinePoint*> &ctrlPts, vector<SplinePoint*> &polyline, int totalSamples, int extra);
    void renderSweep(vector<SplinePoint*> &polyline, vector<SplinePoint*> &crossSection, double crossSectionScale);
    void freePolyline(vector<SplinePoint*> &pts);
    // helpful function to rotate a vector about dir by the appropriate amount given local and global azimuths and twist
    void orientVectorInFrame(const vec3 &dir, double percent, double localAz, vec3 &inFrame);
    // rotation minimizing frame computed by double reflection
    vec3 advanceFrame(const vec3 &xi, const vec3 &xi1,
                        const vec3 &ti, const vec3 &si, const vec3 &ri,
                          const vec3 &ti1);
};


#endif

