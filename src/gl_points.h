#ifndef __GL_POINTS_H__
#define __GL_POINTS_H__

#include "fastvolume.h" //dealing with volumes
#include "loader.h"     //access to loader of volumes
#include "v3tools.h"    //utils
#define GLFW_DLL
#include "GL/glfw.h"  //TODO included only for GLint/GLdouble; try to remove


//set of points to display as a brain
// includes whole volume
// and several point sets for different layers
// controls editing of the volume and updating of the volume.
struct GlPoints {

  // current viewpoint to draw oneself
  GLint viewport[4];
  GLdouble modelview[16];
  GLdouble projection[16];

  // color scheme to use
  int scheme;

  // tool id
  int tool;

  // extent of the tool (effect of this depends on the tool in question.
  int tool_size;

  /// loading volume

  //storage for the raw volume
  Loader loader;

  //active volume array + masks
  FastVolume vol;

  // GUI-controlled params
  //point size
  double pnt;
  float tw_cursor_hit; //where cursor hits the surface.
  float tw_cursor_depth; //how far after hit cursor goes
  float tw_mri_value; //what is the mri value at the cursor is
  double tw_pnt;  
  float tw_transparency; // alpha for the point set
  bool tw_pnt_smooth;    //use smooth points (slow)
  V3f cursor;

GlPoints(): tool(0),  tw_transparency(0.35f),  cursor(128.0f,128.0f,128.0f), cur_level(1) {
  
  tool = 0;
  tool_size=1;
};


  //choose a level to display
  void set_level(float l);

  //set currect color scheme
  void set_scheme(int sch); ///set color scheme

  //storage for surface layers
  std::vector<int> list[15]; //10 top layers
  int cur_level; //the level we want to

  void pick(int,int); //pick a point from the volume

  //run surface-finding algorithm
  //void findSurface(int n);

  bool load(const char *);
  void find_surface(); ///find the several layers of the loaded volume; not always needed;

  bool save(const char *);

  //draw point cloud
  void draw(V3f z);

  //init opengl projection  
  void set_projection();

  void point_size(float _pnt){pnt = _pnt;};

  
  void set_cursor(V3f);

  void update();

  void apply();

};


#endif






