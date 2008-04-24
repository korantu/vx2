#include "gl_points.h"
#include "glfw.h"
#include "v3.h"
#include "v3tools.h"
#include <string>
#include "misc.h"

void GlPoints::set_level(float l){
  list.clear();;
  printf("Really finding surface...%f\n", l);
  vol.findSurface(list, (int )l); 
};

bool GlPoints::load(char * in){
  try{
    int cnt = loader.read(std::string(in));
    printf("Read %d bytes.\n", cnt);

    loader.read_volume(vol);

    set_level(1);
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


  //if(tw_pnt  0.5)
  //  glPointSize(pnt);
  //else
  float nicety_coefficient = 1.5;

  glPointSize(nicety_coefficient*tw_pnt*pnt);

  if(tw_pnt_smooth)
    glEnable(GL_POINT_SMOOTH);
  else
    glDisable(GL_POINT_SMOOTH);

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

void GlPoints::set_cursor(V3f c){
  cursor = c;
  tw_mri_value = (float)vol.vol[vol.getOffset((int)cursor.x, (int)cursor.y, (int)cursor.z)];
};

void GlPoints::pick(int x, int y){
  double nx, ny, nz;
  double fx, fy, fz;
  gluUnProject( x, y, -1, modelview, projection, (GLint *)viewport, &nx, &ny, &nz);
  gluUnProject( x, y, 1, modelview, projection, viewport, &fx, &fy, &fz);
  V3f near(nx, ny, nz);
  V3f far(fx, fy, fz);
  V3f dir(far); dir -= near;
  //we need to step maximum integer steps, to get into evry layer.
  int steps = (int)MAX3(ABS(near.x-far.x),ABS(near.y-far.y),ABS(near.z-far.z)); 
  V3f step(dir); step /= steps;
  V3f cur(near); //start with near;
  for(int i = 0; i < steps; i++){
    cur += step;
    if((cur.x > 0) && (cur.y > 0) && (cur.z > 0) &&
       (cur.x < 256) && (cur.y < 256) && (cur.z < 255)){
      //inside the cube, let's check.
      int offset = vol.getOffset((int)cur.x, (int)cur.y, (int)cur.z);
      if(vol.vol[offset] > tw_cursor_hit){ // hit condition tuned by gui
	set_cursor(cur+step*tw_cursor_depth); // how deep below the surface we want our cursor to go.
	//say("found",cursor)
	return;
      };
    };
  };
};

#include <AntTweakBar.h>

  TwBar *points_bar;			// Pointer to a tweak bar

int level = 3;
void TW_CALL get_level(void * value, void * UserData){
  //printf("getting level %f\n", (float)level);
  (*((float *)value))=(float)level;
};
void TW_CALL set_level(const void * value, void * UserData){
  level = *((float *)value);
  printf("setting level %f\n", level);
  ((GlPoints *)UserData)->set_level((float)level);
};


void GlPoints::gui(){
    tw_pnt=1.0;
    tw_pnt_smooth=false;
        
    /// Create a tweak bar
    points_bar = TwNewBar("Display");

    ///building bars is a different business..
     
    TwDefine(" GLOBAL help='Voxelbrain Voxel editor.' "); // Message added to the help bar.


    TwAddVarRW(points_bar, "", TW_TYPE_DOUBLE, &tw_pnt, " label='Point size' min=0.2 max=4 step=0.01 keyIncr=d keyDecr=D help='Size of the display points in relation to optimal' ");
    TwAddVarRW(points_bar, "", TW_TYPE_BOOLCPP, &tw_pnt_smooth, " label='Smooth points' keyIncr=v keyDecr=V help='Size of the display points. ' ");

    //display cursor
    TwAddSeparator(points_bar, "Position.", NULL);
    TwAddVarRW(points_bar, "", TW_TYPE_FLOAT, &cursor.x, " label='X' help='Cursor X' ");
    TwAddVarRW(points_bar, "", TW_TYPE_FLOAT, &cursor.y, " label='Y' help='Cursor Y' ");
    TwAddVarRW(points_bar, "", TW_TYPE_FLOAT, &cursor.z, " label='Z' help='Cursor Z' ");
    
    //allow to change
    TwAddSeparator(points_bar, "Operation.", NULL);
    TwAddVarCB(points_bar, "", TW_TYPE_FLOAT, ::set_level, ::get_level, this, " min=-1 max=150 step=1 label='Isovalue'");
    tw_cursor_hit = 3;
    TwAddVarRW(points_bar, "", TW_TYPE_FLOAT, &tw_cursor_hit, "  min=2 max=150 step=1 label='Hit point' help='Point where the cursor considered to have hit the surface' ");
    tw_cursor_depth = 0;
    TwAddVarRW(points_bar, "", TW_TYPE_FLOAT, &tw_cursor_depth, " min=0 max=50 step=1 label='Cursor depth' help='How deep cursor goes after an impact.' ");
    TwAddVarRO(points_bar, "", TW_TYPE_FLOAT, &tw_mri_value, " label='ValueAtCursor' help='MRI data value at the calcualtor.' ");

};











