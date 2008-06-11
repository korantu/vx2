#include "surface.h"

#define DMARK(X) do{ \
   printf("\n%s:%d: error: ", __FILE__, __LINE__); \
   printf(X); \
   }while(0)

void t_read_surface();
void t_rasterize_surface();
void t_find_center_point();
void t_refine_triangle();
void t_rasterize_simple();

int main(void){
  
  // t_read_surface();
  t_rasterize_surface();
  // t_refine_triangle();
  // t_find_center_point();
  // t_rasterize_simple();


  return 0;
};

char * real_surf = "data/SS127_fs/kdl/rh.pial.asc";
char * simple_surf = "data/simple_surface.asc";
Surface it;
GlPoints pnt;

void t_read_surface(){
    read_surface(it, real_surf);
};

void t_rasterize_simple(){
  Surface it;
  read_surface(it, simple_surf);
  
  GlPoints pnt;
  
  RenderingTraits t;
  t.inside = false;
  t.tru = true;
  t.half = false;

  rasterize_surface(it, pnt, t);

  //check that one plane was completely filled in.
  for(int i = 108; i <= 118; i++)
    for(int j = 108; j <= 118; j++)
      for(int k = 148; k <= 138; k++){
	if(i == 118){
	  if(!(pnt.vol.mask[pnt.vol.getOffset(i,j,k)] & TRU)){
	    DMARK("Outside, but should be inside.\n");
	  };
	}else{
	  if((pnt.vol.mask[pnt.vol.getOffset(i,j,k)] & TRU)){
	    DMARK("Inside, but has to be outside.\n");	
	  };
	};
      };

  

};

void t_refine_triangle(){
  V3f a = V3f(10,10,10);
  V3f b = V3f(10,10,20);
  V3f c = V3f(10,20,10);
  V3f d = V3f(10,20,20);
  
  GlPoints pnt;
  
  RenderingTraits t;
  t.inside = false;
  t.tru = true;
  t.half = false;

  refine_triangle( a, b, c, pnt, V3f(1,0,0), t);
  refine_triangle( b, c, d, pnt, V3f(1,0,0), t);

  //check that one plane was completely filled in.
  for(int i = 10; i <= 20; i++)
    for(int j = 10; j <= 20; j++)
      for(int k = 10; k <= 20; k++){
	if(i == 10){
	  if(!(pnt.vol.mask[pnt.vol.getOffset(i,j,k)] & TRU)){
	    DMARK("Outside, but should be inside.\n");
	  };
	}else{
	  if((pnt.vol.mask[pnt.vol.getOffset(i,j,k)] & TRU)){
	    DMARK("Inside, but has to be outside.\n");	
	  };
	};
      };

};

void t_rasterize_surface(){
  Surface it;
  read_surface(it, real_surf);

  RenderingTraits tr;

  rasterize_surface(it, pnt, tr);
};

void t_find_center_point(){
  V3f a = find_center_point(it);
  printf("center: %f %f %f\n", a.x, a.y, a.z);
};
