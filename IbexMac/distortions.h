/*
 * distortions.h
 *
 *  Created on: Sep 12, 2012
 *      Author: Hesham Wahba
 */

#ifndef DISTORTIONS_H_
#define DISTORTIONS_H_

#include "opengl_helpers.h"

int init_distortion_shader();
void render_both_frames(const GLuint textureId);
void render_distorted_frame(const bool left, const GLuint textureId);

int init_distortion_shader_cache();
void render_distortion_lenses();

#endif /* DISTORTIONS_H_ */
