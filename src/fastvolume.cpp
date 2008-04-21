#include "fastvolume.h"

FastVolume::FastVolume()
{
  vol = new t_vox[256*256*256]; 
  if(!vol)throw "Out of memory\n";
};

void FastVolume::copy(t_vox * arr, int width, int height, int depth)
{
  for(int i = 0; i < 0xff*0xff*0xff; i++) vol[i] = 0; //empty
  for(int z = 0; z < depth; z++)
    for(int y = 0; y < height; y++)
      for(int x = 0; x < width; x++){
	vol[getOffset(x,y,z)] = arr[x+width*y+width*height*z]; 
      }; 	
};

FastVolume::~FastVolume(){
  delete[] vol;
};

void FastVolume::findSurface(std::vector<int> & res, int border){
  for(int z = 1; z < 0xfe; z++)
    for(int y = 1; y < 0xfe; y++){
      int max_cur = getOffset(0xfe,y,z);
      for(int cur = getOffset(1,y,z); cur < max_cur; cur++){
	if(vol[cur] > border)
	  if(vol[cur+dx] <= border ||
	     vol[cur-dx] <= border ||
	     vol[cur+dy] <= border ||
	     vol[cur-dy] <= border ||
	     vol[cur+dz] <= border ||
	     vol[cur-dz] <= border)  res.push_back(cur);
      };
    };
};

void FastVolume::iterate(Iterator & it){
  for(int z = 1; z < 0xfe; z++)
    for(int y = 1; y < 0xfe; y++){
      int max_cur = getOffset(0xfe,y,z);
      int cur = getOffset(1,y,z);     
      it.line(cur, max_cur);
    };
};


inline bool valid(int x, int y, int z){
  return ((x < 256) && (y < 256) && (z < 256) && (x > 0) && (y > 0) && (z > 0)); 
};

/// slice an input buffer
void FastVolume::raster(V3f o, V3f _dx, V3f _dy, int w, int h, unsigned char * buf){
  int pos = 0;
  V3i cur((int)o.x, (int)o.y, (int)o.z);     //integers should be seriously faster
  V3i dx((int)_dx.x, (int)_dx.y, (int)_dx.z); 
  V3i dy((int)_dy.x, (int)_dy.y, (int)_dy.z);
  cur -= (dx*w/2);
  cur -= (dy*h/2);
  for(int y = 0; y < h; y++){
    V3i line = cur;
    for(int x = 0; x < w; x++){
      if(valid(cur.x, cur.y, cur.z))
	buf[pos] = vol[getOffset(line.x, line.y, line.z)];
      else
	buf[pos] = 0;
      pos++;
      line += dx;
    };
    cur += dy;
  };
};




