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



#endif // __3dtools_h__
