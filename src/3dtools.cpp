#include "3dtools.h"

InteractiveRenderer::InteractiveRenderer(Drawable & scene):
  scene_(scene){
    // Initialise GLFW
  glfwInit();
  
  // Open OpenGL window
  if( !glfwOpenWindow( 500, 500, 0,0,0,0, 16,0, GLFW_WINDOW ) )
    {
      glfwTerminate();
      valid(false);
    }
    glfwSetWindowTitle( "3D" );
    glfwEnable( GLFW_STICKY_KEYS );
    glfwEnable( GLFW_MOUSE_CURSOR );
    glfwDisable( GLFW_AUTO_POLL_EVENTS );
};

InteractiveRenderer::~InteractiveRenderer(){
  if(valid())glfwTerminate();
};

void InteractiveRenderer::Loop(){
  while(!glfwGetKey( GLFW_KEY_ESC ) && glfwGetWindowParam( GLFW_OPENED )){
    DrawFrame();
  };
};

void InteractiveRenderer::DrawFrame(){
  SetupProjections(); // viewport, projection and modelview go here.
  scene_.Draw();
  glfwWaitEvents();
};

//Implementation of various 3d visualization tools.

// Crossection display.
// Unimplemented

Crossection & Crossection::Resize(int, int){
  return *this;
};

Crossection & Crossection::Position(V3f &){
  return *this;
};

Crossection & Crossection::Orientation(V3f &){
  return *this;
};

Crossection & Crossection::Draw(){
  return *this;
}; 
