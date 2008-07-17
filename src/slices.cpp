#include "slices.h"
#define GLFW_DLL
#include "GL/glfw.h"
#include "color.h"

void slices::free_store(){
  for(int i = 0; i < xn*yn; i++)delete tiles[i];
  delete[] tiles;
  tiles = 0;
};

slices::slice::slice(int w, int h){
  data = new unsigned char [w*h*3];
  if(!data)throw("Cannot allocate slice");
  for(int i = 0; i < w*h*3; i++){data[i]=(i%255);};
};

slices::slice::slice(){
  delete data;
};


void slices::tiles_coverage(float _ax, float _ay){
  area_x = _ax; area_y  = _ay;
  xn_toshow = (int)(xn*_ax);
  yn_toshow = (int)(yn*_ay);
  if(0 == xn_toshow)xn_toshow = 1;
  if(0 == yn_toshow)yn_toshow = 1;

  if(xn_toshow != xn) //then align to the right
    x_orig = width-xn_toshow*tile_w - y_orig;
  else
    x_orig = (width - (xn*tile_w))/2; //put 'em in the center;
  
  update();
  
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
  res = display_center + (dz*idz)+(dy*(idy/zoom))+(dx*(idx/zoom));
  return true;
};


bool slices::locate(V3f pos, int &x, int &y){
  V3f dr; dr = pos - center;
  int idx = (int)(dr.dot(dx)/dx.length2());
  int idy = (int)(dr.dot(dy)/dy.length2());
  int idz = (int)(dr.dot(dz)/dz.length2());
  if(ABS(idx)*zoom > tile_w/2)return false;
  if(ABS(idy)*zoom > tile_h/2)return false;
  if(ABS(idz) > xn_toshow*yn_toshow/2)return false;

  //starting from 0
  idz += xn_toshow*yn_toshow/2;
  
  //find the tile
  int xo = idz % xn_toshow;
  int yo = (idz - xo)/xn_toshow;
 
  x = x_orig + xo*tile_w + (tile_w/2) + (idx*zoom);
  y = y_orig + yo*tile_h + (tile_h/2) + (idy*zoom);
  
  return true;
};

void draw_line(V3f a, V3f b){
  glVertex3f(a.x, a.y, a.z);
  glVertex3f(b.x, b.y, b.z);
};

void draw_edges(V3f o, V3f ex, V3f ey, V3f ez, int fl){
  V3f zero(0,0,0);
  for(int n = 0; n < 3; n++){
    if(!(fl & (1 << n))){
      V3f a = o + ((fl & 1)?ex:zero) + ((fl & 2)?ey:zero) + ((fl & 4)?ez:zero);
      int fb = fl + (1 << n);
      V3f b = o + ((fb & 1)?ex:zero) + ((fb & 2)?ey:zero) + ((fb & 4)?ez:zero);
      draw_line(a, b);
    };
  };
};

void slices::draw_box(){
  V3f o(display_center);
  V3f ex = dx*tile_w/zoom;
  V3f ey = dy*tile_h/zoom;
  V3f ez = dz*xn_toshow*yn_toshow;
  o -= (ex/2);
  o -= (ey/2);
  o -= (ez/2);

  glLineWidth(2.0);
  glBegin(GL_LINES);
  glColor3f(0,1,1);
  //let us make 4 parallel boxes
  int N = 2; ez/=N;
  for(int slab = 0; slab < N; slab++){
    for(int i = 0; i < 7; i++){ //the vertice number; bitmask
      draw_edges(o, ex, ey, ez, i);
    };
     o += ez;
    };
  glEnd();

  glLineWidth(1.0);
};

void slices::draw(){
  setup_projection();
  for(int y = 0 ; y < yn_toshow; y++)
    for(int x = 0; x < xn_toshow; x++){
      float ox = (float)(x_orig+x*tile_w);
      float oy = (float)(y_orig+y*tile_h);
      glRasterPos3f( ox, oy, 0);
      glDrawPixels(tile_w, tile_h, GL_RGB, GL_UNSIGNED_BYTE, tiles[xn*y+x]->data);
      //frame
      glBegin(GL_LINES);
      glColor3f(0,1,1);
      glVertex3f(ox,oy,0);glVertex3f(ox,tile_h+oy,0);
      glVertex3f(ox,tile_h+oy,0);glVertex3f(tile_w+ox, tile_h+oy,0);
      glVertex3f(tile_w+ox,tile_h+oy,0);glVertex3f(tile_w+ox,oy,0);
      glVertex3f(tile_w+ox,oy,0);glVertex3f(ox,oy,0);
      glEnd();
    };
  
  int x; int y; int cs = 10*zoom;
  
  locate(pnts->cursor, x, y);
  printf("Setting cursor at %f %f %f, getting %d %d\n", pnts->cursor.x, 
	 pnts->cursor.y,
	 pnts->cursor.z, x, y);
  locate(center, x, y);
  
  glDisable(GL_DEPTH_TEST);
  

  glBegin(GL_LINES);
  glColor3f(1,0,0); 
  glVertex3i(x-cs,y,0);  glVertex3i(x+cs,y,0);
  glColor3f(0,0,1); 
  glVertex3i(x,y-cs,0);  glVertex3i(x,y+cs,0);
  glEnd();
  

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

void slices::update(FastVolume & in, V3f _center = V3f(-1,-1,-1))//, V3f _dx, V3f _dy, V3f _dz){
{
  //ok, setting minimum und maximum
  in.min_x = (int)(_center.x - 40);
  in.max_x = (int)(_center.x + 40);
  in.min_y = (int)(_center.y - 40);
  in.max_y = (int)(_center.y + 40);
  in.min_z = (int)(_center.z - 40);
  in.max_z = (int)(_center.z + 40);
  //and center
  if(_center.x > 0)in.center = _center;

  //cool. whenther or not to use it is defined in
  //gui.cpp

  ColorMapper m;
  scheme_fill(m, this->scheme);
  //the_volume = &in;
  
  //  dx = _dx; dy = _dy; dz=_dz; 
  //we don not want to change orientation while updating; the parameters should just go
  // a second version of the slicer will fix it.
//_dx = dx; _dy = dy; _dz=dz; 
  //center=_center;
  
  V3f c(display_center); c -= (dz*((xn_toshow*yn_toshow)/2+1));
  for(int y = 0 ; y < yn_toshow; y++)
    for(int x = 0; x < xn_toshow; x++){
      c+=dz;
      in.raster( c, dx, dy, tile_w, tile_h, tiles[xn*y+x]->data, m, zoom, show_mask);
    };
  update_needed = false;
  
};

void slices::update(FastVolume & in){
  /// if(the_volume)
  update(in, display_center);//, dx, dy,dz);
}

void slices::switch_crossections(){
  V3f ndz = dy;
  V3f ndx = dz;
  dy = dx;
  dz = ndz;
  dx = ndx; 
  //rotate unit vectors, to give better projection.
  update();
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
  tiles_coverage(0.25, 1.0);
  update();
  
};

slices::slices(GlPoints * _pnts, int _width, int _height, int _tile_w, int _tile_h){
  display_center = V3f(128,28,128);
  pnts = _pnts;
  allocate_store(_width, _height, _tile_w, _tile_h);
  area_x = 1.0; area_y = 1.0;
  zoom = 2;
  scheme = 0;
  show_mask = true;
  //the_volume = 0;
};

//dummy screen; will be resized.
slices::slices(GlPoints * _pnts){
  display_center = V3f(128,28,128);
  pnts = _pnts;
  allocate_store(201, 201, 150, 150);
  area_x = 1.0; area_y = 1.0;
  zoom = 2;
  scheme = 0;
  show_mask = true;
  dx = V3f(1,0,0);
  dy = V3f(0,1,0);
  dz = V3f(0,0,1);
  //the_volume = 0;
};

slices::~slices(){
  free_store();
};


















