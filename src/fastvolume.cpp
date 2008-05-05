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
	int off = getOffset(x,y,z);
	vol[off] = arr[x+width*y+width*height*z];
	if(vol[off] < 1)
	  mask[off]=ZRO;
	else
	  mask[off]=0;
      }; 	
};

void FastVolume::reset(){
  //init mask
  for(int i = 0; i < n_voxels; i++){
    mask[i] = 0;
    depth[i] = 254;
   };
  markers.clear();
  cur_gen=1;
};

void FastVolume::reseed(){
  int i, j, cur;
  markers.clear();

  for(i = getOffset(1,1,1); i < getOffset(254,254,254); i++){
    if(BDR & mask[i]){
      mask[i] -= BDR; mask[i] |= MSK; //just make sure we seed what we seed
    };
    if((mask[i] & MASK) && !(vol[i] & ZRO)){ 
      for(j = 0; j < 6; j++){
	cur =  i+neighbours[j];
	if(mask[cur]==0){
	  mask[i]=mask[i] | MASK; //we don't care if it is also a mask 
	  markers.push_back(i);
	  j = 100; //next point, please
	};//if(mask[cur]==0)
      };//for(j = 0; j < 6; j++)
    };
  };
};

FastVolume::~FastVolume(){
  delete[] vol;
  delete[] mask;
  delete[] depth;
};


void FastVolume::findSurface(std::vector<int> & res, int border){
  cur_gen = 1;
  for(int cur = getOffset(1,1,1); cur < getOffset(255,255,255); cur++){
    if(vol[cur] < 1)mask[cur] |= ZRO;
  };

  for(int cur = getOffset(1,1,1); cur < getOffset(255,255,255); cur++){
    if(!(mask[cur] & ZRO))
      if((mask[cur+dx] & ZRO) ||
	 (mask[cur-dx] & ZRO) ||
	 (mask[cur+dy] & ZRO) ||
	 (mask[cur-dy] & ZRO) ||
	 (mask[cur+dz] & ZRO) ||
	 (mask[cur-dz] & ZRO))  res.push_back(cur);
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
void FastVolume::raster(V3f o, V3f _dx, V3f _dy, int w, int h, unsigned char * buf, ColorMapper & mapper, int zoom, bool show_mask){
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
	if(!((MASK | TRU) & mask[offset]) || !(show_mask)){
	  mapper.map((void *)(&(buf[pos*3])), vol[offset]);
          //buf[pos*3]=(depth[offset]*20)%256;
	}else{
	  buf[pos*3]=(mask[offset] & BDR)?255:0; 
	  buf[pos*3+1]=(mask[offset] & MSK)?255:0; 
	  buf[pos*3+2]=(mask[offset] & TRU)?255:0;;
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
  if(BDR & mask[offset])
    {
      markers.push_back(offset);
      mask[offset] = BDR | cur_gen;
    };
};

//here we add tools; to be replaced with proper structures
void FastVolume::use_tool(int idx, int what){
  switch(what){
  case 0:// do nothing
    break;
  case 1:
    if(!(BDR & mask[idx])){
      mask[idx]=BDR | cur_gen;
      markers.push_back(idx);
    };
  };
};

bool lookahead(FastVolume * in, std::vector<int> &res, int start, int dir, int amount, int cur_gen){
  int cur = start;
  for(int i = 0; i <= amount; i++){
    cur += dir;
    if(in->mask[cur] & (ZRO | MASK)){//we are out; mark everything in between
      for(int j = i; j >= 0; j--){
	cur -= dir;	
	if(!(BDR & in->mask[cur])){
	  if(GEN((in->mask[cur])) != 0) //check if the cell already had a generation
	    in->mask[cur] = BDR | (GEN(in->mask[cur]));
	  else
	    in->mask[cur] = BDR | cur_gen;

	  res.push_back(cur);
	    };
	
      };
      return true;  // was able to jump
    };
  };
  return false; //no luck; just propagate one step.
};

void FastVolume::propagate(int threshold, int dist, int max_depth, int times){
  cur_gen++;
  int cur, cur_val, cursor_idx;
  std::vector<int> res;
  int cur_idx;  

  for(int iter = 0; iter < times; iter++){
    res.clear();
  //every point
    for(std::vector<int>::iterator i = markers.begin(); i != markers.end(); i++){
      cur = *i;
      if(!GEN(mask[cur]))mask[cur]=cur_gen | MSK; //mark it as mask
      //every neighbour
      for(int j = 0; j < 6; j++){
	cur_idx = cur + neighbours[j];
	cur_val = vol[cur_idx];
	if(MASK & mask[cur_idx])continue;
	//try lookahead first; proceed as normal if unsuccessful;
	if(!lookahead(this, res, cur, neighbours[j], dist, cur_gen)){
	  if((!(ZRO & mask[cur_idx])) && (cur_val < threshold) && depth[cur_idx] < max_depth){
	    if(!(BDR & mask[cur_idx])){
	      mask[cur_idx] = BDR | mask[cur_idx];
	      res.push_back(cur_idx);
	    };
	  };
	};
      };
    };
    markers = res;
    printf("Markers %d\n", markers.size());
  };
  if(cur_gen > (GEN_MAX-3))downshift(MASK);
  reseed();
};

void FastVolume::undo(){
  for(int i = getOffset(1,1,1); i < getOffset(255,255,255); i++){
    if((mask[i] & MASK))printf("%d gen, while cur. gen %d\n", GEN(mask[i]), cur_gen);
    if((mask[i] & MASK) && ((GEN(mask[i])) >= cur_gen)){ 
      mask[i]=(FLAGS & (mask[i]-(mask[i] & MASK)));
      printf(".\n");
    };
  };//each point  

  if(cur_gen > 1)cur_gen--;
  
  markers.clear();
  reseed();

};

void FastVolume::downshift(int flags = MASK){
  for(int i = getOffset(1,1,1); i < getOffset(255,255,255); i++){
    if((flags & mask[i]) && (GEN(mask[i]) > 0))
      { //ok, we want to deal with this voxel- it is our type and can be downshifted
	mask[i] = /*flags*/((0xff-GEN_MAX) & mask[i]) + /*gen-1*/(GEN(mask[i])-1);
    
      };
  };
};
