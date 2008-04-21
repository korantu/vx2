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
#include "tw_gui.h"

#include "v3.h"

#include <stdio.h>

#include "gl_wrapper.h"
#include "gl_points.h"


#include "v3tools.h"

///mouse button
//	Callback function called by GLFW when a mouse button is clicked
struct main_module : public gl_wrapper_reciever {

  bool render_required;

  TwGui tw_gui;

  main_module(){
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

  GlPoints volume;

  bool load(char * in){ 
    render_required = true;
    return volume.load("brainmask.mgh"); 
  };
  
  void do_render(){
  };

  void do_resize(){
    render_required = true;
    printf("resz: w:%d; h:%d\n", st.width, st.height);
  };
  void do_key(){
  };
  void do_mouse(){
    float speed = 4;
    if(st.m_b==2){ //change view
      float div = (st.width<st.height)?((float)st.width):((float)st.height);
      proj.rot(speed*(float)st.dy/div,speed*(float)st.dx/div);
      render_required = true;
    };
    if(st.m_b==1){ //point something out
      volume.pick(st.x, st.height-st.y);
      render_required = true;
    };
  };

  void do_wheel(){
  };


  void draw(){
    if(render_required == true || st.interface_updated == true){
      //proj.rot(0.15, 0.092);
      //printf("w%d:h%d\n", width, height);
      tw_gui.resize(st.width, st.height);
      glClearColor(1.0,1.0,1.0, 1.0);
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
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

      volume.draw();
	    
      tw_gui.draw();
      render_required = false;
      st.interface_updated = false;
    }else{
      //do nothing; 
    };
  };
};

///main function... built in main loop; the rest is done by regidtering callback functions.
// Main

int main(int argc, char ** argv) 
{
  main_module core;

  core.tw_gui.init();
  core.volume.gui();
  if( ! core.volume.load("brainmask.mgh") )return -1;
  
  gl_init(&core);


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
  core.tw_gui.stop();
  
  return 0;
};








