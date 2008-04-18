#include "v3tools.h"
#include <stdio.h>
#include <stdlib.h>

//print to check
void same(char * msg, float a, float b){
  printf("Same, %s: %f, %f\n", msg, a, b);
};

int main(){
  V3f x(0,0,1); V3f xr(rot_x(x, 0.87)); same("x rotation", x.dot(xr), cos(0.87));
  V3f y(0,0,1); V3f yr(rot_y(y, 0.23)); same("y rotation", y.dot(yr), cos(0.23));
  V3f z(1,0,0); V3f zr(rot_z(z, 0.19)); same("z rotation", z.dot(zr), cos(0.19));

  V3f nx(3,2,5);
  V3f ny(-2,3,4);
  V3f nz(-4,4,3.8);

  V3f nnx(3,2,5);
  V3f nny(-2,3,4);
  V3f nnz(-4,4,3.8);

  ortoNormalize(nnx, nny, nnz);
  
  same("x unit", nnx.length(), 1.0);
  same("y unit", nny.length(), 1.0);
  same("z unit", nnz.length(), 1.0);

  V3f tmp; tmp.cross(nnx, nx);

  same("x colinear", tmp.length(), 0.0);
  
  tmp.cross(nnx, nny); tmp-=nnz; same("x orto", tmp.length(), 0);
  tmp.cross(nny, nnz); tmp-=nnx; same("y orto", tmp.length(), 0);
  tmp.cross(nnz, nnx); tmp-=nny; same("z orto", tmp.length(), 0);


};
