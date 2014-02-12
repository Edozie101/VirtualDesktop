//
//  GeometricUtils.h
//  IbexMac
//
//  Created by Hesham Wahba on 2/9/14.
//  Copyright (c) 2014 Hesham Wahba. All rights reserved.
//

#ifndef __IbexMac__GeometricUtils__
#define __IbexMac__GeometricUtils__

#include <glm/glm.hpp>
#include <glm/ext.hpp>

typedef glm::vec4 Point;
typedef glm::vec4 Vector;

typedef struct {
    Point P0;
    Point P1;
} Ray;


typedef struct {
    Point V0;
    Point V1;
    Point V2;
} Triangle;


/// For code below:
//
// Copyright 2001 softSurfer, 2012 Dan Sunday
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

// intersect3D_RayTriangle(): find the 3D intersection of a ray with a triangle
//    Input:  a ray R, and a triangle T
//    Output: *I = intersection point (when it exists)
//    Return: -1 = triangle is degenerate (a segment or point)
//             0 =  disjoint (no intersect)
//             1 =  intersect in unique point I1
//             2 =  are in the same plane
int intersect3D_RayTriangle(const Ray &R, const Triangle &T, Point* I);
// END

#endif /* defined(__IbexMac__GeometricUtils__) */
