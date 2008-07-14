#ifndef __navigator_h__
#define __navigator_h__

#define GLFW_DLL
#include "glfw.h"

///mouse delta can be replaced by simple x/y struct; no need to accumulate anything;

#include "v3.h"

struct camera {
  V3f center;
  V3f eye;
  V3f up;

  camera();
};

/// Move a camera by dx, dy; dx/dy can be obtained from gui
camera & move(camera &, int dx, int dy);

///project camera
void lookAt(camera & to_look);


#endif // __navigator_h__
