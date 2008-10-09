#ifndef __V3TOOLS_H__
#define __V3TOOLS_H__

#include "v3.h"

///Rotate a vector along an axis
V3f rot_x(V3f in, float r);
V3f rot_y(V3f in, float r);
V3f rot_z(V3f in, float r);

void ortoNormalize(V3f &x, V3f &y, V3f &z);

void say(char *, const V3f &);

#endif //__V3TOOLS_H__







