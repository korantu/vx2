#ifndef __GL_SLICER_H__
#define __GL_SLICER_H__

#include "fastvolume.h"

struct GlSlicerI {
  static GlSlicerI * create(const FastVolume &);
  virtual void setup3dTexture() = 0;
};

#endif // __GL_SLICER_H__
