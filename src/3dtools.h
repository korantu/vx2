// 3D visualization tools.

#ifndef __3dtools_h__
#define __3dtools_h__

#include "v3.h"
#include "fastvolume.h"

//Crossection of a FastVolume.

class Crossection {
  Crossection & Resize(int, int);
  Crossection & Position(V3f &);
  Crossection & Orientation(V3f &);
  Crossection & Draw(); 

 private:

  Crossection & Update(const FastVolume &);
  
  V3f position_;
  V3f orientation_;
  int width_; 
  int height_;
};

/*
  Simplest interface to OpenGL;
*/

// Visualization code; Should be easily copyable around.
/* Drawable is a thin wrapper around an object to visualize.
 It can assume that 0,0,0 is the center, and everything
should be placed inside a 256 cube.*/

class Drawable {
  void Draw() = 0;
};

//Should be draggable around with a mouse;
class InteractiveRenderer: public Validatable {
public:
  InteractiveRenderer(Drawable &);
  ~InteractiveRenderer();
  void DrawFrame(); // Waits for events;
  void Loop();

  Drawable & scene;
};

#endif // __3dtools_h__
