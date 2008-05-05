#include "gl_points.h"
#include "glfw.h"
#include "v3.h"
#include "v3tools.h"
#include <string>
#include "misc.h"
#include "native.h"


void GlPoints::set_level(float l){
  // vol.reset();
  for(int i = 0; i < 15; i++)list[i].clear();
  //std::vector<int> list;
  //list.clear();
  printf("Really finding surface...%f\n", l);
  vol.findSurface(list[0], 0); 

  for(int i = 0 ; i < vol.max; i++)vol.depth[i]=254;
  int cur_depth = 1;
  std::vector<int> cur = list[0];
  //cur.push_back(in->getOffset(100,100,100));
  // vol.findSurface(cur, 0);

  std::vector<int> scratch;
  while(cur_depth < 15){
    for( std::vector<int>::iterator i = cur.begin(); i != cur.end(); i++)
      {
	int cur_idx = *i;
	if((vol.depth[cur_idx]> cur_depth) && (vol.vol[cur_idx]>=1) && (vol.vol[cur_idx]<=200)   ){
	  vol.depth[cur_idx] = cur_depth;
	  list[cur_depth].push_back(cur_idx);
	  for(int i = 0; i < 6; i++){
	    scratch.push_back(cur_idx+vol.neighbours[i]); //schedule for lookaround;
	  };
	};
	
      };
    cur_depth++;    //next iteration
    cur = scratch;
    scratch.clear();
  };
};



bool GlPoints::load(const char * in){
  try{
    int cnt = loader.read(std::string(in));
    printf("Read %d bytes.\n", cnt);

    vol.reset();

    loader.read_volume(vol);

    set_level(1);
    for(int ll = 1; ll < 10; ll++)
      printf("found %d points at level %d.\n", list[ll].size(), ll);
    
  }catch(const Loader::Ex & err){
    printf("Problem occured: %s\n", err.reason.c_str());
    return false;
  };
  return true;
};

bool GlPoints::save(const char * out){
  try {
    loader.write_volume(vol); //save data from volume into the loader
    loader.write(std::string(out));
  }catch(const Loader::Ex & err){
    printf("Problem occured: %s\n", err.reason.c_str());
    return false;
  };
  return true;
};;

#include "color.h"

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


  ColorMapper mapper;
  scheme_fill(mapper, scheme);

  if(tw_pnt_smooth)
    glEnable(GL_POINT_SMOOTH);
  else
    glDisable(GL_POINT_SMOOTH);

  glEnable(GL_DEPTH_TEST);
  glBegin(GL_POINTS);
  int x, y, z;
  for(std::vector<int>::iterator i = list[cur_level].begin(); i != list[cur_level].end(); i++){
    vol.getCoords(*i, x,y,z);
    float pnt_col =  vol.vol[*i]/300.0;
    mapper.map(vol.vol[*i]);
    // glColor3f(pnt_col, pnt_col, pnt_col);
    glVertex3i(x,y,z);
  };
  glEnd();
  

  //and now, for something completely different:
  int cur_depth;
  glPointSize(nicety_coefficient*1.0*pnt);
  glBegin(GL_POINTS);
  for(std::vector<int>::iterator i = vol.markers.begin(); i != vol.markers.end(); i++){
    cur_depth = vol.depth[*i]; 
    if(cur_depth < 2)continue; //do not show top layer
     vol.getCoords(*i, x,y,z);
     //float pnt_col =  vol.vol[*i]/300.0;
     glColor3f((float)cur_depth*20.0/200, 0, (200.0-cur_depth*20.0)/200.0);
     glVertex3i(x,y,z);
  };
  glEnd();
};

void GlPoints::apply(){
  for(int i = vol.getOffset(1,1,1); i <= vol.getOffset(255,255,255); i++)
    if(vol.mask[i])vol.vol[i]=0;
  vol.reset();
  set_level(1);
};

void GlPoints::set_cursor(V3f c){
  cursor = c;
  int idx = vol.getOffset((int)cursor.x, (int)cursor.y, (int)cursor.z);
  tw_mri_value = (float)vol.vol[idx];
  vol.use_tool(idx, tool);
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







