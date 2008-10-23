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

const int FastVolume::corners[8] = {
  0,  dx,      dy,      dx+dy, 
  dz, dx + dz, dy + dz, dx+dy + dz };

/***
UNDO, version 3.
Simply record modification verbatim;
Conflict resolution - if undo info does not match - don't apply.

***/
struct UndoOp{
	int pos;
	int before;
	int after;
};

//marking the end of a single operation
bool is_action_marker(UndoOp & in){
	return in.pos == 0;
};

UndoOp action_marker(){
	UndoOp cur = {0,0,0};
	return cur;
};

std::vector<UndoOp> undo_info;

/// before - nneded to check what is being undone, and perform some custom operations;
/// ignores info with identical undo info; 
void FastVolume::record_operation(int pos, unsigned char before, unsigned char after){
	if(before == after)return;
	UndoOp cur = {pos, before, after};
	undo_info.push_back(cur);
}; //call if you want to remember 

void FastVolume::record_operation(int pos, unsigned char after){
	record_operation(pos, mask[pos], after);
	mask[pos] = after;
};

void FastVolume::record_done(){
	undo_info.push_back(action_marker());
};

//helper function
inline bool erase_element(std::vector<int> & where, int what){
  for(std::vector<int>::iterator i = where.begin(); i != where.end(); i++){
    if(what == *i){
      where.erase( i);
      return true;
    };
  };

  return false;
};


void FastVolume::undo(){
	if(undo_info.size() == 0)return;
	if(is_action_marker(undo_info.back()))undo_info.pop_back(); //remove marker, if any;

	while(undo_info.size() != 0){
	   UndoOp cur = undo_info.back(); //check back 
	   if(is_action_marker(cur)) return; //return if marker;
	   undo_info.pop_back();             //remove current action from stack
	   if(mask[cur.pos] == cur.after){
		   if((cur.after & BDR) && !(cur.before & BDR))erase_element(markers, cur.pos);
		   if(!(cur.after & BDR) && (cur.before & BDR)){ //if there was border and now not - make as it was before;
				erase_element(markers, cur.pos);
				markers.push_back(cur.pos);
		   };
		   mask[cur.pos] = cur.before; //undo, if matches
		//TODO: process markers properly
	   };
	};
};
 
void FastVolume::undo_reset(){
	undo_info.clear();
};

std::vector<int> undo_buffer;

FastVolume::FastVolume()
{
  vol = new t_vox[n_voxels]; 
  mask = new unsigned char [n_voxels];
  depth = new unsigned char [n_voxels];
  
  use_scope = false;

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
  plane.clear(); 
  cur_gen=1;
  undo_reset();
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
	  ///highlight
	  if(mask[offset] & HIG)buf[pos*3]=230;
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
	    record_operation(c, BDR | cur_gen);
	    markers.push_back(c);
	  };
	  break;
	case 2: //re-prime
	  if(mask[c] & MSK){
	    if(!(BDR & mask[c])){   //if not mask already
	      record_operation(c, BDR | cur_gen);
	      markers.push_back(c);
	    };
	    //ok, that was border
	    break;
	  };
	  break;
	case 3: //de-prime
	  if(mask[c] & BDR){
	    record_operation(c, mask[c] - (mask[c] & BDR)); 
	    for(unsigned int k = 0; k < markers.size(); k++)
	      if(markers[k] == c)markers[k] = 500; 
	    //set it to some arbitrary out of brain point
	  };
	  break;
	case 4: //add truth
	  record_operation(c, mask[c] | TRU); break;
	case 5: //clean everything
//	  mask[c] -= ((MASK | TRU) & mask[c]); break;
		record_operation(c, mask[c] - ((MASK | TRU) & mask[c])); break;
	
	};
	  };
     record_done(); // ok;

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
	    in->record_operation(cur, BDR | (GEN(in->mask[cur])));
	  else
	    in->record_operation(cur, BDR | cur_gen);

	  res.push_back(cur);
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
		in->record_operation(cur, in->mask[cur] | BDR);
	    res.push_back(cur);
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

//Well-writen first;
float FastVolume::Sample(float x_in, float y_in, float z_in){
  // Least possible corner.
  float ox = floorf(x_in);
  float oy = floorf(y_in);
  float oz = floorf(z_in);

  // Offset of that corner.
  int offset = getOffset((int)ox, (int)oy, (int)oz);

  // Coordinates inside the cell; [1 0]
  float x = x_in - ox;
  float y = y_in - oy;
  float z = z_in - oz;

  // 1-x coordinates;
  float X = 1-x;
  float Y = 1-y;
  float Z = 1-z;
  
  return 
    Z*(vol[offset+corners[0]]*X*Y+
       vol[offset+corners[1]]*x*Y+
       vol[offset+corners[2]]*X*y+
       vol[offset+corners[3]]*x*y)+
    z*(vol[offset+corners[4]]*X*Y+
       vol[offset+corners[5]]*x*Y+
       vol[offset+corners[6]]*X*y+
       vol[offset+corners[7]]*x*y);
};

void FastVolume::propagate_spread(int threshold, int dist, int max_depth, int times){


  // cur_gen++;
  int cur, cur_val;//, cursor_idx;
  std::vector<int> res;
  int cur_idx;  

  std::vector<step> steps;



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

	  unsigned char was = mask[cur]; 
      mask[cur] -= MASK & mask[cur];
      mask[cur] |= MSK;
	  record_operation(cur, was, mask[cur]);
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
	record_operation(interesting, mask[interesting] | BDR);
	res.push_back(interesting);
	}; 

    };    

    //ok, we are done; clear everything and off to the next round. YAHOO!!!
    markers = res;
    steps.clear();
    printf("Markers %d\n", (int)markers.size());
  };

  record_done();

  //reseed();  
};


void FastVolume::propagate(int threshold, int dist, int max_depth, int times){

	// cur_gen++;
  int cur, cur_val;//, cursor_idx;
  std::vector<int> res;
  int cur_idx;  

  for(int iter = 0; iter < times; iter++){
    res.clear();
  //every point
    for(std::vector<int>::iterator i = markers.begin(); i != markers.end(); i++){
      cur = *i;
      //if(!GEN(mask[cur]))mask[cur]=cur_gen | MSK; //mark it as mask

      if(mask[cur] & TRU)continue;

      record_operation(cur, (mask[cur] - (MASK & mask[cur])) | MSK);

     if(!in_scope(cur)){ //wait a bit
	record_operation(cur, mask[cur] | BDR);
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
	      record_operation(cur_idx, mask[cur_idx] | BDR);
	      res.push_back(cur_idx);
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


  //reseed();
	record_done();

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



void FastVolume::Set(int x, int y, int z, short data){
  vol[getOffset(x,y,z)] = data;
};

short FastVolume::Get( int x, int y, int z) const{
  return vol[getOffset(x ,y, z)];
};


