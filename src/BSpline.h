/*
 * BSpline.h
 *
 *  Created on: March 19, 2009
 *      Author: jima (referencing jfhamlin)
 */

#ifndef BSPLINE_H_
#define BSPLINE_H_

#include "global.h"

struct SplinePoint {
	virtual ~SplinePoint() {};
    virtual SplinePoint* lerp(SplinePoint *b, double t) = 0;
    virtual double dist2(SplinePoint *b) = 0;

    static SplinePoint* sampleBSpline(vector<SplinePoint*>& cps, double t, bool closed = true, int degree = 3);
};

struct SweepPoint : public SplinePoint {
    vec3 point;
    double azimuth;

    SweepPoint() {}
    SweepPoint(vec3 pt, double az = 0) : point(pt), azimuth(az) {}

    SplinePoint* lerp(SplinePoint *b, double t) {
        SweepPoint* other = (SweepPoint*)b;
        return new SweepPoint((1-t)*point+t*other->point, (1-t)*azimuth+t*other->azimuth);
    }

    double dist2(SplinePoint *b) {
        SweepPoint* other = (SweepPoint*)b;
        return (other->point - point).length2();
    }
};

struct CrossSectionPoint : public SplinePoint {
    vec2 point;

    CrossSectionPoint() {}
    CrossSectionPoint(vec2 pt) : point(pt) {}

    SplinePoint* lerp(SplinePoint *b, double t) {
        CrossSectionPoint* other = (CrossSectionPoint*)b;
        return new CrossSectionPoint((1-t)*point + t*other->point);
    }

    double dist2(SplinePoint *b) {
        CrossSectionPoint *other = (CrossSectionPoint*)b;
        return (other->point - point).length2();
    }
};

#endif
