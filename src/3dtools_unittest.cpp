#include <gtest/gtest.h>

#include "3dtools.h"
#include "fastvolume.h"
#include "loader.h"
#include "glfw.h"

// Test loading of volume
TEST(Mgz, Interface){
  FastVolume volume;
  MgzLoader mri(volume);
  MgzLoader mri_bad(volume);
  EXPECT_TRUE(mri.Load("brainmask.mgz"));
  EXPECT_FALSE(mri_bad.Load("brainmask.mgz.unexistent"));

  int random_voxel = (++volume.vol[5]); //alter a voxel

  EXPECT_TRUE(mri.Save("brainmask.mgz"));
  EXPECT_TRUE(mri.Load("brainmask.mgz"));
  
  EXPECT_EQ(random_voxel, volume.vol[5]);
}; 

/// Conditions:
/// Rendering happens in object space.
/// Rendering happens in window space.

/// One can convert from window space to object vector.
/// 

/// Facilities:
/// Adding drawing hooks.
/// Getting events.

//Show an opengl window.


class Drawer{
  virtual void draw() = 0;
};

class Scene{
  void Run();
  void Add3D();
private:
  void OpenWindow();
  void CloseWindow();
  void DrawFrame();
};
  void OpenWindow(){
    glfwInit();
    if( !glfwOpenWindow( 500, 500, 0,0,0,0, 16,0, GLFW_WINDOW ) )
      {
        glfwTerminate();
      }
    glfwSetWindowTitle( "Test window." );
    glfwEnable( GLFW_STICKY_KEYS );
    glfwEnable( GLFW_MOUSE_CURSOR );
    glfwDisable( GLFW_AUTO_POLL_EVENTS );
  };

void DrawFrame(){
      glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
      glBegin(GL_TRIANGLES);
      glVertex3f(0,0,0);
      glVertex3f(0,1.0,0);
      glVertex3f(0,1.0,2.0);
      glEnd();
};

void CloseWindow(){
    glfwTerminate();
};

TEST(OGL, BasicWindow){

    do
    {
      DrawFrame();
      glfwSwapBuffers();
      glfwWaitEvents();
    }
    while(!glfwGetKey( GLFW_KEY_ESC ) &&
	  glfwGetWindowParam( GLFW_OPENED ));

};


