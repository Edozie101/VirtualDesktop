/*
 * opengl_helpers.h
 *
 *  Created on: Sep 12, 2012
 *      Author: Hesham Wahba
 */

#ifndef OPENGL_HELPERS_H_
#define OPENGL_HELPERS_H_

bool gluInvertMatrix(const double m[16], double invOut[16]);
double distort2ShaderScaleFactor(double ax, double ay);

#endif /* OPENGL_HELPERS_H_ */
