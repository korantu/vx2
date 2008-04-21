#ifndef __SLICES_H__
#define __SLICES_H__

#include "slices.h"
#include "fastvolume.h"


struct slices{
  

  struct slice{
    unsigned char * data;
    slice(int w, int h);
    slice();
  };

  typedef slice (* slicep); 

  int x_orig, y_orig;  //where zero tile starts;
  int xn, yn;          //number of tiles that fit on the screen;
  int tile_w, tile_h;  
  int width, height;
  slicep * tiles; //get space for all buffers
  
  void update(FastVolume & in, V3f _center, V3f _dx, V3f _dy, V3f _dz);
  void draw();
  void setup_projection();
  void resize_all(int _width, int _height, int _tile_w, int _tile_h);
  void resize_screen(int _width, int _height);
  void resize_tile(int _tile_w, int _tile_h);
  void allocate_store(int _width, int _height, int _tile_w, int _tile_h);
  void free_store();

  slices(int _width, int _height, int _tile_w, int _tile_h);
  slices();
  ~slices();
};

#endif //__SLICES_H__













