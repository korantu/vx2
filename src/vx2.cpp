#define GLFW_DLL

#include "glfw.h"
#include "gui.h"
#include "v3.h"
#include <stdio.h>
#include "gl_wrapper.h"
#include "gl_points.h"
#include "v3tools.h"
#include "slices.h"
#include "misc.h"
#include "color.h"


#ifdef LINUX
/// artificially adding a dummy local constant to force this thing compile;
/// just to see if it works;
/// needed for GCC4.1 
int  __stack_chk_fail_local;
#endif

///mouse button
///	Callback function called by GLFW when a mouse button is clicked
struct main_module : public gl_wrapper_reciever {

  bool render_required;

  slices crossection;


  GlPoints volume;

  float zoomf;
  

  main_module():crossection(& volume){
    render_required = true;
    zoomf = 1.0;
};

  struct t_proj {
    double matrix[16];
    
    V3f ex;
    V3f ey;
    V3f ez;

    t_proj() : ex(1,0,0), ey(0,1,0), ez(0,0,1) {
      double _matrix[] = {1.0, 0.0, 0.0, 0.0,
			  0.0, 1.0, 0.0, 0.0,
			  0.0, 0.0, 1.0, 0.0,
			  0.0, 0.0, 0.0, 1.0};
      for(int i = 0; i < 16; i++)matrix[i]=_matrix[i];
    };

    V3f z(){return V3f(ex.z, ey.z, ez.z);};

    /// first argument - rotation over x axis;
    /// second - over y axis
    /// which means they are inverted for mouse navigation.
    void rot(float x, float y){
      ex = rot_x(ex,x); ey = rot_x(ey,x); ez = rot_x(ez,x);
      ex = rot_y(ex,y); ey = rot_y(ey,y); ez = rot_y(ez,y);
      //printf("%f\n",ex.x);;
      ortoNormalize(ex,ey,ez);
    };

    void loadmatrix(){
      matrix[0]=ex.x; matrix[1]=ex.y; matrix[2]=ex.z;
      matrix[4]=ey.x; matrix[5]=ey.y; matrix[6]=ey.z;
      matrix[8]=ez.x; matrix[9]=ez.y; matrix[10]=ez.z;
      //glMatrixMode(GL_MODELVIEW);
      glLoadMatrixd(matrix);
    };
  } proj;


  bool load(char * in){ 
    render_required = true;
    return volume.load("brainmask.mgh"); 
  };
  
  void do_render(){
  };

  void do_resize(){
	  ///sanity check:
		if(st.width < 10 || st.width > 2000)return;
		if(st.height < 10 || st.height > 2000)return;
    render_required = true;
    printf("resz: w:%d; h:%d\n", st.width, st.height);
    crossection.resize_screen(st.width, st.height);
    //  crossection.tiles_coverage(0.25, 1.0);
    crossection.update(volume.vol, volume.cursor);//, V3f(1,0,0), V3f(0,1,0), V3f(0,0,1));
    
  };
  void do_key(){
    if(st.k >= 49 && st.k <= 54){
      printf("Trying to do a key... %d", st.k);
      volume.tool = (st.k - 49);
    };
  };

  void do_mouse(){
    float speed = 4;
    if(st.m_b==2){ //change view
      ///trying to pick crossection
      V3f res;
      //  crossection.tiles_coverage(0.25, 1.0);
      //      crossection.update(volume.vol, volume.cursor, V3f(1,0,0), V3f(0,1,0), V3f(0,0,1)),
      bool picked = crossection.pick(st.x, st.y, res);
      if(picked){
	crossection.display_center = res;
	printf("Trying to set the window to %d %d %d !!!\n", (int)volume.cursor.x, 
	       (int)volume.cursor.y, 
	       (int)volume.cursor.z);
	volume.set_cursor(res);
	printf("Picked...%f %f %f\n", res.x, res.y, res.z);
	crossection.update();
      }else{
	float div = (st.width<st.height)?((float)st.width):((float)st.height);
	proj.rot(speed*(float)st.dy/div,speed*(float)st.dx/div);
      };
      render_required = true;
    }
    if(st.m_b==1){ //point something out
      ///trying to pick crossection
      V3f res;
      //  crossection.tiles_coverage(0.25, 1.0);
      //      crossection.update(volume.vol, volume.cursor, V3f(1,0,0), V3f(0,1,0), V3f(0,0,1)),
      bool picked = crossection.pick(st.x, st.y, res);
      if(picked){
	volume.set_cursor(res);
	if(volume.tool != 0)crossection.update();
      }else{
	volume.pick(st.x, st.height-st.y);
	crossection.display_center = volume.cursor;
	printf("Trying to set the window to %d %d %d !!!\n", (int)volume.cursor.x, 
	       (int)volume.cursor.y, 
	       (int)volume.cursor.z);
	crossection.update(volume.vol, volume.cursor);
      };
      render_required = true;
    };
  };

  void do_wheel(){
    printf("wheel:%d\n", st.w);
    if(st.w > 100 || st.w < -50)return;
    zoomf=1.0f+(0.01f*st.w);
    render_required = true;
  };



  void draw(){


    if(render_required == true || st.interface_updated == true){
      //
      if(volume.vol.updated){
	volume.vol.updated = false;
	crossection.update(volume.vol, volume.cursor);
      };
      
      //proj.rot(0.15, 0.092);
      //printf("w%d:h%d\n", width, height);
      gui_resize(st.width, st.height);

      glClearColor(1.0,1.0,1.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
      glEnable(GL_DEPTH_TEST);
      glViewport(0,0, st.width, st.height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
   
      if(st.width>st.height){
	glOrtho((-zoomf*(float)st.width/(float)st.height), zoomf*((float)st.width/(float)st.height), -zoomf*1, zoomf*1, -2, 2);
	volume.point_size(1.5f/zoomf*((float)st.height)/(float)256);
	//volume.point_size(0.2f/zoomf*((float)st.height)/(float)256);
      }else{
	glOrtho(-zoomf*1, zoomf*1, (-zoomf*(float)st.height/(float)st.width), (zoomf*(float)st.height/(float)st.width), -2, 2);
	volume.point_size(1.5f/zoomf*((float)st.width)/(float)256);
	//volume.point_size(0.2f/zoomf*((float)st.width)/(float)256);
      };

 
  
      glMatrixMode(GL_MODELVIEW);
      proj.loadmatrix();
      // glLoadIdentity();

      volume.set_projection();
      crossection.draw_box(); //using volume projection
      volume.draw(proj.z());
  
      if(crossection.update_needed)
	crossection.update(volume.vol, V3f(-1,-1,-1));
      crossection.draw();

      gui_draw();

      render_required = false;
      st.interface_updated = false;
	  glfwSwapBuffers(); //definitely did this before; check GIT...
    }else{
      //do nothing; 
    };
  };
};

///main function... built in main loop; the rest is done by regidtering callback functions.
// Main

#include <string>
using namespace std;

#define RASTER_HELP "vx2 raster lh.pial.asc rh.pial.asc template.mgz result.mgz\n"
#define VOLUMES_HELP "vx2 volumes sample.mgz truth.mgz plus5.mgz all.mgz\n"
#define STAT_HELP "vx2 stat brainmask.mgz attempt.mgz truth.mgz plus5.mgz all.mgz SS080_name res.txt\n"
#define LOAD_HELP "vx2 load to_load.mgz\n"
#define DIFF_HELP "vx2 diff a.mgz b.mgz #tell the difference between 2 masks.\n"
#define JOIN_HELP "vx2 join a.mgz b.mgz res.mgz #join masks in a&b into res.\n"
#define ALL_HELP "vx2 all SS056_IDX # generate volumes and statistics for the patient\n"
#define HELP_HELP ((string("")+RASTER_HELP+VOLUMES_HELP+STAT_HELP+ALL_HELP+DIFF_HELP+LOAD_HELP).c_str())  

//loading right hemisphere + left hemisphere and propagating around it
void vitali(main_module & core, string left, string right, string sample, string result, bool _half = false){
  core.volume.load(sample.c_str());
  read_voxels(left, & core.volume, _half, false /*use border*/, true /*yes, fill please*/);
  read_voxels(right, & core.volume, _half, false, true);
  core.volume.loader.cur_plane = Loader::MASK_PLANE;
  core.volume.save(result.c_str());
};

void create_volumes(main_module & core, string sample, string truth, string truthPlus, string all){
  core.volume.load(sample.c_str());
  core.volume.loader.cur_plane = Loader::MASK_PLANE;
  core.volume.load(truth.c_str());
  core.volume.find_surface();
  core.volume.vol.reseed();
  core.volume.vol.propagate(1000, 0, 1000, 2); //1 layer
  core.volume.save((truth+"_").c_str()); //save stuff
  core.volume.vol.propagate(1000, 0, 1000, 5); //+5 voxels
  core.volume.save(truthPlus.c_str());
  core.volume.vol.propagate(1000, 0, 1000, 100); //+everything
  core.volume.save(all.c_str());
};



void stat(main_module & core, string brainmask, string removed, string truth, string truthPlus, string all, string any_name, string res){
  const int TR = 1;     // truth
  const int TRP = 2;    // 5 mm layer
  const int ALL = 4;    // all but truth
  const int RM = 8;     // removed by user
  const int TRPT = 16;  // 5 mm layer truth & 10 mm layer mask
   const int TRL = 32;   // truth with one layer

  const int max = 256*256*256;

  int wm = 0;
  int wm_n = 0;
  int csf = 0;

  int wmgm = 0;
  int wrongly_preserved = 0;       // in 5 mm layer
  int wrongly_preserved_top = 0;   // in 5 mm layer and no deeper than 10 mm from mask border
  int wrongly_preserved_all = 0;   // in all the non/brain volume avaliable
  int wrongly_removed = 0;         // removed voxels inside truth
  int wrongly_removed_before = 0;  // !!! values less than or equal to 0 within ground truth?
  int correctly_removed = 0;
  int wrongly_preserved_before = 0;
  int wrongly_preserved_before_top = 0;
  int wrongly_preserved_before_all = 0;

  FILE * rep = fopen(res.c_str(), "a");
  if(!rep){
    printf("Unable to report statistics...bailing out.\n");
    return;
  };

  unsigned char * bits = new unsigned char[max];

  core.volume.load(brainmask.c_str());
  core.volume.find_surface();
  printf("REP: %s\n", brainmask.c_str());
  core.volume.loader.cur_plane = Loader::MASK_PLANE;

  //load truth + 1 layer; including half-occupancy
   core.volume.load((truth+"_").c_str());
   for(int i = 0; i < max; i++)
     if(MSK & core.volume.vol.mask[i]){
      bits[i] |= TRL;
   };


  //load truth and compute wm
  core.volume.load(truth.c_str());
  for(int i = 0; i < max; i++)
    if( core.volume.vol.mask[i] != 0){ //TODO
      bits[i] |= TR;
      wm += core.volume.vol.vol[i];
      wm_n++;
    };
  wm /= wm_n; csf = (int)((float)wm * 0.5);
  printf("REP: White matter intensity is %d, taking border %d\n", wm, csf);
  //load truth+
  core.volume.load(truthPlus.c_str());
  core.volume.find_surface();
  for(int i = 0; i < max; i++)
    if((MSK & core.volume.vol.mask[i]) && 
       (!(bits[i] & TRL))){
      bits[i] |= TRP;
      if( core.volume.vol.depth[i] < 10 ){
	bits[i] |= TRPT;
      };
    };
  //load all
  core.volume.load(all.c_str());
  for(int i = 0; i < max; i++)
    if((MSK & core.volume.vol.mask[i])  && 
       (!(bits[i] & TRL))){
      bits[i] |= ALL;
    };

  //load removed
  core.volume.load(removed.c_str());
  for(int i = 0; i < max; i++)
    if(MSK & core.volume.vol.mask[i])bits[i] |= RM;

  //counting itself
  for(int i = 0; i < max; i++){
    core.volume.vol.mask[i]=0;
    if(core.volume.vol.vol[i] > 40  /* csf */  ){ //
      if(bits[i] & TR)wmgm++;
      if(bits[i] & TRPT)wrongly_preserved_before_top++;      
      if(bits[i] & TRP)wrongly_preserved_before++;
      if(bits[i] & ALL)wrongly_preserved_before_all++;
      if((bits[i] & (TR | RM)) == (TR | RM))wrongly_removed++;
      if((bits[i] & (TRP | RM)) == TRP){
	wrongly_preserved++;
	//	core.volume.vol.mask[i] |= MSK;
      };
      if((bits[i] & (TRPT | RM)) == TRPT)wrongly_preserved_top++;
      if((bits[i] & (ALL | RM)) == ALL){
	wrongly_preserved_all++;      
	core.volume.vol.mask[i] |= MSK;
      };

	if((bits[i] & (ALL | RM)) == (ALL | RM))correctly_removed++;
    }else{ //it was edit out previously, and is a FN
      if((core.volume.vol.vol[i] <= 0)&&(bits[i] & TR))wrongly_removed_before++;
    };
  };

  // core.volume.loader.cur_plane = Loader::VOLUME_PLANE;
  core.volume.save((removed+"_").c_str());

  //reporting:
  fprintf(rep, "%s, %d, %d, %d, %d, %d,  %d,  %d, %d, %d\n", any_name.c_str(), \
	  wmgm, wrongly_removed, wrongly_removed_before,\
	  wrongly_preserved_before, wrongly_preserved_before_top, wrongly_preserved_before_all, \
	  wrongly_preserved, wrongly_preserved_top, wrongly_preserved_all);

  delete[] bits;
  fclose(rep);
};

void diff(main_module & core, string a, string b)
{
  core.volume.load(a.c_str()); //load a
  core.volume.loader.cur_plane = Loader::MASK_PLANE; //b goes into mask
  core.volume.load(b.c_str());

  int matches = 0;
  int only_a = 0;
  int only_b = 0;
  for(int i = 0; i < 256*256*256; i++){
    if((core.volume.vol.vol[i] > 0) && (core.volume.vol.mask[i] > 0))matches++;
    if((core.volume.vol.vol[i] > 0) && !(core.volume.vol.mask[i] > 0))only_a++;
    if(!(core.volume.vol.vol[i] > 0) && (core.volume.vol.mask[i] > 0))only_b++;
  };

  printf("RES: matches:%d, a:%d, b:%d\n", matches, only_a, only_b);
};


void join(main_module & core, string a, string b, string res)
{
  core.volume.load(a.c_str()); //load a
  core.volume.loader.cur_plane = Loader::MASK_PLANE; //b goes into mask
  core.volume.load(b.c_str());

  //int matches = 0;
  //int only_a = 0;
  //int only_b = 0;
  for(int i = 0; i < 256*256*256; i++){
    if(core.volume.vol.vol[i] > 0)core.volume.vol.mask[i] |= MSK;
  };

  core.volume.save(res.c_str());
};

bool exists(string filename){
  FILE * ex = fopen(filename.c_str(), "r");
  if(!ex) {
    printf((string("") + "Unable to find " + filename + "\n").c_str());
    return false;
  };
  fclose(ex);
  return true;
};

void do_all(main_module & core, string id){
  string subjects_dir;
  char * e = getenv("SUBJECTS_DIR");
  if(!e){
    printf("Please set SUBJECTS_DIR\n");
    return;
  };

  subjects_dir = string(e);

  string file_mask = subjects_dir + "/" + id + "/mri/brainmask.auto.mgz";
  string file_truth = subjects_dir + "/" + id + "/kdl/truth.mgz";
  string file_plus5 = subjects_dir + "/" + id + "/kdl/plus5.mgz";
  string file_all = subjects_dir + "/" + id + "/kdl/all.mgz";
  string file_attempt = subjects_dir + "/" + id + "/kdl/attempt0.mgz";
  string result = "res.txt";

  if(!(exists(file_mask) && exists(file_truth) && exists(file_plus5) && exists(file_all) && exists(file_attempt)))return; //something is amiss.

  create_volumes(core, file_mask, file_truth, file_plus5, file_all);
  stat(core, file_mask, file_attempt, file_truth, file_plus5, file_all, id, result); 
};

int main(int argc, char ** argv) 
{
  main_module core;
  

  //let's check what those aliens are up to:
  if(argc > 1){

    if(strstr(argv[1], "help")){
 	printf(HELP_HELP);
	return 1;
    };

    if(strstr(argv[1], "raster")){
      if((argc != 6) && (argc != 7)){
	printf(RASTER_HELP);
	return 1;
      }else{
	if(argc == 6){
	  vitali(core, argv[2], argv[3], argv[4], argv[5], false);
	}else{
	  vitali(core, argv[2], argv[3], argv[4], argv[5], true);
	};	  
	return 0; //all done.
      };
    };

    if(strstr(argv[1], "volumes")){
      if(argc != 6){
	printf(VOLUMES_HELP);
	return 1;
      }else{
	create_volumes(core, argv[2], argv[3], argv[4], argv[5]);
	return 0; //all done.
      };
    };

    if(strstr(argv[1], "stat")){
      if(argc != 9){
	printf(STAT_HELP);
	return 1;
      }else{
	stat(core, argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8]);
	return 0; //all done.
      };
    };


    if(strstr(argv[1], "all")){
      if(argc != 3){
	printf(ALL_HELP);
	return 1;
      }else{
	do_all(core, argv[2]);
	return 0; //all done.
      };
    };

    if(strstr(argv[1], "diff")){
      if(argc != 4){
	printf(DIFF_HELP);
	return 1;
      }else{
	diff(core, argv[2], argv[3]);
	return 0; //all done.
      };
    };

    if(strstr(argv[1], "join")){
      if(argc != 5){
	printf(JOIN_HELP);
	return 1;
      }else{
	join(core, argv[2], argv[3], argv[4]);
	return 0; //all done.
      };
    };


    if(strstr(argv[1], "load")){
      if(argc != 3 && argc != 4){
	printf(LOAD_HELP);
	return 1;
      }else{
	core.volume.load(argv[2]); //just continue as normal
	set_current_file(std::string(argv[2]));
	if(argc == 4){
	  core.volume.loader.cur_plane = Loader::MASK_PLANE; //b goes into mask
	  core.volume.load(argv[3]); //load the second file as a mask
	};
      };
    };

  }else{
    printf("No command given; trying to use default brainmask.mgz\n"); //else, just load default stuff
    if( ! core.volume.load(NULL) )return -1;
    set_current_file("Default dummy.");
  };

  core.volume.find_surface();

  color_init();
    
  gl_init(&core);
  gui_start( &core.crossection, &core.volume);


  // Initialize time
  //  tw_gui.time = glfwGetTime();
  // Main loop (repeated while window is not closed and [ESC] is not pressed)
  while( glfwGetWindowParam(GLFW_OPENED) && !glfwGetKey(GLFW_KEY_ESC) )
    {
      core.draw();
     // glfwSwapBuffers(); #seems like I have already deleted it..?
	  glfwPollEvents();	
      glfwSleep(0.02);
    }

  // Terminate AntTweakBar and GLFW
  glfwTerminate();

  gui_stop();
  
  return 0;
};

