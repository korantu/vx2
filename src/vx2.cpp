//	---------------------------------------------------------------------------
//
//	@file		TwSimpleGLFW.c
//	@brief		A simple example that uses AntTweakBar with 
//				OpenGL and the GLFW windowing system.
//
//				AntTweakBar: http://www.antisphere.com/Wiki/tools:anttweakbar
//				OpenGL:		 http://www.opengl.org
//				GLFW:		 http://glfw.sourceforge.net
//	
//	@author		Philippe Decaudin - http://www.antisphere.com
//	@date		2006/05/20
//
//	note:		TAB=4
//
//	Compilation:
//	http://www.antisphere.com/Wiki/tools:anttweakbar:examples#twsimpleglfw
//
//	---------------------------------------------------------------------------


#ifdef USE_TW
#endif // USE_TW

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


///mouse button
///	Callback function called by GLFW when a mouse button is clicked
struct main_module : public gl_wrapper_reciever {

  bool render_required;

  slices crossection;


  GlPoints volume;


  main_module():crossection(& volume){
    render_required = true;
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
    render_required = true;
    printf("resz: w:%d; h:%d\n", st.width, st.height);
    crossection.resize_screen(st.width, st.height);
    //  crossection.tiles_coverage(0.25, 1.0);
    crossection.update(volume.vol, volume.cursor);//, V3f(1,0,0), V3f(0,1,0), V3f(0,0,1));
    
  };
  void do_key(){
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
	volume.set_cursor(res);
	printf("Picked...%f %f %f\n", res.x, res.y, res.z);
	crossection.center = res;
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
	crossection.update(volume.vol, volume.cursor);
      };
      render_required = true;
    };
  };

  void do_wheel(){
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
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      glEnable(GL_DEPTH_TEST);
      glViewport(0,0, st.width, st.height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
   
      if(st.width>st.height){
	glOrtho((-(float)st.width/(float)st.height), ((float)st.width/(float)st.height), -1, 1, -2, 2);
	volume.point_size(1.5*((float)st.height)/(float)256);
      }else{
	glOrtho(-1, 1, (-(float)st.height/(float)st.width), ((float)st.height/(float)st.width), -2, 2);
	volume.point_size(1.5*((float)st.width)/(float)256);
      };

 
  
      glMatrixMode(GL_MODELVIEW);
      proj.loadmatrix();
      // glLoadIdentity();

      volume.set_projection();
      crossection.draw_box(); //using volume projection
      volume.draw(proj.z());
  
      if(crossection.update_needed)
	crossection.update(volume.vol);
      crossection.draw();

      gui_draw();

      render_required = false;
      st.interface_updated = false;
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
#define STAT_HELP "vx2 stat brainmask.mgz attempt.mgz truth.mgz plus5.mgz all.mgz\n"
#define LOAD_HELP "vx2 load to_load.mgz\n"

//loading right hemisphere + left hemisphere and propagating around it
void vitali(main_module & core, string left, string right, string sample, string result){
  core.volume.load(sample.c_str());
  read_voxels(left, & core.volume);
  read_voxels(right, & core.volume);
  core.volume.loader.cur_plane = Loader::MASK_PLANE;
  core.volume.save(result.c_str());
};

void create_volumes(main_module & core, string sample, string truth, string truthPlus, string all){
  core.volume.load(sample.c_str());
  core.volume.loader.cur_plane = Loader::MASK_PLANE;
  core.volume.load(truth.c_str());
  core.volume.find_surface();
  core.volume.vol.reseed();
  core.volume.vol.propagate(1000, 0, 1000, 6); //+5 voxels
  core.volume.save(truthPlus.c_str());
  core.volume.vol.propagate(1000, 0, 1000, 100); //+everything
  core.volume.save(all.c_str());
  
};

void stat(main_module & core, string brainmask, string removed, string truth, string truthPlus, string all){
  const int TR = 1;  //truth
  const int TRP = 2; //5 mm layer
  const int ALL = 4; // all but truth
  const int RM = 8;  // removed by user
  const int TRPT = 16; //5 mm layer truth & 10 mm layer mask

  const int max = 256*256*256;

  int wm = 0;
  int wm_n = 0;
  int csf = 0;

  int wmgm = 0;
  int wrongly_preserved = 0;       //wrongly preserved in 5 mm layer
  int wrongly_preserved_top = 0;   //wrongly preserved in 5 mm layer and no deeper than 10 mm from mask bdr
  int wrongly_preserved_all = 0;
  int wrongly_removed = 0;
  int correctly_removed = 0;
  int wrongly_preserved_before = 0;
  int wrongly_preserved_before_top = 0;
  int wrongly_preserved_before_all = 0;

  unsigned char * bits = new unsigned char[max];

  core.volume.load(brainmask.c_str());
  core.volume.find_surface();
  printf("REP: %s\n", brainmask.c_str());
  core.volume.loader.cur_plane = Loader::MASK_PLANE;
  //load truth and compute wm
  core.volume.load(truth.c_str());
  for(int i = 0; i < max; i++)
    if(MSK & core.volume.vol.mask[i]){
      bits[i] |= TR;
      wm += core.volume.vol.vol[i];
      wm_n++;
    };
  wm /= wm_n; csf = (int)((float)wm * 0.4);
  printf("REP: White matter intensity is %d, taking border %d\n", wm, csf);
  //load truth+
  core.volume.load(truthPlus.c_str());
  core.volume.find_surface();
  for(int i = 0; i < max; i++)
    if((MSK & core.volume.vol.mask[i]) && 
       (!(bits[i] & TR))){
      bits[i] |= TRP;
      wrongly_preserved_before++;
      if( core.volume.vol.depth[i] < 10 ){
	wrongly_preserved_before_top++;      
	bits[i] |= TRPT;
      };
   };
  //load all
  core.volume.load(all.c_str());
  for(int i = 0; i < max; i++)
    if(MSK & core.volume.vol.mask[i]  && 
       (!(bits[i] & TR))){
      bits[i] |= ALL;
      wrongly_preserved_before_all++;
    };

  //load removed
  core.volume.load(removed.c_str());
  for(int i = 0; i < max; i++)
    if(MSK & core.volume.vol.mask[i])bits[i] |= RM;

  //counting itself
  for(int i = 0; i < max; i++){
    if(core.volume.vol.vol[i] > csf){
      if(bits[i] & TR)wmgm++;
      if((bits[i] & (TR | RM)) == (TR | RM))wrongly_removed++;
      if((bits[i] & (TRP | RM)) == TRP)wrongly_preserved++;
      if((bits[i] & (TRPT | RM)) == TRPT)wrongly_preserved_top++;
      if((bits[i] & (ALL | RM)) == ALL)wrongly_preserved_all++;
      if((bits[i] & (ALL | RM)) == (ALL | RM))correctly_removed++;
    };
  };


  //reporting:
  printf("REP: %d, %d, %d, %d,  %d,  %d, %d, %d\n", wmgm, wrongly_removed, wrongly_preserved_before, wrongly_preserved_before_top, wrongly_preserved_before_all, wrongly_preserved, wrongly_preserved_top, wrongly_preserved_all);

  delete bits;
};

int main(int argc, char ** argv) 
{
  main_module core;
  

  //let's check what those aliens are up to:
  if(argc > 1){
    if(strstr(argv[1], "raster")){
      if(argc != 6){
	printf(RASTER_HELP);
	return 1;
      }else{
	vitali(core, argv[2], argv[3], argv[4], argv[5]);
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
      if(argc != 7){
	printf(STAT_HELP);
	return 1;
      }else{
	stat(core, argv[2], argv[3], argv[4], argv[5], argv[6]);
	return 0; //all done.
      };
    };

    if(strstr(argv[1], "load")){
      if(argc != 3){
	printf(LOAD_HELP);
	return 1;
      }else{
	core.volume.load(argv[2]); //just continue as normal
      };
    };

  }else{
    printf("No command given; trying to use default brainmask.mgz\n"); //else, just load default stuff
    if( ! core.volume.load(NULL) )return -1;
  };

  core.volume.find_surface();

  color_init();
;
  
   gl_init(&core);
   gui_start( &core.crossection, &core.volume);


  // Initialize time
  //  tw_gui.time = glfwGetTime();
  // Main loop (repeated while window is not closed and [ESC] is not pressed)
  while( glfwGetWindowParam(GLFW_OPENED) && !glfwGetKey(GLFW_KEY_ESC) )
    {
      core.draw();
      glfwSwapBuffers();
      glfwSleep(0.02);
    }

  // Terminate AntTweakBar and GLFW
  glfwTerminate();

  gui_stop();
  
  return 0;
};











