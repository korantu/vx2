#include "color.h"

#include <string.h>
#include <vector>
#include "v3.h"

ColorMapper::ColorMapper(){
  for(unsigned int i = 0; i < 256; i++)memset((void *)(&(col[i*3])), i, 3);
};

void ColorMapper::map(void * where, int i){
  memcpy(where, (void *)(&(col[(i%255)*3])), 3);
};
 
void ColorMapper::map(int & r, int & g, int &b, int i){
  i = i % 255;
  r = col[i*3];
  g = col[i*3+1];
  b = col[i*3+2];
};


int start;
V3f from;

void fill_init(V3f _f){start = 0; from = _f;}; 

void fill(ColorMapper & in, int end, V3f to){
  if(end > 255)end = 255;
  V3f dc(to);
  dc -= from;
  dc /=(end-start);
  V3f cur(from);

  for(int i = start; i < end; i++){
    in.col[i*3] = cur.x;
    in.col[i*3+1] = cur.y;
    in.col[i*3+2] = cur.z;
    cur += dc;
  };

  start = end;
  from = to;
};


#include <stdlib.h>
#include <stdio.h>
  
void scheme_fill(ColorMapper & in, int scheme){
  printf("sch(%d)\n", scheme);

  switch(scheme){
  case 0:
    fill_init( V3f(0,0,0) );
    fill(in, 255, V3f(255,255,255));
    // fill(in, 255, V3f(0,0,0));
    break;
  case 1:
    fill_init( V3f(0,0,0) );
    fill(in, 20, V3f(30,244,0));
    fill(in, 50, V3f(30,0,0));
    fill(in, 100, V3f(60,140,0));
    fill(in, 255, V3f(0,0,0));
    // fill(in, 255, V3f(0,0,0));
    break;
  case 2:
    fill_init( V3f(0,0,255) );
    fill(in, 50, V3f(30,30,40));
    fill(in, 100, V3f(30,200,10));
    fill(in, 200, V3f(60,140,10));
    fill(in, 255, V3f(0,0,0));
    // fill(in, 255, V3f(0,0,0));
    break;
  case 3:
    fill_init( V3f(255,255,255) );
    fill(in, 50, V3f(30,30,30));
    fill(in, 255, V3f(0,0,0));
    // fill(in, 255, V3f(0,0,0));
    break;
  case 4:
    fill_init( V3f(0,0,0) );
    fill(in, 20, V3f(100,0,0));
    fill(in, 60, V3f(0,244,0));
    fill(in, 80, V3f(0,0,0));
    fill(in, 255, V3f(244,244,0));
    // fill(in, 255, V3f(0,0,0));
    break;
  };
};






