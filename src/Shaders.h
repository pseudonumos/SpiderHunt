/*
 * Shaders.h
 *
 *  Created on: March 19, 2009
 *      Author: jima, based on nvidia and lighthouse3D example code.
 */

#ifndef SHADERS_H_
#define SHADERS_H_

#include "global.h"

bool setShader(GLhandleARB p, const char *srcfile, GLenum type);
bool linkAndValidateShader(GLhandleARB p);

#endif 
