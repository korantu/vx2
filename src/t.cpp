#include "surface.h"

void t_read_surface();
void t_rasterize_surface();
void t_find_center_point();

int main(void){
  
  t_read_surface();
  //  t_rasterize_surface();
  t_find_center_point();


  return 0;
};

char * real_surf = "data/SS127_fs/kdl/rh.pial.asc";
Surface it;
GlPoints pnt;

void t_read_surface(){
  
  read_surface(it, real_surf);
};

void t_rasterize_surface(){

  RenderingTraits tr;

  rasterize_surface(it, pnt, tr);
};

void t_find_center_point(){
  V3f a = find_center_point(it);
  printf("center: %f %f %f\n", a.x, a.y, a.z);
};
