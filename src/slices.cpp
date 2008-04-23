#include "slices.h"
#include "glfw.h"


void slices::free_store(){
  for(int i = 0; i < xn*yn; i++)delete tiles[i];
  delete[] tiles;
  tiles = 0;
};

slices::slice::slice(int w, int h){
  data = new unsigned char [w*h];
  if(!data)throw("Cannot allocate slice");
  for(int i = 0; i < w*h; i++){data[i]=(i%255);};
};

slices::slice::slice(){
  delete data;
};


void slices::tiles_coverage(float _ax, float _ay){
  area_x = _ax; area_y  = _ay;
  xn_toshow = xn*_ax;
  yn_toshow = yn*_ay;
  if(0 == xn_toshow)xn_toshow = 1;
  if(0 == yn_toshow)yn_toshow = 1;
  
};

bool slices::pick(int x, int _y, V3f & res){
  int y = height - _y;
  if( (x < x_orig) || (x > x_orig+xn_toshow*tile_w) ) return false;
  if( (y < y_orig) || (y > y_orig+yn_toshow*tile_h) ) return false;
  int nnx = (x-x_orig)/tile_w; //tile pos.
  int nny = (y-y_orig)/tile_h;
  int total = xn_toshow*yn_toshow;
  int idz = nnx+(xn_toshow*nny)-(total/2);
  int idx = (x - nnx*tile_w - x_orig) - tile_w/2;
  int idy = (y - nny*tile_h - y_orig) - tile_h/2;
  res = center + (dz*idz)+(dy*idy)+(dx*idx);
  return true;
};


void slices::allocate_store(int _width, int _height, int _tile_w, int _tile_h){
  
  ///calcualting stuff
  width = _width; height = _height;
  tile_w = _tile_w; tile_h = _tile_h;
  xn = (width) / tile_w; yn = (height) / tile_h;
  x_orig = (width - (xn*tile_w))/2; //put 'em in the center;
  y_orig = (height - (yn*tile_h))/2; 
  
  tiles_coverage(1.0, 1.0);
  

  tiles = new slicep[xn*yn];

  printf("sc:%dX%d; tiles:%dX%d; org:%dX%d\n", width, height, xn, yn, x_orig, y_orig);

  //now we can init buffers:
  for(int i = 0; i < xn*yn; i++){
    try{
      tiles[i] = new slice(tile_w, tile_h);
    }catch(...){
      do{ i--; delete tiles[i]; }while(i>0);
      delete[] tiles;
      throw "failed to allocate array of slices;";
    };
  };

};

void slices::setup_projection(){
  //proper texture handling
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  
  glDisable(GL_DEPTH_TEST); //render anyway;
  //where we want our tiles
  glViewport(0, 0, width, height);
  //pixel/by/pixel
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, width, 0, height, -2, 2);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
};

void slices::update(FastVolume & in, V3f _center, V3f _dx, V3f _dy, V3f _dz){

  dx = _dx; dy = _dy; dz=_dz; center=_center;
  
  V3f c(_center); c -= (_dz*((xn_toshow*yn_toshow)/2));
  for(int y = 0 ; y < yn_toshow; y++)
    for(int x = 0; x < xn_toshow; x++){
      c+=_dz;
      in.raster( c, _dx, _dy, tile_w, tile_h, tiles[xn*y+x]->data);
    };
};


void slices::draw(){
  setup_projection();
  for(int y = 0 ; y < yn_toshow; y++)
    for(int x = 0; x < xn_toshow; x++){
      float ox = (float)(x_orig+x*tile_w);
      float oy = (float)(y_orig+y*tile_h);
      glRasterPos3f( ox, oy, 0);
      glDrawPixels(tile_w, tile_h, GL_GREEN, GL_UNSIGNED_BYTE, tiles[xn*y+x]->data);
      //frame
      glBegin(GL_LINES);
      glColor3f(0,1,1);
      glVertex3f(ox,oy,0);glVertex3f(ox,tile_h+oy,0);
      glVertex3f(ox,tile_h+oy,0);glVertex3f(tile_w+ox, tile_h+oy,0);
      glVertex3f(tile_w+ox,tile_h+oy,0);glVertex3f(tile_w+ox,oy,0);
      glVertex3f(tile_w+ox,oy,0);glVertex3f(ox,oy,0);
      glEnd();
    };



};

//changing layout
void slices::resize_screen(int w, int h){
  resize_all(w, h, tile_w, tile_h);
};


void slices::resize_tile(int tw, int th){
  resize_all(width, height, tw, th);
};


void slices::resize_all(int w, int h, int tw, int th){
  free_store();
  allocate_store(w, h, tw, th);
};

slices::slices(int _width, int _height, int _tile_w, int _tile_h){
  allocate_store(_width, _height, _tile_w, _tile_h);
  area_x = 1.0; area_y = 1.0;
};

//dummy screen; will be resized.
slices::slices(){
  allocate_store(201, 201, 100, 100);
  area_x = 1.0; area_y = 1.0;
};

slices::~slices(){
  free_store();
};
















