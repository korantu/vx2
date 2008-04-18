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


#define USE_TW 1

#ifdef USE_TW
#include <AntTweakBar.h>
#endif // USE_TW

#define GLFW_DLL

#include "glfw.h"

#include "v3.h"

#include <stdio.h>

#include "navigator.h"

/// abstraction is too high.
struct state_t{
  enum tool_t{
    DO_ROTATE,
    DO_SCALE,
    DO_NOTHING,
  } tool;

  /// do we need a separate 2d GUI layer?
  camera main_camera;
  int mouse_x; //for tracking mouse movements
  int mouse_y;

  state_t(){
    tool = DO_NOTHING;
  };

} state;

///ok.... this should be better thought out
///change current action; reset original x and y;
void switch_action(state_t & state, int x, int y, state_t::tool_t new_tool){
  state.mouse_x = x;
  state.mouse_y = y;
  state.tool = new_tool;
};

///windowsize - 
/// + redraw
//_* Callbacks
// Callback function called by GLFW when window size changes
void GLFWCALL WindowSizeCB(int width, int height)
{
  // Set OpenGL viewport and camera
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if(0 == height)height = 1; 
  glOrtho(-1, 1, -((float)height/(float)width), ((float)height/(float)width), -2, 2);
	
  // Send the new window size to AntTweakBar
#ifdef USE_TW
  TwWindowSize(width, height);
#endif

}


//_* Drawing
// This example program draws a possibly transparent cube 
void DrawModel(int wireframe)
{
  int pass, numPass;
  
  // Enable OpenGL transparency and light (could have been done once at init)
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHT0);	// use default light diffuse and position
  glDisable(GL_LIGHTING);	// use default light diffuse and position
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
  glEnable(GL_LINE_SMOOTH);
  glLineWidth(3.0);
  glBegin(GL_LINES);
  //kdl: just to fine-tune motions.
  glColor3f(1,0,0);  glVertex3f(0.5,0.5,0.5); glVertex3f(1,0.5,0.5); 
  glColor3f(0,1,0);  glVertex3f(0.5,0.5,0.5); glVertex3f(0.5,1,0.5); 
  glColor3f(0,0,1);  glVertex3f(0.5,0.5,0.5); glVertex3f(0.5,0.5,1); 
  glEnd();
}


//callback functions

///mouse button
//	Callback function called by GLFW when a mouse button is clicked
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
      // Event has not been handled by AntTweakBar
      // Do something if needed.
	  
      int cur_x, cur_y;
      glfwGetMousePos(&cur_x, &cur_y);

      ///releasing
      if(glfwAction == GLFW_RELEASE){
	switch_action(state, cur_x, cur_y, state_t::DO_NOTHING);
	return;
      };

      ///moving around
      if(glfwButton == GLFW_MOUSE_BUTTON_LEFT){
	switch_action(state, cur_x, cur_y, state_t::DO_ROTATE);
      };

      printf("mb(button = %d, action = %d)\n", glfwButton, glfwAction);
    }
}

///mouse pos; the overlay interface shoud be handled automatically; can it be done in uniform way?
//	Callback function called by GLFW when mouse has moved
void GLFWCALL OnMousePos(int mouseX, int mouseY)
{
  if( 
#ifdef USE_TW
     !TwEventMousePosGLFW(mouseX, mouseY) 
#else
     true
#endif
      )	// Send event to AntTweakBar
    {
      // Event has not been handled by AntTweakBar
      // Do something if needed.
      printf("mp(x = %d, y = %d)\n", mouseX, mouseY);	  
	  
      if(state.tool == state_t::DO_ROTATE)
	move(state.main_camera, mouseX - state.mouse_x, mouseY - state.mouse_y); 

    }

  state.mouse_x = mouseX;
  state.mouse_y = mouseY;

}

///mouse wheell... additional param to the state event.. 
//	Callback function called by GLFW on mouse wheel event
void GLFWCALL OnMouseWheel(int pos)
{
  if( 
#ifdef USE_TW 
     !TwEventMouseWheelGLFW(pos) 
#else
     true
#endif
      )	// Send event to AntTweakBar
    {
      // Event has not been handled by AntTweakBar
      // Do something if needed.
      printf("wl(pos = %d)\n", pos);
    }
}

///On key.... needs focus or something.
//	Callback function called by GLFW on key event
void GLFWCALL OnKey(int glfwKey, int glfwAction)
{
  if(
#ifdef USE_TW
     !TwEventKeyGLFW(glfwKey, glfwAction) 
#else
     true
#endif
     )	// Send event to AntTweakBar
    {
      if( glfwKey==GLFW_KEY_ESC && glfwAction==GLFW_PRESS ) // Want to quit?
	glfwCloseWindow();
      else
	{
	  // Event has not been handled
	  // Do something if needed.
	  printf("kb(key = %d, action = %d)\n", glfwKey, glfwAction);
	}
    }
}


struct TwGui{

  init(){
#ifdef USE_TW
    ///initalization of the interface.
    // Initialize AntTweakBar
    if( !TwInit(TW_OPENGL, NULL) )
      {
	// A fatal error occured	
	fprintf(stderr, "AntTweakBar initialization failed: %s\n", TwGetLastError());
	glfwTerminate();
	return 1;
      }


        
    /// Create a tweak bar
    bar = TwNewBar("Values");

    ///building bars is a different business..
     
    TwDefine(" GLOBAL help='Voxelbrain Voxel editor. "); // Message added to the help bar.


    // Add 'speed' to 'bar': it is a modifable (RW) variable of type TW_TYPE_DOUBLE. Its key shortcuts are [s] and [S].
    TwAddVarRW(bar, "speed", TW_TYPE_DOUBLE, &speed, " label='Rot speed' min=0 max=2 step=0.01 keyIncr=s keyDecr=S help='Rotation speed of the cube (in turns/second)' ");
    // Add 'wire' to 'bar': it is a modifable variable of type TW_TYPE_BOOL32 (32 bits boolean). Its key shortcut is [w].
    TwAddVarRW(bar, "wire", TW_TYPE_BOOL32, &wired, " label='Wireframe mode' key=w help='Toggle wireframe display mode.' ");
    // Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
    TwAddVarRO(bar, "time", TW_TYPE_DOUBLE, &time, " label='Time' precision=1 help='Time (in seconds).' ");			
    // Add 'bgColor' to 'bar': it is a modifable variable of type TW_TYPE_COLOR3F (3 floats color)
    TwAddVarRW(bar, "bgColor", TW_TYPE_COLOR3F, &bgColor, " label='Background color' ");
    // Add 'cubeColor' to 'bar': it is a modifable variable of type TW_TYPE_COLOR32 (32 bits color) with alpha
    TwAddVarRW(bar, "cubeColor", TW_TYPE_COLOR32, &cubeColor, " label='Cube color' alpha help='Color and transparency of the cube.' ");
#endif //USE_TW
    ///callbacks

  };

  stop(){
#ifdef USE_TW
    TwTerminate();
#endif
  };

  void draw(){
#ifdef USE_TW
    TwDraw();
#endif
  };

#ifdef USE_TW
  ///interface state:
  TwBar *bar;			// Pointer to a tweak bar
#endif	

  GLFWvidmode mode; 	// GLFW video mode
  ///simulation state. completely different thing.
  double time = 0, dt;// Current time and enlapsed time
  double turn = 0; 	// Model turn counter
  double speed = 0.3;	// Model rotation speed
  int wire = 0;		// Draw model in wireframe?
  int wired = 0;		//KDL dummy variable
  float bgColor[] = { 0.1f, 0.2f, 0.4f };			// Background color	
  unsigned char cubeColor[] = { 255, 0, 0, 128 };	// Model color (32bits RGBA)


};

TwGui tw_gui;

///main function... built in main loop; the rest is done by regidtering callback functions.
// Main
int main(int argc, char ** argv) 
{

  GlPoints volume;

  if( !volume->load("brainmask.mgh") )return -1;


  //kdl_projection theProjection;
  ///initializtion of the interface.
  // Intialize GLFW	
  if( !glfwInit() )
    {
      // A fatal error occured
      fprintf(stderr, "GLFW initialization failed\n");
      return 1;
    }
  
  tw_gui.init();

  ///create desktop - also an intializaiton.
  // Create a window
  glfwGetDesktopMode(&mode);
  if( !glfwOpenWindow(640, 480, mode.RedBits, mode.GreenBits, mode.BlueBits, 0, 16, 0, GLFW_WINDOW /* or GLFW_FULLSCREEN */) )
    {
      // A fatal error occured	
      fprintf(stderr, "Cannot open GLFW window\n");
      glfwTerminate();
      return 1;
    }
  glfwEnable(GLFW_MOUSE_CURSOR);
  glfwEnable(GLFW_KEY_REPEAT);
  glfwSetWindowTitle("3D voxel editor.");

  // Set GLFW event callbacks
  // - Redirect window size changes to the callback function WindowSizeCB
  glfwSetWindowSizeCallback(WindowSizeCB);
  // - Directly redirect GLFW char events to AntTweakBar
#ifdef USE_TW
  glfwSetCharCallback((GLFWcharfun)TwEventCharGLFW);
#endif

  //Callbacks used by app and anttweakbar:
  // - Filter mouse button events
  glfwSetMouseButtonCallback((GLFWmousebuttonfun)OnMouseButton);
  // - Filter mouse position events
  glfwSetMousePosCallback((GLFWmouseposfun)OnMousePos);
  // - Filter mouse wheel events
  glfwSetMouseWheelCallback((GLFWmousewheelfun)OnMouseWheel);
  // - Filter keyboard events
  glfwSetKeyCallback((GLFWkeyfun)OnKey);


  // Initialize time
  time = glfwGetTime();
  // Main loop (repeated while window is not closed and [ESC] is not pressed)
  while( glfwGetWindowParam(GLFW_OPENED) && !glfwGetKey(GLFW_KEY_ESC) )
    {
      glClearColor(bgColor[0], bgColor[1], bgColor[2], 1);
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();


      volume->draw();
	    

      //kdlSetupProjection(theProjection);
      tw_gui->draw();
      glfwSwapBuffers();
    }

  // Terminate AntTweakBar and GLFW
  glfwTerminate();
  tw_gui.stop();
  return 0;
}








