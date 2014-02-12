//
//  GeometricUtils.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 2/9/14.
//  Copyright (c) 2014 Hesham Wahba. All rights reserved.
//

#include "GeometricUtils.h"


/// For code below:
//
// Copyright 2001 softSurfer, 2012 Dan Sunday
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.


// Assume that classes are already given for the objects:
//    Point and Vector with
//        coordinates {float x, y, z;}
//        operators for:
//            == to test  equality
//            != to test  inequality
//            (Vector)0 =  (0,0,0)         (null vector)
//            Point   = Point ± Vector
//            Vector =  Point - Point
//            Vector =  Scalar * Vector    (scalar product)
//            Vector =  Vector * Vector    (cross product)
//    Line and Ray and Segment with defining  points {Point P0, P1;}
//        (a Line is infinite, Rays and  Segments start at P0)
//        (a Ray extends beyond P1, but a  Segment ends at P1)
//    Plane with a point and a normal {Point V0; Vector  n;}
//    Triangle with defining vertices {Point V0, V1, V2;}
//    Polyline and Polygon with n vertices {int n;  Point *V;}
//        (a Polygon has V[n]=V[0])
//===================================================================


#define SMALL_NUM   0.00000001 // anything that avoids division overflow
// dot product (3D) which allows vector operations in arguments
///#define dot(u,v)   ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)

// intersect3D_RayTriangle(): find the 3D intersection of a ray with a triangle
//    Input:  a ray R, and a triangle T
//    Output: *I = intersection point (when it exists)
//    Return: -1 = triangle is degenerate (a segment or point)
//             0 =  disjoint (no intersect)
//             1 =  intersect in unique point I1
//             2 =  are in the same plane
int intersect3D_RayTriangle(const Ray &R, const Triangle &T, Point* I )
{
    // triangle vectors
    // get triangle edge vectors and plane normal
    const glm::vec3 u = glm::vec3(T.V1 - T.V0);
    const glm::vec3 v = glm::vec3(T.V2 - T.V0);
    const glm::vec3 n = glm::cross(glm::vec3(u),glm::vec3(v));//u * v;              // cross product
    if (n == glm::vec3(0.0f))             // triangle is degenerate
        return -1;                  // do not deal with this case
    
    // ray vectors
    const glm::vec3 dir =  glm::vec3(R.P1 - R.P0);              // ray direction vector
    const glm::vec3 w0  =  glm::vec3(R.P0 - T.V0);
    
    // params to calc ray-plane intersect
    const float a   = -glm::dot(n,w0);//-dot(n,w0);
    const float b   =  glm::dot(n,dir);//dot(n,dir);
    if (fabs(b) < SMALL_NUM) {     // ray is  parallel to triangle plane
        if (a == 0)                 // ray lies in triangle plane
            return 2;
        else return 0;              // ray disjoint from plane
    }
    
    // get intersect point of ray with triangle plane
    const float r = a / b; // params to calc ray-plane intersect
    if (r < 0.0)                    // ray goes away from triangle
        return 0;                   // => no intersect
    // for a segment, also test if (r > 1.0) => no intersect
    
    *I = R.P0 + r * glm::vec4(dir.x, dir.y, dir.z, 0);            // intersect point of ray and plane
    
    // is I inside T?
    float    uu, uv, vv, wu, wv, D;
    uu = glm::dot(u,u);
    uv = glm::dot(u,v);
    vv = glm::dot(v,v);
    const glm::vec3 w =  glm::vec3(*I - T.V0); // ray vector
    wu = glm::dot(w,u);
    wv = glm::dot(w,v);
    D  = uv * uv - uu * vv;
    
    // get and test parametric coords
    float s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)         // I is outside T
        return 0;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)  // I is outside T
        return 0;
    
    return 1;                       // I is in T
}
// END
