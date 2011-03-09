#include "BSpline.h"

SplinePoint* SplinePoint::sampleBSpline(vector<SplinePoint*>& cps, double t, bool closed, int degree)
{
    if (cps.empty()) {
        UCBPrint("sampleBSpline", "need at least one control point");
        return NULL;
    }

    // get into 0,1 range
    if (t > 1.0 || t < 0.0)
        t = fmod(t, 1.0);
    if (t < 0.0)
        t += 1.0;
    
    // adjust degree down as needed to get a curve, if too few control points and not on closed loop
    int numCPs = int(closed ? cps.size() + degree : cps.size());
    if (degree >= numCPs)
        degree = numCPs - 1;

    // rescale t to minSupport,maxSupport range
    double minSupport = degree;
    double maxSupport = numCPs;
    t = (1-t)*minSupport + t*maxSupport;

    // 'recursive' form of b-spline is done iteratively here
    vector<SplinePoint*> bases; bases.resize(degree+1);
    int k = (int)t;

    for (int i = 0; i <= degree; ++i) {
        bases[i] = cps[(k - degree + i) % cps.size()];
    }

    for (int power = 1; power <= degree; ++power) {
        for (int i = 0; i <= degree - power; ++i) {
            int knot = k - degree + power + i;
            double u_i = (double)knot;
            double u_ipr1 = double(knot + degree - power + 1);
            double a = (t - u_i) / (u_ipr1 - u_i);
            SplinePoint *b_i = bases[i];
            bases[i] = bases[i]->lerp(bases[i+1], a);
            if (power > 1)
                delete b_i;
        }
    }

    SplinePoint *result = bases[0];

    if (degree > 1) {
        for (int i = 1; i < degree-1; i++) {
            delete bases[i];
        }
    }

    return result;
}