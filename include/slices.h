#ifndef __SLICES_H__
#define __SLICES_H__

#include "slices.h"
#include "fastvolume.h"
#include "v3.h"
#include "gl_points.h"
#include "misc.h"

struct slices{

  struct slice{
    unsigned char * data;
    slice(int w, int h);
    slice();
  };

  ///for detecting what point should go to what slice.
  /// knowing dimensions and widh of tiles, we ca find the origin.
  V3f center; 
  V3f dx;
  V3f dy;
  V3f dz;
  V3f display_center;

  bool pick(int x, int y, V3f & res);
  bool locate(V3f pos, int &x, int &y);

  typedef slice (* slicep); 

  
  GlPoints * pnts;

  int zoom;

  int scheme;

  int x_orig, y_orig;  //where zero tile starts;
  int xn, yn;          //number of tiles that fit on the screen;
  int tile_w, tile_h;  
  int width, height;
  slicep * tiles; //get space for all buffers

  float area_x, area_y; // percentage of area we want covered.
  int xn_toshow;
  int yn_toshow;
  bool update_needed;
  bool show_mask;

  void update(FastVolume & in, V3f _center);//, V3f _dx, V3f _dy, V3f _dz);
  void update(FastVolume & in);
  void update(){update_needed = true;};
  void draw();
  void draw_box();
  void setup_projection();
  void resize_all(int _width, int _height, int _tile_w, int _tile_h);
  void resize_screen(int _width, int _height);
  void resize_tile(int _tile_w, int _tile_h);
  void switch_crossections();
  void tiles_coverage(float _area_x, float _area_y); //what percentage we want covered with tiles
  void allocate_store(int _width, int _height, int _tile_w, int _tile_h);
  void free_store();
  

  slices(GlPoints * _p, int _width, int _height, int _tile_w, int _tile_h);
  slices(GlPoints * _p);
  ~slices();
};

#endif //__SLICES_H__













