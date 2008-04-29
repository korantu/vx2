#include "fastvolume.h"

const int FastVolume::neighbours[6] = {dx, -dx, dy, -dy, dz, -dz};


FastVolume::FastVolume()
{
  vol = new t_vox[n_voxels]; 
  mask = new unsigned char [n_voxels];
  depth = new unsigned char [n_voxels];
  

  if((!vol) || (!mask) || (!depth)) throw "Out of memory\n";

  reset();
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

void FastVolume::reset(){
  //init mask
  for(int i = 0; i < n_voxels; i++){
    mask[i] = 0;
    depth[i] = 254;
   };
  markers.clear();
};

void FastVolume::reseed(){
  int i, j, cur;
  markers.clear();

  for(i = getOffset(1,1,1); i < getOffset(254,254,254); i++){
    if(mask[i]==255)mask[i]=100; //just make sure we seed what we seed
    if((mask[i] != 0) && (vol[i] != 0)){
      for(j = 0; j < 6; j++){
	cur =  i+neighbours[j];
	if(mask[cur]==0){
	  mask[i]=255; markers.push_back(i);
	  j = 100; //next point, please
	};
      };
    };
  };
};

FastVolume::~FastVolume(){
  delete[] vol;
  delete[] mask;
  delete[] depth;
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

#include "color.h"

/// slice an input buffer
void FastVolume::raster(V3f o, V3f _dx, V3f _dy, int w, int h, unsigned char * buf, ColorMapper & mapper, int zoom){
  //scheme_fill(mapper, 0);
  int pos = 0;
  int offset;
  V3i cur((int)o.x, (int)o.y, (int)o.z);     //integers should be seriously faster
  V3i dx((int)_dx.x, (int)_dx.y, (int)_dx.z); 
  V3i dy((int)_dy.x, (int)_dy.y, (int)_dy.z);
  cur -= (dx*w/2)/zoom;
  cur -= (dy*h/2)/zoom;
  int zoomx = 0;
  int zoomy = 0;
  for(int y = 0; y < h; y++){
    V3i line = cur;
    for(int x = 0; x < w; x++){
      if(valid(cur.x, cur.y, cur.z)){
	offset = getOffset(line.x, line.y, line.z);
	if(!mask[offset]){
	  mapper.map((void *)(&(buf[pos*3])), vol[offset]);
          buf[pos*3]=(depth[offset]*20)%256;
	}else{
	  buf[pos*3]=mask[offset];
	  buf[pos*3+1]=0;
	  buf[pos*3+2]=255;
	}
      }
      else  //completely outside
	mapper.map((void *)(&(buf[pos*3])), 0);
      pos++;
      zoomx ++ ; if((zoomx % zoom) == 0)line += dx;
    };

    //cur += dy;
    zoomy ++ ; if((zoomy % zoom) == 0)cur += dy;
    zoomx = 0;
  };
};

void FastVolume::add_point(V3f &pnt){
  int offset = getOffset(pnt.x, pnt.y, pnt.z); 
  if(mask[offset]!=255)
    {
      markers.push_back(offset);
      mask[offset] = 255;
	};
}; 

bool lookahead(FastVolume * in, std::vector<int> &res, int start, int dir, int amount){
  int cur = start;
  for(int i = 0; i <= amount; i++){
    cur += dir;
    if((in->mask[cur] != 0) || (in->vol[cur] == 0)){//we are out; mark everything in between
      for(int j = i; j >= 0; j--){
	cur -= dir;	
	in->mask[cur] = 255;
	res.push_back(cur);
	
      };
      return true;  //was able to jump
    };
  };
  return false; //no luck; just propagate one step.
};

void FastVolume::propagate(int threshold, int generation, int dist){
  int cur, cur_val, cursor_idx;
  std::vector<int> res;
  int cur_idx;  

  //every point
  for(std::vector<int>::iterator i = markers.begin(); i != markers.end(); i++){
    cur = *i;
    mask[cur]=generation;
    //every neighbour
    for(int j = 0; j < 6; j++){
      cur_idx = cur + neighbours[j];
      cur_val = vol[cur_idx];
      if(mask[cur_idx])continue;
      //try lookahead first; proceed as normal if unsuccessful;
      if(!lookahead(this, res, cur, neighbours[j], dist)){
	if((cur_val > 0) && (cur_val < threshold) && depth[cur_idx] < 10){
	  mask[cur_idx] = 255;
	  res.push_back(cur_idx);
	};
      };
    };
  };
  markers = res;
};
