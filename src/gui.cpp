
#include "gui.h"
#include "native.h"
#include "surface.h"

#include <vector>
#include <string>
#include <AntTweakBar.h>

struct GuiContainer{
  GuiContainer(slices * _sl, GlPoints * _pnt);
  ~GuiContainer();

  void create(); // populate the container

  slices * sl; //2d crossection
  GlPoints * pnt; // collection of 3d points and volume

  TwBar * bar;

  typedef enum { BROWSING=0, ADD_SEEDS=1 } Modes;
  Modes mode;
  
  int level;
  int threshold;
  int generation;
  int amount;
  int iterations;
  int size;
  int scheme;
  int zoom;
  float coverage;
  int depth;
  int propagator_type; //what kind of propagation do we want
  int radius; //influence radius

  //2d options

  


  /* Helper functions */

  static void TW_CALL get_level(void * value, void * );
  static void TW_CALL set_level(const void * value, void * );
  static void TW_CALL load_file( void * );
  static void TW_CALL test_shape( void * );
  static void TW_CALL test_button( void * );
  static void TW_CALL save_file( void * );
  static void TW_CALL save_file_as( void * );
  static void TW_CALL load_mask( void * );
  static void TW_CALL save_mask( void * );
  static void TW_CALL load_file_truth( void * );
  static void TW_CALL grow_truth( void * );
  static void TW_CALL erode_truth( void * );
  static void TW_CALL apply_truth( void * ); //remove voxels around ground truth
  static void TW_CALL apply_mask( void * );
  static void TW_CALL reseed( void * );
  static void TW_CALL kill_seeds( void * );
  static void TW_CALL switch_crossections( void * );
  static void TW_CALL set_band( void * );
  static void TW_CALL step( void * );
  static void TW_CALL step_all( void * );
  static void TW_CALL remove_hanging_pieces( void * );
  static void TW_CALL undo( void * );
  static void TW_CALL get_size(void * value, void * );
  static void TW_CALL set_size(const void * value, void * );
  static void TW_CALL get_zoom(void * value, void * );
  static void TW_CALL set_zoom(const void * value, void * );
  static void TW_CALL get_mask(void * value, void * );
  static void TW_CALL set_mask(const void * value, void * );
  static void TW_CALL get_coverage(void * value, void * );
  static void TW_CALL set_coverage(const void * value, void * );
  static void TW_CALL get_scheme(void * value, void * );
  static void TW_CALL set_scheme(const void * value, void * );


};

/* global structure */

GuiContainer * the_gui = 0;


/* declarations for constructor */

/* Constructor */

GuiContainer::GuiContainer(slices * _sl, GlPoints * _pnt):
  sl(_sl), 
  pnt(_pnt), 
  level(3), 
  threshold(40), 
  generation(3), 
  amount(2), 
  iterations(2),
  size(100),
  scheme(0),
  zoom(1),
  coverage(0.25),
  depth(10),
  propagator_type(1),
  radius(15)
{
  bar = TwNewBar("Options");
};


void GuiContainer::create(){


  // Defining enums for different actions... 
  TwEnumVal ioEV[] = {
    {0, "Mask"},
    {1, "Volume"}
  };
  TwType ioType = TwDefineEnum("IoType", ioEV, 2);

  TwEnumVal modesEV[] = { 
    {0, "Browse"}, 
    {1, "Extend propagation front"},
    {2, "Enable propagation"},
    {3, "Inhibit propagation"},
    {4, "Add ground truth"},
    {5, "Clear all modifications"}};
  TwType modesType = TwDefineEnum("ModeType", modesEV, 6);

  TwEnumVal propagatorsEV[] = { 
    {0, "Threshold"}, 
    {1, "Similarity"}};
  TwType propagatorType = TwDefineEnum("PropagatorsType", propagatorsEV, 2);

  ///create type for colors... well; damnit? stupid.        
  std::vector<std::string> col = color_type();
#ifndef WIN32
  const int colors_num = col.size();
  TwEnumVal colorsEV[colors_num];
#else //if using msvc, have to mantain a schemes table of a fixed size.
  TwEnumVal colorsEV[100];	/// (msvc hack here) ///
//  col.erase(col.begin()+99, col.end());
#endif
  for(size_t i = 0 ; i < col.size(); i++){
    const char * ss = col[i].c_str();
    TwEnumVal a = {(int)i, ss};
    //  a.Label[0]=col[i].c_str()[0];
    colorsEV[i] = a;
  };
  TwType colorsType = TwDefineEnum("ColorType", colorsEV, (unsigned int)col.size());

  ///building bars is a different business..
  
  TwDefine(" GLOBAL help='Voxelbrain Voxel editor.' "); // Message added to the help bar.
  TwDefine("Options size='200 600'");
  pnt->tw_pnt = 1.0;
  TwAddButton(bar, "", load_file, NULL, "label='Load' key='l' group=File");
  TwAddButton(bar, "", save_file, NULL, "label='Save' group=File");
  TwAddButton(bar, "", save_file_as, NULL, "label='Save As' group=File");
  TwAddVarRW(bar, "IO Type", ioType, &pnt->loader.cur_plane, "group=File");
  TwAddVarRW(bar, "", TW_TYPE_DOUBLE, &pnt->tw_pnt, " label='Point size' group=Visualization min=0.2 max=4 step=0.01 keyIncr=d keyDecr=D help='Size of the display points in relation to optimal' ");
  TwAddVarRW(bar, "", TW_TYPE_FLOAT, &pnt->tw_transparency, " label='Transparency' group=Visualization min=0.03 max=1 step=0.01 keyIncr=d keyDecr=D help='Transparency' ");
  pnt->tw_pnt_smooth=true;
  TwAddVarRW(bar, "", TW_TYPE_BOOLCPP, &pnt->tw_pnt_smooth, " label='Smooth points' group=Visualization keyIncr=v keyDecr=V help='Size of the display points. ' ");
  TwAddVarRW(bar, "", TW_TYPE_INT32, &pnt->cur_level, " label='Voxel layer' group=Visualization min=0 max=14 step=1 help='Show underlying layers' ");

  //display cursor
  TwAddVarRW(bar, "", TW_TYPE_FLOAT, &pnt->cursor.x, " label='X' help='Cursor X'  group=Cursor");
  TwAddVarRW(bar, "", TW_TYPE_FLOAT, &pnt->cursor.y, " label='Y' help='Cursor Y' group=Cursor ");
  TwAddVarRW(bar, "", TW_TYPE_FLOAT, &pnt->cursor.z, " label='Z' group='Cursor' help='Cursor Z' ");
  TwAddVarRO(bar, "", TW_TYPE_FLOAT, &pnt->tw_mri_value, " label='Intensity' group=Cursor help='MRI data value at the calcualtor.' ");
    
  //allow to change
  //  TwAddVarCB(bar, "", TW_TYPE_FLOAT, GuiContainer::set_level, GuiContainer::get_level, NULL, " min=-1 max=150 step=1 label='Isovalue'");
  pnt->tw_cursor_hit = 3;
  TwAddVarRW(bar, "", TW_TYPE_FLOAT, &pnt->tw_cursor_hit, "  min=0 max=150 step=1 label='Depth (by intensity)' group=Cursor help='Point where the cursor considered to have hit the surface' ");
  pnt->tw_cursor_depth = 0;
  TwAddVarRW(bar, "", TW_TYPE_FLOAT, &pnt->tw_cursor_depth, " min=0 max=50 step=1 label='Depth (by distance)' group='Cursor' help='How deep cursor goes after an impact.' ");
  pnt->tw_mri_value = 0.0;
  //TwAddVarRW(bar, "", TW_TYPE_INT32, &pnt->tool, " min=0 max=1 step=1 label='Editing mode' help='0-nop, 1-mark seeds.' ");
  TwAddVarRW(bar, "Mode", modesType, &pnt->tool, " group='Interaction'");
  TwAddVarRW(bar, "", TW_TYPE_INT32, &pnt->tool_size, " min=1 max=60 step=1 label='Modification area size' group='Interaction' help='Point size' ");
  TwAddVarRW(bar, "", TW_TYPE_INT32, &threshold, " min=3 max=1000 step=1 label='Prop. threshold' group='Parameters' help='what possible thresholds are avaliable' ");
  TwAddVarRW(bar, "", TW_TYPE_INT32, &amount, " min=1 max=10 step=1 label='Lookahead' group='Parameters' help='Skip this many voxels in search for suitable areas.' ");
  TwAddVarRW(bar, "", TW_TYPE_INT32, &depth, " min=0 max=256 step=1 label='Depth limit' group='Parameters' help='Go no deeper than this value. 0 means no limit' ");
  TwAddVarRW(bar, "", TW_TYPE_INT32, &iterations, " min=1 max=400 step=10 label='Step size' help='Iterations per button press' group='Parameters'");

  TwDefine("Options/Parameters group=Propagation");

  TwAddVarRW(bar, "Propagation method", propagatorType, &propagator_type, "group='Propagation'");
  TwAddVarRW(bar, "", TW_TYPE_BOOLCPP, &(pnt->vol.use_scope), " label='Prefer point of view' group='Propagation' ");
  TwAddButton(bar, "", set_band, NULL, " label='Autodetect parameters' key='b' group='Propagation'");
  TwAddButton(bar, "", step, NULL, " label='Process (a step)' key='g' group='Propagation'");
  TwAddButton(bar, "", step_all, NULL, " label='Process (all)' key='g' group='Propagation'");
  TwAddButton(bar, "", remove_hanging_pieces, NULL, " label='Remove unconnected voxels' group='ROI' ");
  TwAddButton(bar, "", undo, NULL, " label='Undo' key='z' group='Propagation'");
  TwAddButton(bar, "", reseed, NULL, " label='Init propagation front' group='ROI' ");
  TwAddButton(bar, "", kill_seeds, NULL, " label='Clear propagation front' key='n' group='ROI'");
  TwAddButton(bar, "", apply_mask, NULL, " label='Toggle Preview' group='ROI' ");
  TwAddButton(bar, "", load_file_truth, NULL, " label='Load' key='t' group='Ground truth' ");
  TwAddVarRW(bar, "", TW_TYPE_INT32, &radius, " min=1 max=60 step=1 label='Modification radius' group='Ground truth' help='The size of the area where we want to grow the truth' ");
  TwAddButton(bar, "", grow_truth, NULL, " label='Grow' group='Ground truth'");
  TwAddButton(bar, "", erode_truth, NULL, " label='Erode' key='e' group='Ground truth'");
  TwAddButton(bar, "", apply_truth, NULL, " label='Process' group='Ground truth'");

  ///and now the slice control:
  sl->tiles_coverage(0.25, 1.0);
    

  TwAddVarCB(bar, "", TW_TYPE_INT32, GuiContainer::set_size, GuiContainer::get_size, NULL, " min=20 max=300 step=5 label='Tile size' group='2D'");
  TwAddVarCB(bar, "", TW_TYPE_INT32, GuiContainer::set_zoom, GuiContainer::get_zoom, NULL, " min=1 max=5 step=1 label='Zoom' group='2D'");
  TwAddVarCB(bar, "", TW_TYPE_BOOLCPP, GuiContainer::set_mask, GuiContainer::get_mask, NULL, " label='Toggle 2D Mask' key='m' group='2D'");
  TwAddVarCB(bar, "", TW_TYPE_FLOAT, GuiContainer::set_coverage, GuiContainer::get_coverage, NULL, " min=0.25 max=1.0 step=0.03 label='Coverage' group='2D'");
  TwAddVarCB(bar, "Color scheme", colorsType, GuiContainer::set_scheme, GuiContainer::get_scheme, NULL, " group=Visualization min=0 max=4 step=1 label='Color scheme' ");
  TwAddButton(bar, "", GuiContainer::switch_crossections, NULL, "label='Switch planes' key='c' group=2D");

  TwAddButton(bar, "", test_shape, NULL, "group=Testing label='Shape'");
  TwAddButton(bar, "", test_button, NULL, "group=Testing label='Action'");


};

/* Destructor */
GuiContainer::~GuiContainer(){
};

/* Helper functions */

void TW_CALL GuiContainer::get_level(void * value, void * UserData){
  //printf("getting level %f\n", (float)level);
  (*((float *)value))=(float)(the_gui->level);
};
void TW_CALL GuiContainer::set_level(const void * value, void * UserData){
  the_gui->level = (int)(*((float *)value));
  printf("setting level %f\n", (double)the_gui->level);
  the_gui->pnt->set_level((float)the_gui->level);
};

void TW_CALL GuiContainer::load_file( void * UserData){
  printf("Trying to load a file.\n");
  std::string in = getFile();
  if(in.length() > 0){
    printf("indeed, got %s\n", in.c_str());
    set_current_file(in);
    the_gui->pnt->load(in.c_str());
    the_gui->pnt->find_surface();
  };

  //TODO: clear the surface.
  ///get_active_surfaces()->clear();
  the_gui->sl->update();
  the_gui->pnt->update(); //and make sure all is shown up.....

};

void TW_CALL GuiContainer::test_shape( void * UserData){
  printf("Coming up with a surface.\n");
  int adj = 0;
  int gmwm = 0;
  the_gui->pnt->vol.reset();
  for(int i = 0; i < 255*255*255; i++)the_gui->pnt->vol.vol[i]=0;
  for(int x = -60; x< 60; x++)
    for(int y = -60; y< 60; y++)
      for(int z = -60; z< 60; z++){
	int dist = (int)(60.0-sqrtf((float)(x*x+y*y+z*z)));
	if(dist < 0 && dist >= -5)adj++; 
	if(dist < 0 || !((x+50)%80))dist=0;
	if(dist>0){
	  //dist+=50;
	  dist *= 5;
	  gmwm++;
	};
	int offset = the_gui->pnt->vol.getOffset(x+128,y+128, z+128);
	the_gui->pnt->vol.vol[offset] = dist;
      };
  the_gui->pnt->find_surface();
  printf("gmwm: %d, adj: %d\n", gmwm, adj);
};


//remove half the brain
int cutted = 10;

void TW_CALL GuiContainer::test_button( void * UserData){
  printf("Coming up with a surface.\n");
  int adj = 0;
  int gmwm = 0;
  //the_gui->pnt->vol.reset();
  for(int i = 0; i < 255*255*cutted; i++){
    the_gui->pnt->vol.vol[i] = 0;
  };
  the_gui->pnt->find_surface();
  printf("gmwm: %d, adj: %d\n", gmwm, adj);
  cutted+=3;
};

#include "v3.h"
#include "stdio.h"

int dead = 0;
bool half = false; //incude > 50% voxels
bool inside = true;
bool tru = false;

bool refine(V3f & v0, V3f & v1, V3f & v2, GlPoints * pnt, V3f n){
  V3f v[3] = {v0, v1, v2};
  bool good; //mark what voxels are inside as good

  for(int i = 0; i < 3; i++){ ///each vertex
      
    /*     V3f vec((vtx&1)?ceil(v[i].x):floor(v[i].x), //check neighbouring voxels
	      (vtx&2)?ceil(v[i].y):floor(v[i].y),
	      (vtx&4)?ceil(v[i].z):floor(v[i].z));
    */
    
    V3f vec(v[i].x, v[i].y, v[i].z);
      V3f dir = vec-v[i];
      good = (dir.dot(n)<0);
      if(!half)good = true;

      int cur = pnt->vol.getOffset((int)floor(v[i].x), (int)floor(v[i].y), (int)floor(v[i].z));
      if(!(pnt->vol.mask[cur] & TRU)){
	pnt->vol.mask[cur] |= TRU;
	if(good)pnt->vol.mask[cur] |= (tru?TRU:MSK);
	if((pnt->vol.mask[cur] & MASK) 
	   && (pnt->vol.vol[cur] > 35))dead++; //count intersecting pixels
      };
    };
  
  //check if the refinement is needed, i.e. tris are too big ( > 0.5 voxels )
  if((v0-v1).length2() > 0.2 ||
     (v1-v2).length2() > 0.2 ||
     (v2-v0).length2() > 0.2){
    V3f o0 = (v0+v1)/2;
    V3f o1 = (v1+v2)/2;
    V3f o2 = (v2+v0)/2;
   
    //if triangles are too big, render smaller ones...
    refine( v0, o0, o2, pnt, n);
    refine( o0, v1, o1, pnt, n);
    refine( o1, v2, o2, pnt, n);
    refine( o0, o1, o2, pnt, n);
  }; 
  return true;
};

void read_voxels(std::string in, GlPoints * pnt, bool _half = false, bool _tru = false, bool _fill = true){
  V3f center(0,0,0);
  int N = 0;

  half = _half;
  printf("Including %s voxles\n", half?">50% occupancy":"all");

  tru = _tru; //using true voxels.

  dead = 0;
  FILE * f = fopen(in.c_str(), "ro");
  std::vector<V3f> stor;

  if(f == NULL){
    printf("Cannot open file;\n");
    return;
  };

  char buf[1000];
  fgets( buf, 1000, f); //first line, don't care
  int points;
  int tris;
  fscanf(f, "%d %d\n", &points, &tris);
  printf("trying to get %d points", points);
  //reading lines
  for(int i = 0; i < points; i++){
    V3f in;
    int dummy;
    fscanf(f, "%f  %f  %f  %d\n", &in.x, &in.y, &in.z, &dummy);
    in = V3f(-in.x, -in.z, +in.y);
    //  in+=V3f(124.34, 100.59, 138.53);
    //in+=V3f(4.65, 27.4, -10.53);

    FastVolume::t_tr tr = the_gui->pnt->vol.tr;

    //  in+=V3f(128.0+tr.c_r, 128.0+tr.c_s, 128.0-tr.c_a);
    //  in+=V3f(-tr.c_r, -tr.c_s, tr.c_a);

    in+=V3f(128, 128, 128);


    stor.push_back(in);
    center+=in;
    N++;
  };

  //ok, now all the correct points in tri.
  //read tris now
  V3f m[3];
  //  std::vector<int> tristor; //triangle storage
  for(int i = 0; i < tris; i++){
    int a, b, c, zero;
    fscanf(f, "%d %d %d %d\n", &a, &b, &c, &zero);
    m[0]=stor[a];
    m[1]=stor[b];
    m[2]=stor[c];
  
    V3f n; n.cross(m[1]-m[0], m[2]-m[0]);
    n /= -n.length(); //normal - outside

    refine(m[0], m[1], m[2], pnt,  n);
  };
 
  fclose(f);
  printf("stor size is:%d\n", (int)stor.size());
  printf("%d pixels false positive.\n", dead);

  if(_fill){
  printf("Trying to fill it\n");
   center /= N; //this is the average;
   printf("The seed is %f %f %f; %d in total;  so what? \n", center.x, center.y, center.z, N);
   int cur = pnt->vol.getOffset((int)center.x, (int)center.y, (int)center.z);
   pnt->vol.mask[cur] |= BDR;
   pnt->vol.markers.push_back(cur);
  
  pnt->vol.propagate(1000, 0, 1000, 400); //propagate everywhere from centerpoint  

  //remove truth mask as not propagating anymore
  for(int i = 0; i < 256*256*256; i++){
    pnt->vol.mask[i] -= (pnt->vol.mask[i] & TRU);
  };
  };
  pnt->vol.updated = true;
  pnt->update(); //and make sure all is shown up.....

  
  
};

//add depth close to the sides of the region; should be nice 
int more_depth(int x, int y, int z, V3f w, int r){
  V3f cur(((float)x-w.x)/(float)r,
	  ((float)y-w.y)/(float)r,
	  ((float)z-w.z)/(float)r);

  return (int)(cur.length2()*3);
};

void do_grow_truth(FastVolume & v, V3f where, int radius){
  //we are going to push undo stuff.

  for(int i = 0; i <3; i++){
    radius = (where[i]+radius > 255)?255-(int)(where[i]):radius;
    radius = (where[i]-radius < 1)?((int)where[i]-1):radius;
    if(radius < 0)radius = 0;
  };
  for(int x = (int)(where.x-radius); x < (int)(where.x+radius); x++)
    for(int y = (int)(where.y-radius); y < (int)(where.y+radius); y++)
      for(int z = (int)(where.z-radius); z < (int)(where.z+radius); z++){
	int offset = v.getOffset(x,y,z);
	if(!(v.mask[offset] & (TRU | ZRO | MASK ) )){
	  //checking neighbours
	  for(int nbr = 0; nbr < 6; nbr++){
	    if(v.mask[offset+v.neighbours[nbr]] & TRU)
	      v.mask[offset] |= AUX;
	  };
	};
      };

  for(int x = (int)(where.x-radius); x < (int)(where.x+radius); x++)
    for(int y = (int)(where.y-radius); y < (int)(where.y+radius); y++)
      for(int z = (int)(where.z-radius); z < (int)(where.z+radius); z++){
	int offset = v.getOffset(x,y,z);
	if(v.mask[offset] & AUX){
	  v.mask[offset] -= AUX;
	  v.record_operation(offset, v.mask[offset] | TRU);
 	};
      };

	v.record_done();
};

void do_erode_truth(FastVolume & v, V3f where, int radius){

  V3f up(1,1,1);
  V3f o(0,0,0);
  V3f cur;
  int cx, cy, cz;

  /// bounding box
  int maxp[3] = {-1000,-1000,-1000};
  int minp[3] = { 1000, 1000, 1000};

  ///use first 3 seeds both for plane definition and for dialog box
  V3f pnt[3];
  if(v.markers.size() < 3)return; //the thing is not set yet
  for(int i = 0; i < 3; i++){
    v.getCoords( v.markers[i], cx, cy, cz); pnt[i]=V3f(cx,cy,cz);
    if(cx < minp[0])minp[0]=cx; if(cy < minp[1])minp[1]=cy; if(cz < minp[2])minp[2]=cz;
    if(cx > maxp[0])maxp[0]=cx; if(cy > maxp[1])maxp[1]=cy; if(cz > maxp[2])maxp[2]=cz;
  };
  
  up.cross(pnt[1]-pnt[0], pnt[2]-pnt[0]);
  up /= up.length();
  ///make sure it is directed up
  if((pnt[0]-V3f(128.0f, 128.0f, 128.0f)).dot(up) < 0)up *= -1.0; 

  printf("Got up vector as %f,%f,%f", up.x, up.y, up.z);

  ///remove any plane points, if required:
  v.plane.clear();

	//unmarking

  unmark(*get_active_surfaces(), the_gui->pnt->cursor, (float)the_gui->radius);

  for(int i = 0; i <3; i++){
    radius = (where[i]+radius > 255)?255-(int)where[i]:radius;
    radius = (where[i]-radius < 1)?((int)where[i]-1):radius;
    if(radius < 0)radius = 0;
  };

  float max_depth = -1000;
  float min_depth = 1000;
  float threshold; //defined after first pass;
  float marking_threshold; //threshold for the next step

  printf("\n[%d %d %d]-[%d %d %d]\n", minp[0], minp[1], minp[2], 
	                            maxp[0], maxp[1], maxp[2]);

  for(int pass = 0; pass <= 1; pass++){
    //on the last pass define the threshold
    if(pass == 1){
      threshold = max_depth - (max_depth-min_depth)/40.0f;
      marking_threshold = max_depth - (max_depth-min_depth)/10.0f;
    };

    //around points with 3 margin
    for(int x = minp[0]-3; x <= maxp[0]+3; x++)
      for(int y = minp[1]-3; y <= maxp[1]+3; y++)
	for(int z = minp[2]-3; z <= maxp[2]+3; z++){
	  int offset = v.getOffset(x,y,z);
	  if(!pass)v.mask[offset] -= (HIG & v.mask[offset]);
	  if(v.mask[offset] & TRU){
	    //offset
	    v.getCoords(offset, cx, cy, cz); cur = V3f( cx, cy, cz);	  
	    float cur_depth = up.dot(cur);
	    if(cur_depth > max_depth)max_depth = cur_depth;
	    if(cur_depth < min_depth)min_depth = cur_depth;
	    if( pass ){
	      if( threshold < cur_depth )
		v.record_operation(offset, 
				   v.mask[offset] - ((HIG | TRU) & v.mask[offset]));
	      if(cur_depth > marking_threshold &&
		 cur_depth < threshold){
		v.mask[offset] |= HIG;
	      ///and add it to 3d plane as well
		v.plane.push_back(offset);
	      };
	    };
	  };	  
	};

  };
  v.record_done();
};

void TW_CALL GuiContainer::grow_truth( void * UserData){
  do_grow_truth(the_gui->pnt->vol, the_gui->pnt->cursor, the_gui->radius);
//unmarking
  
    unmark(*get_active_surfaces(), the_gui->pnt->cursor, (float)the_gui->radius);
  the_gui->pnt->vol.updated = true;
  the_gui->sl->update();
  the_gui->pnt->update(); //and make sure all is shown up.....
};

void TW_CALL GuiContainer::erode_truth( void * UserData){
  do_erode_truth(the_gui->pnt->vol, the_gui->pnt->cursor, the_gui->radius);
  the_gui->pnt->vol.updated = true;
  the_gui->pnt->update(); //and make sure all is shown up.....
};

void TW_CALL GuiContainer::load_file_truth( void * UserData)
{
  printf("Trying to load a file.\n");
  std::string in = getFile();

  if(in.length() > 0){
    printf("indeed, got %s\n", in.c_str());
    Surface & it = *get_active_surfaces();
    if(!read_surface_binary(it, in))return;

    printf("Obtained %d vertices, and %d triangles, thanks for asking.\n",
	   (int)it.n.size(), (int)it.tri.size() );
    
    RenderingTraits t = {0, false, true, true};
    analyze_surface(it, *the_gui->pnt);
    rasterize_surface(it, *the_gui->pnt, t);

  };  
  the_gui->sl->update();

};

void do_all_work(GlPoints * pnt){
  pnt->vol.markers.clear();

  for(int i = pnt->vol.getOffset(1,1,1); i <= pnt->vol.getOffset(255,255,255); i++)
    {
      if((pnt->vol.depth[i] < 10) && !(pnt->vol.mask[i] & (TRU | ZRO | MASK))){
	//if there is a truth nearby, we can mark our pos.
	for(int nbr = 0; nbr < 6; nbr++){
	  int cur_nbr = i+pnt->vol.neighbours[nbr];
	  if(TRU & pnt->vol.mask[cur_nbr]){
	    //ok, we are interested; there is truth somewhere here;
	    pnt->vol.markers.push_back(i);
	    pnt->vol.mask[i] |= MASK;
	    break;
	  };
	};
      };
    };

  //ok, got interesting points; now propagating.
  pnt->vol.propagate(1000, 8, 10, 4);
  pnt->vol.updated = true;
  pnt->update(); //and make sure all is shown up.....

}

void TW_CALL GuiContainer::apply_truth( void * UserData)
{
  printf("Removing everything close to ground truth.\n");
  do_all_work(the_gui->pnt);
};

void TW_CALL GuiContainer::switch_crossections( void * UserData){
  printf("Trying to switch crossections.\n");
  the_gui->sl->switch_crossections();
};


void TW_CALL GuiContainer::save_file( void * UserData){
  printf("Trying to save a default file.\n");
    the_gui->pnt->save(NULL);
};


void TW_CALL GuiContainer::save_file_as( void * UserData){
  printf("Trying to save a file.\n");
  std::string in = putFile();
  if(in.length() > 0){
    printf("indeed, got %s to save in.\n", in.c_str());
    the_gui->pnt->save(in.c_str());
  };
};


//saving and loading masks

void TW_CALL GuiContainer::save_mask( void * UserData){
  printf("UnImplemented.\n");
  std::string in = putFile();
  if(in.length() > 0){
    //  printf("indeed, got %s to save in.\n", in.c_str());
    //the_gui->pnt->loader.save_mask(in, the_gui->pnt->vol.mask);
  };
};


void TW_CALL GuiContainer::load_mask( void * UserData){
  printf("UnImplemented.\n");
  std::string in = getFile();
  if(in.length() > 0){
    printf("indeed, got %s to load.\n", in.c_str());
    TwDefine((std::string("") + " GLOBAL help='" + in + "' ").c_str()); // Change global help to the file being edited 
    
    // the_gui->pnt->loader.load_mask(in, the_gui->pnt->vol.mask);
  };

  the_gui->pnt->update(); //and make sure all is shown up.....

};


void TW_CALL GuiContainer::reseed( void * UserData){
  printf("Reseed\n");
  the_gui->pnt->vol.reseed();
  the_gui->pnt->vol.updated = true;
  the_gui->pnt->update(); //and make sure all is shown up.....

};

void TW_CALL GuiContainer::kill_seeds( void * UserData){
  printf("Kill seeds\n");
  for(int i = 0; i < 256*256*256; i++){
    if(the_gui->pnt->vol.mask[i] & BDR){
      the_gui->pnt->vol.mask[i] -= (the_gui->pnt->vol.mask[i] & (MASK | HIG));
    };
  };

  the_gui->pnt->vol.markers.clear();
  the_gui->pnt->vol.plane.clear();

  the_gui->pnt->vol.updated = true;
  the_gui->pnt->update(); //and make sure all is shown up.....

};


void TW_CALL GuiContainer::apply_mask( void * UserData){
  printf("Apply mask\n");
  the_gui->pnt->apply();
  the_gui->pnt->vol.updated = true;
  the_gui->pnt->update(); //and make sure all is shown up.....

};

void TW_CALL GuiContainer::set_band( void * UserData){
  the_gui->pnt->vol.set_band();
};

void TW_CALL GuiContainer::remove_hanging_pieces( void * UserData){
  the_gui->pnt->vol.scan_for_disconnected_regions();
  the_gui->pnt->vol.updated = true;
  the_gui->pnt->update(); //and make sure all is shown up.....  
};

void TW_CALL GuiContainer::step( void * UserData){
  // TODO act upon CASE
  switch(the_gui->propagator_type){
  case 0: // jump
    the_gui->pnt->vol.propagate(the_gui->threshold, the_gui->amount, the_gui->depth, the_gui->iterations);
    break;
  case 1: //similarity
    the_gui->pnt->vol.propagate_spread(the_gui->threshold, the_gui->amount, the_gui->depth, the_gui->iterations);
  };
  the_gui->pnt->vol.updated = true;
  the_gui->pnt->update(); //and make sure all is shown up.....
};

///
/// TODO
///
///
void TW_CALL GuiContainer::step_all( void * UserData){
  switch(the_gui->propagator_type){
  case 0: // jump
    the_gui->pnt->vol.propagate(the_gui->threshold, the_gui->amount, the_gui->depth, 200);
    break;
  case 1: //similarity
    the_gui->pnt->vol.propagate_spread(the_gui->threshold, the_gui->amount, the_gui->depth, 200);
  };
  the_gui->pnt->vol.updated = true;
  the_gui->pnt->update(); //and make sure all is shown up.....
  
};


void TW_CALL GuiContainer::undo( void *){
  the_gui->pnt->vol.undo();
  the_gui->pnt->vol.updated = true;
  the_gui->pnt->update(); //and make sure all is shown up.....
};

//slices
void TW_CALL GuiContainer::get_size(void * value, void * UserData){
  (*((int *)value))=(int)(the_gui->size);
};

void TW_CALL GuiContainer::set_size(const void * value, void * UserData){
  the_gui->size = *((int *)value);
  printf("setting level %d\n", the_gui->size);
  the_gui->sl->resize_tile((int)(the_gui->size), (int)(the_gui->size));
};

//zoom
void TW_CALL GuiContainer::get_zoom(void * value, void * UserData){
  (*((int *)value))=the_gui->sl->zoom;
};

void TW_CALL GuiContainer::set_zoom(const void * value, void * UserData){
  the_gui->sl->zoom = (*((int *)value));
  the_gui->sl->update();
};

//mask
void TW_CALL GuiContainer::get_mask(void * value, void * UserData){
  (*((bool *)value))=the_gui->sl->show_mask;
};

void TW_CALL GuiContainer::set_mask(const void * value, void * UserData){
  the_gui->sl->show_mask = (*((bool *)value));
  the_gui->sl->update();
};

//coverage
void TW_CALL GuiContainer::get_coverage(void * value, void * UserData){
  (*((float *)value))=the_gui->coverage;
};

void TW_CALL GuiContainer::set_coverage(const void * value, void * UserData){
  the_gui->coverage = (*((float *)value));
  the_gui->sl->tiles_coverage(the_gui->coverage, 1.0);
  the_gui->sl->update();
  
};

void TW_CALL GuiContainer::get_scheme(void * value, void * UserData){
  (*((int *)value))=(int)(the_gui->scheme);
};

void TW_CALL GuiContainer::set_scheme(const void * value, void * UserData){
  the_gui->scheme = *((int *)value);
  printf("setting scheme %d\n", the_gui->scheme);
  the_gui->sl->scheme = the_gui->scheme;
  the_gui->pnt->scheme = the_gui->scheme; //and for 3d the same sceme
  the_gui->pnt->set_scheme(the_gui->scheme);
  the_gui->sl->update();
};

/* interface */

void gui_start(slices * sl, GlPoints * pnt){
  if( !TwInit(TW_OPENGL, NULL) )
    {
      // A fatal error occured	
      fprintf(stderr, "AntTweakBar initialization failed: %s\n", TwGetLastError());
      //    glfwTerminate();
      //return 1;
      return;
    }
  
  the_gui = new GuiContainer(sl, pnt);
  the_gui->create(); //populate the thing.
};


void gui_draw(){
  TwDraw();
};

void gui_resize(int x, int y){
  TwWindowSize(x, y);
};

void gui_stop(){
  if(the_gui)delete the_gui;
  TwTerminate();
};

void set_current_file(std::string in){
    TwDefine((std::string("") + " GLOBAL help='Now editing " + in + "' ").c_str()); // Change global help to the file being edited 
  
};











