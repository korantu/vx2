#ifndef __GL_POINTS_H__
#define __GL_POINTS_H__

#include "fastvolume.h"
#include "loader.h"
#include "v3tools.h"
#include "glfw.h"  //TODO included only for GLint/GLdouble; try to remove


struct GlPoints {

GlPoints(): tool(0),  tw_transparency(0.35),  cursor(128,128,128), cur_level(1) {
  tool = 0;
  tool_size=1;
  };

  GLint viewport[4];
  GLdouble modelview[16];
  GLdouble projection[16];

  int scheme;

  int tool;
  int tool_size;

  Loader loader;
  FastVolume vol;
  double pnt;
  float tw_cursor_hit; //where cursor hits the surface.
  float tw_cursor_depth; //how far after hit cursor goes
  float tw_mri_value; //what is the mri value at the cursor is
  double tw_pnt;
  float tw_transparency;
  bool tw_pnt_smooth;
  V3f cursor;
  void set_level(float l);
  void set_scheme(int sch); ///set color scheme

  std::vector<int> list[15]; //10 top layers
  int cur_level; //the level we want to

  void pick(int,int); //pick a point from the volume


  void findSurface(int n);

  bool load(const char *);
  void find_surface(); ///find the several layers of the loaded volume; not always needed;
  bool save(const char *);
  void draw(V3f z);
  void set_projection();
  void point_size(float _pnt){pnt = _pnt;};

  void set_cursor(V3f);

  void update();

  void apply();

};


#endif






