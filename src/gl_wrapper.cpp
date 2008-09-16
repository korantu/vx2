#define GLFW_DLL
#define USE_TW
#include "gl_wrapper.h"
#include <AntTweakBar.h>

///for now, the event is defined here:
///it is initialized first, so guaranteed not to be null;
gl_wrapper_reciever * rcv = 0;

///Constructor for the event structure:
gl_wrapper_reciever::gl_wrapper_reciever(){
  st.width = 0;  // window params
  st.height = 0;
  st.x = 0; //current mouse pos
  st.y = 0;
  st.w = 0; //whell
  st.dx = 0; //previous mouse pos
  st.dy = 0;
  st.dw = 0;
  st.k = 0;    //currently pressed key, if any
  st.m_b = 0;    //currently pressed m/b, if any
  st.interface_updated = false;
};

//virtual destructor to keep the compiler happy.
gl_wrapper_reciever::~gl_wrapper_reciever(){
};

///universal callbacks definitions:
void resize_callback( int x, int y){
  rcv->st.width = x;
  rcv->st.height = y;
  rcv->do_resize();;
};

void move_mouse_callback(int _x, int _y, int key=-1){
  rcv->st.dx = _x-rcv->st.x;
  rcv->st.dy = _y-rcv->st.y;
  rcv->st.x = _x;
  rcv->st.y = _y;
  if(key != -1)rcv->st.m_b = key; //in case key info has changed;
  rcv->do_mouse();
};


void move_wheel_callback(int pos){
  rcv->st.dw = pos-rcv->st.w;
  rcv->st.w = pos;
  rcv->do_wheel();
};


void press_key_callback(int key){
  rcv->st.k = key;
  rcv->do_key();
};


void invalidate(){
  rcv->st.interface_updated = true;
};

/************
 GLFW-specific stuff follows:
************/

#include "GL/glfw.h"
#include "stdlib.h"
#include "stdio.h"

//callback declarations
void GLFWCALL WindowSizeCB(int width, int height);
void GLFWCALL OnMouseButton(int glfwButton, int glfwAction);
void GLFWCALL OnKey(int glfwButton, int glfwAction);
void GLFWCALL OnMousePos(int x, int y);
void GLFWCALL OnMouseWheel(int pos);

//utility functions
void glfw_init_window();
void glfw_init_callbacks();


void gl_init(gl_wrapper_reciever * wr){
  rcv = wr; //can send events now;

  glfw_init_window(); //setup window
  glfw_init_callbacks(); //register callbacks

};

void gl_stop(){
  glfwTerminate();
};

//initializaiton
void glfw_init_window(){


  if( !glfwInit() )
    {
      // A fatal error occured
      fprintf(stderr, "GLFW initialization failed\n");
      throw "Cannot init glfw\n";
    }
  ///create desktop - also an intializaiton.
  // Create a window
  GLFWvidmode mode; 	// GLFW video mode
  glfwGetDesktopMode(&mode);
  if( !glfwOpenWindow(632, 453, mode.RedBits, mode.GreenBits, mode.BlueBits, 0, 16, 0, GLFW_WINDOW /* or GLFW_FULLSCREEN */) )
    {
      // A fatal error occured	
      fprintf(stderr, "Cannot open GLFW window\n");
      glfwTerminate();
      throw "Cannot open window\n";
    }
  glfwEnable(GLFW_MOUSE_CURSOR);
  glfwEnable(GLFW_KEY_REPEAT);
  glfwSetWindowTitle("3D voxel editor.");
  
};

void glfw_init_callbacks(){
  glfwSetWindowSizeCallback(WindowSizeCB);

#ifdef USE_TW
  glfwSetCharCallback((GLFWcharfun)TwEventCharGLFW);
#endif

  // - Filter mouse button events
  glfwSetMouseButtonCallback((GLFWmousebuttonfun)OnMouseButton);
  // - Filter mouse position events
  glfwSetMousePosCallback((GLFWmouseposfun)OnMousePos);
  // - Filter mouse wheel events
  glfwSetMouseWheelCallback((GLFWmousewheelfun)OnMouseWheel);
  // - Filter keyboard events
  glfwSetKeyCallback((GLFWkeyfun)OnKey);
};


//mouse button callback
void GLFWCALL OnKey(int glfwButton, int glfwAction)
{  
  if(
#ifdef USE_TW
     !TwEventKeyGLFW(glfwButton, glfwAction) 
#else
     true
#endif
     )	// Send event to AntTweakBar
    {
      if( glfwButton==GLFW_KEY_ESC && glfwAction==GLFW_PRESS ) // Want to quit?
	glfwCloseWindow();
      else
	{
	  int k;
	  //record mouse button info
	  if(glfwAction == GLFW_PRESS){
	    k = glfwButton;
	  }else{
	    k = 0;
	  };

	  press_key_callback(k); //report
	};
    }else{
  };
    invalidate();
};

//mouse button callback
void GLFWCALL OnMouseButton(int glfwButton, int glfwAction)
{
  if( 
#ifdef USE_TW
     !TwEventMouseButtonGLFW(glfwButton, glfwAction) 
#else
     true
#endif
      )	// Send event to AntTweakBar
    {
  int cur_x, cur_y, m_b;
  glfwGetMousePos(&cur_x, &cur_y);
  
  //record mouse button info
  if(glfwAction == GLFW_PRESS){
    m_b = glfwButton+1; //buttons should start with 0. 
  }else{
    m_b = 0;
  };

  move_mouse_callback(cur_x, cur_y, m_b); //report
    }else{
    invalidate();
  };
};

//mouse position callback
void GLFWCALL OnMousePos(int x, int y){
  if( 
#ifdef USE_TW
     !TwEventMousePosGLFW(x, y) 
#else
     true
#endif
      )	// Send event to AntTweakBar
    {
      
      move_mouse_callback(x, y);
    }else{
    invalidate();
  };
};

//mouse wheel callback
void GLFWCALL OnMouseWheel(int pos){
  if( 
#ifdef USE_TW 
     !TwEventMouseWheelGLFW(pos) 
#else
     true
#endif
      )	// Send event to AntTweakBar
    {
      move_wheel_callback(pos);
    }else{
    invalidate();
  };
};
 
void GLFWCALL WindowSizeCB(int w, int h){
  printf("> w:%d; h:%d\n", w,h);
#ifdef USE_TW
    TwWindowSize(w, h);
#endif
  resize_callback(w, h);
    invalidate();
};










