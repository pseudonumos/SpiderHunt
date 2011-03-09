/*
 * global.h
 *
 *  Created on: Feb 2, 2009
 *      Author: njoubert
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#define IMPLEMENT_ME(file,line)    cout << "METHOD NEEDS TO BE IMPLEMENTED AT " << file << ":" << line << endl; exit(1);

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <vector>
#include <cmath>

#ifdef OSX
#include <GL/glew.h>
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
//... I DON'T HAVE ANY IDEA IF THIS IS RIGHT
#else
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/glu.h>
#endif

//Include our math library
#include <algebra3.h>
//Include our Image Saving library.
#include "UCB/ImageSaver.h"
#include "FreeImage.h"

#endif /* GLOBAL_H_ */
