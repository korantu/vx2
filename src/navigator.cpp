#include "navigator.h"

///setting inital state to the camera;
camera::camera() {
  center = V3f(0,0,0);
  eye = V3f(0,0,-1);
  up = V3f(0,1,0);
};

///Move the camera by dx/dy
camera & move(camera & to_move, int dx, int dy){
  float fdx = 0.01f*dx; //inital values; scaled
  float fdy = 0.01f*dy;

  to_move.eye = to_move.eye + to_move.up*fdy;
  
  V3f side; //side vector

  side.cross((to_move.center-to_move.eye),to_move.up);
  side /= side.length();
  to_move.eye -= side*fdx;
  to_move.up.cross(side,(to_move.center-to_move.eye));
  to_move.up /= to_move.up.length();
  
  V3f dir = to_move.eye-to_move.center; //normalizing looking direction
  dir /= dir.length();
  to_move.eye = to_move.center + dir;
  return to_move;
};

/// setup projection according to the eye
void lookAt(camera & to_look){
  V3f eye(to_look.eye); //init
  V3f center(to_look.center);
  V3f up(to_look.up);

  //use GLU to setup the projection; 
 gluLookAt(eye.x, eye.y, eye.z, 
	    center.x, center.y, center.z, 
	    up.x, up.y, up.z);
};

///get opengl pos out of screen coords; all the required information is obtained from opengl state;
V3f GetOGLPos(int x, int y, int z)
{
  return V3f(0,0,0);
  GLint viewport[4];
  GLdouble modelview[16];
  GLdouble projection[16];
   GLfloat winX = (float)x;
   GLfloat winY = (float)y;
   GLfloat winZ = (float)z;
  GLdouble posX, posY, posZ;

  glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
  glGetDoublev( GL_PROJECTION_MATRIX, projection );
  glGetIntegerv( GL_VIEWPORT, viewport );

  gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

  return V3f((float)posX, (float)posY, (float)posZ);
}


















