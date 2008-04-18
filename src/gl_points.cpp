#include "gl_points.h"
#include "glfw.h"
#include "v3.h"
#include "v3tools.h"
#include <string>

bool GlPoints::load(char * in){
  try{
    int cnt = loader.read(std::string(in));
    printf("Read %d bytes.\n", cnt);

    loader.read_volume(vol);

    vol.findSurface(list, 1); // TODO extract the surface... should be somewhere else;
    printf("found %d points.\n", list.size());
  }catch(const Loader::Ex & err){
    printf("Problem occured: %s\n", err.reason.c_str());
    return false;
  };
  return true;
};

bool GlPoints::save(char * out){
  try {
    loader.write_volume(vol); //save data from volume into the loader
    loader.write(std::string(out));
  }catch(const Loader::Ex & err){
    printf("Problem occured: %s\n", err.reason.c_str());
    return false;
  };
  return true;
};;

void GlPoints::draw(){
  glDisable(GL_LIGHTING);

  //draw to get the idea of the whole thing.
  glBegin(GL_LINES);
  glColor3f(1,0,0); glVertex3f(1,0,0); glVertex3f(-1,0,0);
  glColor3f(0,1,0); glVertex3f(0,1,0); glVertex3f(0,-1,0);
  glColor3f(0,0,1); glVertex3f(0,0,1); glVertex3f(0,0,-1);
  glEnd();

  glScalef(2.0/256.0, 2.0/256.0, 2.0/256.0);
  glTranslatef(-128.0, -128.0, -128.0);

  //ok, the modelview transform is complete here, dumping it for lookup:

  glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
  glGetDoublev( GL_PROJECTION_MATRIX, projection );
  glGetIntegerv( GL_VIEWPORT, (GLint *)viewport );
  


  int cursor_size = 10;

  ///draw cursor
  glBegin(GL_LINES);
  glColor3f(1,0,0); glVertex3f(cursor.x+cursor_size,cursor.y,cursor.z); glVertex3f(cursor.x-cursor_size,cursor.y,cursor.z);
  glColor3f(0,1,0); glVertex3f(cursor.x,cursor.y+cursor_size,cursor.z); glVertex3f(cursor.x,cursor.y-cursor_size,cursor.z);
  glColor3f(0,0,1); glVertex3f(cursor.x,cursor.y,cursor.z+cursor_size); glVertex3f(cursor.x,cursor.y,cursor.z-cursor_size);
  glEnd();


  //glPointSize(pnt);
  glPointSize(1);
  //glEnable(GL_POINT_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glBegin(GL_POINTS);
  int x, y, z;
  for(std::vector<int>::iterator i = list.begin(); i != list.end(); i++){
    vol.getCoords(*i, x,y,z);
    float pnt_col =  vol.vol[*i]/300.0;
    glColor3f(pnt_col, pnt_col, pnt_col);
    glVertex3i(x,y,z);
  };
  glEnd();
  
};

//TODO
#define MAX2(a,b) (((a)>(b))?(a):(b))
#define ABS(a) (MAX2((a),(-(a))))
#define MAX3(a,b,c) MAX2(MAX2(a,b),c)

void GlPoints::pick(int x, int y){
  double nx, ny, nz;
  double fx, fy, fz;
  gluUnProject( x, y, -1, modelview, projection, (GLint *)viewport, &nx, &ny, &nz);
  gluUnProject( x, y, 1, modelview, projection, viewport, &fx, &fy, &fz);
  V3f near(nx, ny, nz);
  V3f far(fx, fy, fz);
  V3f dir(far); dir -= near;
  //we need to step maximum integer steps, to get into evry layer.
  int steps = MAX3(ABS(near.x-far.x),ABS(near.y-far.y),ABS(near.z-far.z)); 
  V3f step(dir); step /= steps;
  V3f cur(near); //start with near;
  for(int i = 0; i < steps; i++){
    cur += step;
    if((cur.x > 0) && (cur.y > 0) && (cur.z > 0) &&
       (cur.x < 256) && (cur.y < 256) && (cur.z < 255)){
      //inside the cube, let's check.
      int offset = vol.getOffset((int)cur.x, (int)cur.y, (int)cur.z);
      if(vol.vol[offset] > 3){ //found
	cursor = cur;
	say("found",cursor);
	return;
      };
    };
  };
};






