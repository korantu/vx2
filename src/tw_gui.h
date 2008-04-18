#ifndef __TW_GUI__
#define __TW_GUI__

#include "glfw.h"
#include <AntTweakBar.h>


struct TwGui{


#ifdef USE_TW
  ///interface state:
  TwBar *bar;			// Pointer to a tweak bar
#endif	
  ///simulation state. completely different thing.
  double time, dt;// Current time and enlapsed time
   double turn ; 	// Model turn counter
   double speed ;	// Model rotation speed
   int wire ;		// Draw model in wireframe?
   int wired ;		//KDL dummy variable

  TwGui(){
   time = 0, dt;// Current time and enlapsed time
   turn = 0; 	// Model turn counter
   speed = 0.3;	// Model rotation speed
   wire = 0;		// Draw model in wireframe?
   wired = 0;		//KDL dummy variable

  };


  void init(){
#ifdef USE_TW
    ///initalization of the interface.
    // Initialize AntTweakBar
    if( !TwInit(TW_OPENGL, NULL) )
      {
	// A fatal error occured	
	fprintf(stderr, "AntTweakBar initialization failed: %s\n", TwGetLastError());
	glfwTerminate();
      }


    /*        
    /// Create a tweak bar
    bar = TwNewBar("Values");

    ///building bars is a different business..
     
    TwDefine(" GLOBAL help='Voxelbrain Voxel editor.' "); // Message added to the help bar.


    // Add 'speed' to 'bar': it is a modifable (RW) variable of type TW_TYPE_DOUBLE. Its key shortcuts are [s] and [S].
    TwAddVarRW(bar, "speed", TW_TYPE_DOUBLE, &speed, " label='Rot speed' min=0 max=2 step=0.01 keyIncr=s keyDecr=S help='Rotation speed of the cube (in turns/second)' ");
    // Add 'wire' to 'bar': it is a modifable variable of type TW_TYPE_BOOL32 (32 bits boolean). Its key shortcut is [w].
    TwAddVarRW(bar, "wire", TW_TYPE_BOOL32, &wired, " label='Wireframe mode' key=w help='Toggle wireframe display mode.' ");
    // Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
    TwAddVarRO(bar, "time", TW_TYPE_DOUBLE, &time, " label='Time' precision=1 help='Time (in seconds).' ");			
    */
#endif //USE_TW
    ///callbacks

  };

  void stop(){
#ifdef USE_TW
    TwTerminate();
#endif
  };

  void draw(){
#ifdef USE_TW
    TwDraw();
#endif
  };

  void resize(int w, int h){
#ifdef USE_TW
    TwWindowSize(w, h);
#endif

  };
};

#endif //__TW_GUI__
