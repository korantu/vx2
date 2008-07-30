#include "fastvolume.h"
#include "misc.h"

#include <algorithm>

const int FastVolume::neighbours[26] = {
  dx, -dx, dy, -dy, dz, -dz,
  dx+dy, dx-dy, dy-dx, -dy-dx,
  dz+dy, dz-dy, dz-dx, -dz-dx,
  dx+dz, dx-dz, dy-dz, -dy-dz,
  dx+dy+dz, 
  dx+dy-dz, 
  dx-dy+dz,
  dx-dy-dz,
  -dx+dy+dz,
  -dx+dy-dz,
  -dx-dy+dz,
  -dx-dy-dz
};


///undo stack
///P - a position
///A - action
/// stack's last item is an action. 
/// when we want to store a part of undo action,
/// the last part is popped out.
std::vector<int> undo_buffer;

//query if the following thing is an action or a position
bool FastVolume::is_action(int in){
	return ((in >= 0) && (in < (int)MAX_ACTION));
};

void FastVolume::undo_action(){
  if(undo_buf.size() == 0)return;
  //	assert(is_action(undo_buffer_multi.back()));
  int pos; //current position; last one is an action.
  std::vector<int> list;
  
  //search until an action.
  while(undo_buf.size() > 0){
    pos = undo_buf.back();
    undo_buf.pop_back();
    if(is_action(pos)){ //ok, found what to do;
      break; 
    }else{              //not action yet; just remember it
      list.push_back(pos); //remember
    };
  };
  
  for(std::vector<int>::iterator i = list.begin(); i != list.end(); i++){
    switch(pos){
    case ADD_TRU:
      mask[*i] -= TRU & mask[*i];
      break;
    case KILL_TRU:
      mask[*i] |= TRU;
      break;  
    };
  };
};	

/// eraze an element from a list.
inline bool erase_element(std::vector<int> & where, int what){
  for(std::vector<int>::iterator i = where.begin(); i != where.end(); i++){
    if(what == *i){
      where.erase( i);
      return true;
    };
  };

  return false;
};


// undo.
void FastVolume::undo(){

  while(undo_buffer.size() && !(undo_buffer.back()))
    undo_buffer.pop_back();

  int cur;
  //undo until a zero
  while(undo_buffer.size() && undo_buffer.back()){
    cur = undo_buffer.back();
    undo_buffer.pop_back();
    if(BDR & mask[cur]){ //possibly marked
		while(erase_element(markers, cur)){}; //make sure it is not in the markers 
    }
    mask[cur] -= (mask[cur] & MASK); //clear the thing itself
    
    for(int i = 0; i < 6; i++){
      int cur_nbr = cur+neighbours[i];
      if((MSK & mask[cur_nbr]) && !(BDR & mask[cur_nbr])){ //if not marked
	mask[cur_nbr] |= BDR;     //mark
	markers.push_back(cur_nbr);
      };
    };

  };
  

};


FastVolume::FastVolume()
{
  vol = new t_vox[n_voxels]; 
  mask = new unsigned char [n_voxels];
  depth = new unsigned char [n_voxels];
  
  use_scope = false;

  if((!vol) || (!mask) || (!depth)) throw "Out of memory\n";

  reset();
};

//check if the current offset is inside the scope
bool FastVolume::in_scope(int off){
  if(!use_scope)return true;
  int x, y, z;
  getCoords(off, x, y, z);
  printf("%d:%d:%d\n", x, y, z);

  return(
	 (x > min_x) && (x < max_x) && 
	 (y > min_y) && (y < max_y) &&
	 (z > min_z) && (z < max_z) 
	);
  
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
  undo_buffer.clear();
  undo_buf.clear();
};

void FastVolume::reseed(){
  int i, j, cur;
  markers.clear();

  

  for(i = getOffset(1,1,1); i < getOffset(254,254,254); i++){
    if(BDR & mask[i]){
      mask[i] -= BDR; mask[i] |= MSK; //just make sure we seed what we seed
    };
    if((mask[i] & MASK) && !(mask[i] & (ZRO|TRU))){ 
      //iterate neighbours
      for(j = 0; j < 6; j++){
	cur =  i+neighbours[j];
	if(!(mask[cur]&(MASK | ZRO | TRU))  && in_scope(cur)){ //if any pixel around is not mask, then...
	  mask[i] |= BDR; //we don't care if it is also a mask 
	  markers.push_back(i);
	  break; //next point, please
	}; //if(mask[cur]==0)
      }; //for(j = 0; j < 6; j++)
    };
  };
};

//flood-fill an AUX flag from the center
//the points without such flag will recieve a mask

void FastVolume::scan_for_disconnected_regions(){
  //clean first
  for(int i = 0; i < 256*256*256; i++){
    mask[i]-=(AUX & mask[i]); //make sure it is not there.
  };

  std::vector<int> buf;
  std::vector<int> buf_tmp;

  buf.push_back(getOffset(127,127,127)); //pray it is inside.
  mask[buf.back()] |= AUX;

  //propagate using AUX mask from the center
  while(buf.size() > 0){
    // go over the buffer
    for(std::vector<int>::iterator p = buf.begin(); p != buf.end(); p++){
      int cur = *p;
      //check the neighbours.
      for(int j = 0; j < 6; j++){
	int cur_nbr = cur+neighbours[j];
	if(!(AUX & mask[cur_nbr]) && !((ZRO | MASK) & mask[cur_nbr]) && (vol[cur_nbr] > 0)){
	  mask[cur_nbr] |= AUX;
	  buf_tmp.push_back(cur_nbr);
	};
      };
    };
    buf = buf_tmp;
    buf_tmp.clear();
  };

  //scan and mark all non-auxed staff
  for(int i = 0; i < 256*256*256; i++){
    if(!(AUX & mask[i]) && !(ZRO & mask[i]) &&(vol[i]>0))mask[i] |= MSK;
    mask[i] -= (AUX & mask[i]);
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
    if(vol[cur] == 0)mask[cur] |= ZRO;
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

/*
void FastVolume::iterate(Iterator & it){
  for(int z = 1; z < 0xfe; z++)
    for(int y = 1; y < 0xfe; y++){
      int max_cur = getOffset(0xfe,y,z);
      int cur = getOffset(1,y,z);     
      it.line(cur, max_cur);
     };
};
*/

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
	  mapper.map((void *)(&(buf[pos*3])), vol[offset]);
	  int flags[] = {BDR, MSK, TRU};

	  for(int i = 0; i < 3; i++){
	    {
	      buf[pos*3+i]+=(mask[offset] & flags[i])?
		((buf[pos*3+i] < 55)?200:255-buf[pos*3+i]):
		0;
	    }; 
	  };

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

/* Unused

void FastVolume::add_point(V3f &pnt){
  int offset = getOffset(pnt.x, pnt.y, pnt.z); 
  if(BDR & mask[offset])
    {
      markers.push_back(offset);
      undo_buffer.push_back(offset);
      mask[offset] = BDR | cur_gen;
    };
  undo_buffer.push_back(0); 
};
*/

void FastVolume::use_tool(int idx, int what, int sz){

  int x, y, z;
  int c;

  if(!what)return;

  getCoords(idx, x,y,z);
  x-=sz/2; y-=sz/2; z-=sz/2; 

  /// a tool is applied in 3d box [sz x sz x sz]
  for(int xi = x; xi < x+sz; xi++)
    for(int yi = y; yi < y+sz; yi++)
      for(int zi = z; zi < z+sz; zi++){
	c = getOffset(xi, yi, zi);
	switch(what){
	case 1: //add seeds
	  if(!(BDR & mask[c])){   //if not mask already
	    mask[c]=BDR | cur_gen;
	    markers.push_back(c);
	  };
	  break;
	case 2: //re-prime
	  if(mask[c] & MSK){
	    if(!(BDR & mask[c])){   //if not mask already
	      mask[c]=BDR | cur_gen;
	      markers.push_back(c);
	    };
	    //ok, that was border
	    break;
	  };
	  break;
	case 3: //de-prime
	  if(mask[c] & BDR){
	    mask[c] -= (mask[c] & BDR); 
	    for(unsigned int k = 0; k < markers.size(); k++)
	      if(markers[k] == c)markers[k] = 500; 
	    //set it to some arbitrary out of brain point
	  };
	  break;
	case 4: //add truth
	  mask[c] |= TRU; break;
	case 5: //clean everything
	  mask[c] -= ((MASK | TRU) & mask[c]); break;
	};
	  if(what == 1) undo_buffer.push_back(c);
  };

};

bool lookahead(FastVolume * in, std::vector<int> &res, int start, int dir, int amount, int cur_gen){
  int cur = start;
  for(int i = 0; i <= amount; i++){
    cur += dir;
    if(in->mask[cur] & TRU)return false;
    if(in->mask[cur] & (ZRO | MASK)){//we are out; mark everything in between
      for(int j = i; j > 0; j--){
	cur -= dir;	
	if(!(BDR & in->mask[cur])){
	  if(GEN((in->mask[cur])) != 0) //check if the cell already had a generation
	    in->mask[cur] = BDR | (GEN(in->mask[cur]));
	  else
	    in->mask[cur] = BDR | cur_gen;

	  res.push_back(cur);
	  undo_buffer.push_back(cur);
	    };
	
      };
      return true;  // was able to jump
    };
  };
  return false; //no luck; just propagate one step.
};

bool lookahead_spread(FastVolume * in, std::vector<int> &res, int start, int dir, int amount){
  int cur = start;
  for(int i = 0; i <= amount; i++){
    cur += dir;
    if(in->mask[cur] & TRU)return false;
    if(in->mask[cur] & (ZRO | MASK)){//we are out; mark everything in between
      for(int j = i; j > 0; j--){
	cur -= dir;	
	if(!(BDR & in->mask[cur])){
	  in->mask[cur] |= BDR;
	  res.push_back(cur);
	  undo_buffer.push_back(cur);
	};
      };

      return true;  // was able to jump
    };
  };
  return false; //no luck; just propagate one step.
};


struct step{
  int from;
  int to;
  float score;

  bool operator() (const step & me, const step & other) const {
    return me < other;
  };

  bool operator< (const step & other) const{
    return(score > other.score);
  };

  
};

float smooth_bell(float x){
  if(x<0)x=-x;
  if(x>1)return 0.0f;
  float square_x = x*x;
  return 2*square_x*x-3*square_x+1; // 2*x^3-3*x^2+1, unit bell.
};


//calculate band of currently selected voxels
void FastVolume::set_band(){
	//analyzing stuff:
	int min = 10000;
	int max = 0;
	for(std::vector<int>::iterator c = markers.begin(); c!=markers.end(); c++){
		int cur = vol[*c];
		if (cur > max)
			max = cur;
		if (cur < min)
			min=cur;
	};

	half_band_size = (float)(max-min)/2.0f;
	//add margins
	half_band_size = half_band_size*1.2f;
	band_center = (float)(max+min)/2;
	// go around same points and plan the move
	printf("band: half:%f, center:%f\n",  half_band_size, band_center);

};

void FastVolume::propagate_spread(int threshold, int dist, int max_depth, int times){

  // cur_gen++;
  int cur, cur_val;//, cursor_idx;
  std::vector<int> res;
  int cur_idx;  

  std::vector<step> steps;


  if(undo_buffer.size() && undo_buffer.back())undo_buffer.push_back(0);

  //iterations
  for(int iter = 0; iter < times; iter++){
    res.clear();
    steps.clear();
  //every point
    for(std::vector<int>::iterator i = markers.begin(); i != markers.end(); i++)
      {
      cur = *i;
      //if(!GEN(mask[cur]))mask[cur]=cur_gen | MSK; //mark it as mask
      if(mask[cur] & TRU)continue;
      mask[cur] -= MASK & mask[cur];
      mask[cur] |= MSK;
      //every neighbour
      for(int j = 0; j < 6; j++){
	cur_idx = cur + neighbours[j];
	cur_val = vol[cur_idx];
	if(MASK & mask[cur_idx]){
	  continue;
	  //mask[cur_idx] -= (BDR & (mask[cur_idx]);
	};
	//try lookahead first; proceed as normal if unsuccessful;
	if(!lookahead_spread(this, res, cur, neighbours[j], dist)){
	  if(!((ZRO | TRU | MASK) & mask[cur_idx])){
	    if(!(BDR & mask[cur_idx])){
	      step a_step = {cur, cur_idx, 0};
	      steps.push_back(a_step);
	    };
	  };
	};
      };
    };

    //now we got all the options, let's rank them

    float max = 0;
    float min = 100000;
    
    float worst_score = 0.0f; 
    

    /// probability of each particular voxel being good,
    /// according to certain criteria.
    /// the higher probability the more chances we'll go in that direction

  ///estimate axuilary parameters
  
  int max_delta = 0;
    for(std::vector<step>::iterator i = steps.begin(); i != steps.end(); i++){
      step the_step = *i;
      float delta =  (float)ABS(vol[the_step.to]-vol[the_step.from]);
      if(delta > max_delta)max_delta = (int)delta;
    };

    for(std::vector<step>::iterator i = steps.begin(); i != steps.end(); i++){
      step the_step = *i;
      float delta =  (float)ABS(vol[the_step.to]-vol[the_step.from]);
      float in_band = smooth_bell((vol[the_step.to]-band_center)/half_band_size);
      float friends=0;
      for(int k = 0; k < 26; k++){
	if(mask[the_step.to+neighbours[k]] & MASK)friends+=1;
      };
      
      friends=friends/27.0f; //smooth_bell(friends-9.0/30)*smooth_bell((friends-9.0)/30);

      //depth factor
      float f_depth = 0.1f;
      if(max_depth < 15){
	f_depth=1.0f-0.07f*depth[the_step.to]; //do not consider 
      }else{
	f_depth = 1.0;
      };                                              //depth too deep...
                                                      //it is meaningless.

      int x, y, z;
      getCoords(the_step.to, x, y, z);
      V3f cur((float)x,(float)y,(float)z);
      //distance; [1-0], 1 - closest, 0 - furtherst
      float distance = smooth_bell((center-cur).length()/50);  
      if(!use_scope)distance = 1.0;


      //magick formula from voxelbrain version 1.
      //      (*i).score = (1.0f-delta/1000.0f)*in_band*friends*f_depth*distance;//*(do_internals || (is_border(vol,dest))?1:0);

      
      delta = ((max_delta - delta)/max_delta);
      delta *= delta;
      delta *= delta;
      (*i).score = delta*in_band*f_depth*f_depth*distance;// (1.0f-delta/1000.0f)*in_band*friends*f_depth*distance;//*(do
      
      //let's experiment with penalties; and control them.
      //(*i).score = (1.0f-in_band);//
      if((*i).score > worst_score)worst_score=(*i).score;
      if((*i).score > max)max = (*i).score;
      if((*i).score < min)min = (*i).score;
    };

    //3. Well... seems like sorting _is_ importand;
    std::sort(steps.begin(), steps.end());
        //thr = steps[steps.size() * 0.9]

    int cnt = 0;

    //everyone below the limit pass. 
    float current_limit = max-(max-min)*0.1f;

    printf("Current limit is %f \n", current_limit);

    for(std::vector<step>::iterator i = steps.begin(); i != steps.end(); i++){
      cnt++;
      bool go = cnt > (steps.size()*0.9);
      if(steps.size() < 10)go=cnt > ((steps.size()*0.5));
      if(steps.size() == 1)go=true;
      if(steps.size() > 100)go=(bool)((*i).score > current_limit);
      int interesting = (go)?(i->to):(i->from);
      //now, use that interesting point and add it to be active,
      //if it is still not

      //in-scope means limit to a bounding box; so far no lexicon for bonuding
      //avaliable as the nesessity is questionable.
      if((!(mask[interesting] & BDR)) /* && in_scope(interesting) */){
	mask[interesting] |= BDR;
	res.push_back(interesting);
	if(go)undo_buffer.push_back(interesting);
      }; 

    };    

    //ok, we are done; clear everything and off to the next round. YAHOO!!!
    markers = res;
    steps.clear();
    printf("Markers %d\n", (int)markers.size());
  };

  if(undo_buffer.size() && undo_buffer.back())undo_buffer.push_back(0);

  //reseed();  
};

void FastVolume::propagate(int threshold, int dist, int max_depth, int times){
  // cur_gen++;
  int cur, cur_val;//, cursor_idx;
  std::vector<int> res;
  int cur_idx;  

  if(undo_buffer.size() && undo_buffer.back())undo_buffer.push_back(0);

  for(int iter = 0; iter < times; iter++){
    res.clear();
  //every point
    for(std::vector<int>::iterator i = markers.begin(); i != markers.end(); i++){
      cur = *i;
      //if(!GEN(mask[cur]))mask[cur]=cur_gen | MSK; //mark it as mask

      if(mask[cur] & TRU)continue;

      mask[cur] -= MASK & mask[cur];
      mask[cur] |= MSK;

     if(!in_scope(cur)){ //wait a bit
	mask[cur] |= BDR;
	res.push_back(cur);
     }else{ //in scope, can go

      //every neighbour
      for(int j = 0; j < 6; j++){
	cur_idx = cur + neighbours[j];
	cur_val = vol[cur_idx];
	if(MASK & mask[cur_idx]){
	  continue;
	  //mask[cur_idx] -= (BDR & (mask[cur_idx]);
	};
	//try lookahead first; proceed as normal if unsuccessful;
	if(!lookahead(this, res, cur, neighbours[j], dist, cur_gen)){
	  if((!((ZRO | TRU | MASK) & mask[cur_idx])) && (cur_val < threshold) && ((depth[cur_idx] < max_depth) || depth == 0)){
	    if(!(BDR & mask[cur_idx])){
	      mask[cur_idx] |= BDR;
	      res.push_back(cur_idx);
	      undo_buffer.push_back(cur_idx);
	    };
	  };
	};
      };
     }; //in scope
    };
    markers = res;
    printf("Markers %d\n", (int)markers.size());
  };
  //if(cur_gen > (GEN_MAX-3))downshift(MASK);

  if(undo_buffer.size() && undo_buffer.back())undo_buffer.push_back(0);

  //reseed();

};

/*
void FastVolume::downshift(int flags = MASK){
  for(int i = getOffset(1,1,1); i < getOffset(255,255,255); i++){
    if((flags & mask[i]) && (GEN(mask[i]) > 1))
      { //ok, we want to deal with this voxel- it is our type and can be downshifted
	mask[i] = / *flags* /((0xff-GEN_MAX) & mask[i]) + / *gen-1* /(GEN(mask[i])-1);
    
      };
  };
};
*/



