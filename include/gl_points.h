#ifndef __GL_POINTS_H__
#define __GL_POINTS_H__

#include "fastvolume.h"
#include "loader.h"
#include "v3tools.h"
#include "glfw.h"  //TODO included only for GLint/GLdouble; try to remove

struct GlPoints {

GlPoints(): cursor(128,128,128), tool(0) {
  };

  GLint viewport[4];
  GLdouble modelview[16];
  GLdouble projection[16];


  int tool;

  Loader loader;
  FastVolume vol;
  double pnt;
  float tw_cursor_hit; //where cursor hits the surface.
  float tw_cursor_depth; //how far after hit cursor goes
  float tw_mri_value; //what is the mri value at the cursor is
  double tw_pnt;
  bool tw_pnt_smooth;
  V3f cursor;
  void set_level(float l);

  std::vector<int> list;

  void pick(int,int); //pick a point from the volume


  void findSurface(int n);

  bool load(const char *);
  bool save(const char *);
  void draw();
  void point_size(float _pnt){pnt = _pnt;};

  void set_cursor(V3f);

  void gui();
};


#endif






