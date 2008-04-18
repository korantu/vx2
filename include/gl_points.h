#ifndef __GL_POINTS_H__
#define __GL_POINTS_H__

#include "fastvolume.h"
#include "loader.h"
#include "v3tools.h"

struct GlPoints {

GlPoints(): cursor(128,128,128){
  };

  int viewport[4];
  double modelview[16];
  double projection[16];


  Loader loader;
  FastVolume vol;
  float pnt;
  V3f cursor;
  std::vector<int> list;

  void pick(int,int); //pick a point from the volume


  bool load(char *);
  bool save(char *);
  void draw();
  void point_size(float _pnt){pnt = _pnt;};
};


#endif






