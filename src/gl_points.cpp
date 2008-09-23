#include "gl_points.h"
#define GLFW_DLL
#include "GL/glfw.h"
#include "v3.h"
#include "v3tools.h"
#include <string>
#include <algorithm>
#include "misc.h"
#include "native.h"
#include "surface.h"

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
    printf("Found %d points\n", cur.size());
    for( std::vector<int>::iterator i = cur.begin(); i != cur.end(); i++)
      {
	int cur_idx = *i;
	if( (cur_idx > 255*255*255) || (cur_idx < 1))continue;
	if((vol.depth[cur_idx] > (cur_depth)) && (!(vol.mask[cur_idx] & ZRO))){
	  vol.depth[cur_idx] = cur_depth;
	  list[cur_depth].push_back(cur_idx);
	  for(int i = 0; i < 6; i++){
	    scratch.push_back(cur_idx+vol.neighbours[i]); ///schedule for lookaround;
	  };
	};
	
      };
    cur_depth++;    //next iteration
    cur = scratch;
    scratch.clear();
  };
};

std::string default_name("brainmask.mgz");
void reset_3d_mask();

bool GlPoints::load(const char * in){

  reset_3d_mask();

  if(in)default_name = std::string(in);
  try{
    int cnt = loader.read(default_name);
    printf("Read %d bytes.\n", cnt);

    vol.reset();

    loader.read_volume(vol);

  }catch(const Loader::Ex & err){
    printf("Problem occured: %s\n", err.reason.c_str());
    return false;
  };
  return true;

  
};

void GlPoints::find_surface(){
  set_level(1);
  for(int ll = 1; ll < 10; ll++)
    printf("found %d points at level %d.\n", (int)list[ll].size(), ll);
};

bool GlPoints::save(const char * out){
  if(out)default_name = std::string(out);
  try {
    loader.write_volume(vol); //save data from volume into the loader
    loader.write(default_name);
  }catch(const Loader::Ex & err){
    printf("Problem occured: %s\n", err.reason.c_str());
    return false;
  };
  return true;
};;

#include "color.h"



  ColorMapper mapper;

struct psortable{
  V3f dir;
  int xa,  ya,  za;
  int xb,  yb,  zb;
  psortable(V3f _dir):dir(_dir){};
  
  bool operator()(int a, int b){
    FastVolume::getCoords(a, xa, ya, za);
    FastVolume::getCoords(b, xb, yb, zb);
    
    return xa*dir.x+ya*dir.y+za*dir.z < xb*dir.x+yb*dir.y+zb*dir.z;
  };
  ~psortable(){};
};


void GlPoints::set_scheme(int sch){
  scheme_fill(mapper, sch);
};

void GlPoints::set_projection(){
  glDisable (GL_BLEND); 

  glDisable(GL_LIGHTING);

  //draw to get the idea of the whole thing.
  /* DISABLE LARGE CROSS FOR NOW
    glBegin(GL_LINES);
  glColor3f(1,0,0); glVertex3f(1,0,0); glVertex3f(-1,0,0);
  glColor3f(0,1,0); glVertex3f(0,1,0); glVertex3f(0,-1,0);
  glColor3f(0,0,1); glVertex3f(0,0,1); glVertex3f(0,0,-1);
  glEnd();
  */
  glScalef(2.0/256.0, 2.0/256.0, 2.0/256.0);
  glTranslatef(-128.0, -128.0, -128.0);


  //ok, the modelview transform is complete here, dumping it for lookup:

  glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
  glGetDoublev( GL_PROJECTION_MATRIX, projection );
  glGetIntegerv( GL_VIEWPORT, (GLint *)viewport );
};

std::vector<int> the_markers;
std::vector<int> the_marked;


//now, where do we call this thing from.....
//adding more connections really harms...

void GlPoints::update(){
  the_markers = vol.markers; //just replace with the current seeds.
  the_marked.clear(); //remove whatever marked volume we had
  for(int i = 1; i < 256*256*256;i++){
    if(vol.mask[i] & MSK){ //then see if it is border... ok?
    
      for(int j = 0; j < 6; j++){
	if(!(MASK & vol.mask[i+FastVolume::neighbours[j]])){
	  //if we have a neighbour outside, then add to the list; 
	  the_marked.push_back(i);
	  j = 42; //make fucking sure get out of here; 
	  //break; //would also work.
	};
      };
      
    };

  };
  printf("seeds: %d, passive border: %d\n", 
	 (int)the_markers.size(), 
	 (int)the_marked.size());

};

void init_lighting(V3f pos) 
{
  pos += V3f(pos.y/3, pos.z/3, pos.x/3);
  //   GLfloat mat_specular[] = { 1.0, 0.0, 0.0, 1.0 };
  //  GLfloat mat_shininess[] = { 2.0 };
   GLfloat light_position[] = { pos.x, pos.y, pos.z, 0.0 };
   GLfloat ambient_color[] = { 1,0,0, 1};
   GLfloat diffuse_color[] = { 0,1,1,1};
   GLfloat specular_color[] = {0,1,1,1};
   


   glShadeModel (GL_SMOOTH);

   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_color);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_color);
   glLightfv(GL_LIGHT0, GL_SPECULAR, specular_color);

   glMaterialfv ( GL_FRONT_AND_BACK, GL_SPECULAR, specular_color ) ;
   glMaterialfv ( GL_FRONT_AND_BACK, GL_EMISSION, ambient_color); 

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_DEPTH_TEST);
   glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;

}


struct compare_tris{
  V3f & axis;
  const Surface & surface;
  
  compare_tris(V3f & _axis, const Surface & _surface): axis(_axis), surface(_surface){
  }; 

  int operator() (const V3i & a, const V3i b) const{
    return surface.v[a[0]].dot(axis) < surface.v[b[0]].dot(axis);
  };
};


void GlPoints::draw(V3f zaxis){
  int cursor_size = 10;
  
  int x, y, z; //tmp



  //if(tw_pnt  0.5)
  //  glPointSize(pnt);
  //else
  float nicety_coefficient = 1.5;

  glPointSize((float)(nicety_coefficient*tw_pnt*pnt));




  //rendering active area first
  int cur_depth;
  glPointSize((float)(nicety_coefficient*1.0f*pnt));
  glBegin(GL_POINTS);

  //markers
  for(std::vector<int>::iterator i = the_markers.begin(); i != the_markers.end(); i++){
    cur_depth = vol.depth[*i]; 
    if(cur_depth < 2)continue; //do not show top layer
     vol.getCoords(*i, x,y,z);
     //float pnt_col =  vol.vol[*i]/300.0;
     glColor3f(cur_depth*20.0f/200.0f, 0.0f, ((200.0f-cur_depth*20.0f)/200.0f));
     glVertex3i(x,y,z);
  };
/*
  //usual stuff
  for(std::vector<int>::iterator i = the_marked.begin(); i != the_marked.end(); i++){
    cur_depth = vol.depth[*i]; 
    if(cur_depth < 2)continue; //do not show top layer
     vol.getCoords(*i, x,y,z);
     //float pnt_col =  vol.vol[*i]/300.0;
     glColor3f(cur_depth*20.0f/200.0f, (200.0f-cur_depth*20.0f)/200.0f, 0.0f);
     glVertex3i(x,y,z);
  };
*/
  glEnd();

  //drawing plane
  glDisable(GL_DEPTH_TEST);
  glBegin(GL_POINTS);
  for(std::vector<int>::iterator i = vol.plane.begin(); i != vol.plane.end(); i++){
    cur_depth = vol.vol[*i]; 
     vol.getCoords(*i, x,y,z);
     //float pnt_col =  vol.vol[*i]/300.0;
     glColor3f(cur_depth, cur_depth, cur_depth);
     glVertex3i(x,y,z);
  };
  glEnd();
  glEnable(GL_DEPTH_TEST);

  ///draw cursor
   glBegin(GL_LINES);
  glColor3f(1,0,0); glVertex3f(cursor.x+cursor_size,cursor.y,cursor.z); glVertex3f(cursor.x-cursor_size,cursor.y,cursor.z);
  glColor3f(0,1,0); glVertex3f(cursor.x,cursor.y+cursor_size,cursor.z); glVertex3f(cursor.x,cursor.y-cursor_size,cursor.z);
  glColor3f(0,0,1); glVertex3f(cursor.x,cursor.y,cursor.z+cursor_size); glVertex3f(cursor.x,cursor.y,cursor.z-cursor_size);
  glEnd();

  Surface * surf = get_active_surfaces();

  //glPointSize(1.0);

  // init_lighting(zaxis+V3f(0.3, 0.3, 0.3));
  
  //I am not able to set up a sane lighting in OpenGL
  //wtf?

  //sorting.
  /*
  glBegin(GL_QUADS);
  glColor3f(0.4,0.8,0.1);
  glVertex3f(0,0,125);
  glVertex3f(0,255,125);
  glVertex3f(255,255,125);
  glVertex3f(255,0,125);
  glEnd();
  */
  glEnable (GL_BLEND); 
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  glBegin(GL_TRIANGLES);

    //each surface surf

    //    compare_tris criterion(zaxis, *surf);
    //    std::sort(surf->tri.begin(), surf->tri.end(), criterion);

    for(vector<V3i>::const_iterator tri = surf->tri.begin(); tri != surf->tri.end(); tri++){
      //each vertex
      for(int corner = 0; corner < 3; corner++){
	int idx = (*tri)[corner];
	float a = zaxis.dot(surf->n[idx]);
	a = a*a*a*a;
	//a *=;
	if(a < 0.3)a=0.3f;
	//float transparency = smooth_bell((surf->v[idx][2]-125)/5);
	//transparency *= transparency;
	//transparency *= transparency;
        //glColor4f(a/4,a/3,a*1.5, 0.1+transparency);

	//      	glColor3f(a/4.0f*surf->c[idx].x,a/3.0f*surf->c[idx].y,a*1.5f*surf->c[idx].z);
	glColor3f(surf->c[idx].x,surf->c[idx].y,surf->c[idx].z);
        glVertex3f( surf->v[idx].x , surf->v[idx].y , surf->v[idx].z );
      };
    };    
  
  glEnd();

  glDisable(GL_LIGHTING);

  if(tw_pnt_smooth)
    glEnable(GL_POINT_SMOOTH);
  else
    glDisable(GL_POINT_SMOOTH);

  glEnable(GL_DEPTH_TEST);

  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glDepthMask(GL_TRUE);
  
  glPushMatrix();
  glTranslatef(-zaxis.x*5, -zaxis.y*5, -zaxis.z*5);
  //depth buffer
  glBegin(GL_POINTS);
  for(std::vector<int>::iterator i = list[cur_level].begin(); i != list[cur_level].end(); i++){
    vol.getCoords(*i, x,y,z);
        glVertex3i(x,y,z);
  };
  glEnd();
  glPopMatrix();
  

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_FALSE);

  std::sort(list[cur_level].begin(), list[cur_level].end(), psortable(zaxis));

  int r,g,b;
  

  glBegin(GL_POINTS);
  for(std::vector<int>::iterator i = list[cur_level].begin(); i != list[cur_level].end(); i++){
    vol.getCoords(*i, x,y,z);
    int t_col = vol.vol[*i];
    if(t_col < 1)t_col=1;
    if(t_col > 254)t_col=254;
    mapper.map(r,g,b ,t_col);
    glColor4f(r / 256.0f, g / 256.0f, b / 256.0f, tw_transparency);
    glVertex3i(x,y,z);
  };
  glEnd();

  
  glDepthMask(GL_TRUE);
  glDisable(GL_DEPTH_TEST);

  glBegin(GL_POINTS);
  for(std::vector<int>::iterator i = vol.plane.begin(); i != vol.plane.end(); i++){
    vol.getCoords(*i, x,y,z);
    int t_col = vol.vol[*i];
    if(t_col < 1)t_col=1;
    if(t_col > 254)t_col=254;
    mapper.map(r,g,b ,t_col);
    glColor3f(r / 256.0f, g / 256.0f, b / 256.0f);
    glVertex3i(x,y,z);
  };
  glEnd();
  glEnable(GL_DEPTH_TEST);


};

//storage

struct backup_point{
  int pos;
  int value;
  int mask;
};

std::vector<backup_point> backup;
std::vector<int> old_mask;
std::vector<int> old_values;

void reset_3d_mask(){
  backup.clear();
  old_mask.clear();
  old_values.clear();
};

void GlPoints::apply(){

  //we don't care if the allocation fails; just dont' use it if it is null
  
  if(backup.size() == 0){
    char * truth_backup;
    truth_backup  = new char[256*256*256];
    for(int i = vol.getOffset(1,1,1); i <= vol.getOffset(255,255,255); i++){
      //saving truth
      if(truth_backup)truth_backup[i] = (vol.mask[i] & TRU)?1:0;
      if(vol.mask[i] & (MASK)){
	backup_point pnt = {i, vol.vol[i], vol.mask[i]};
	backup.push_back(pnt);
	vol.vol[i] = 0;
	vol.mask[i] |= ZRO; //it is a zero
	vol.mask[i] -= (vol.mask[i] & MASK); //and not a mask.
      };
    };
    printf("Saved %d points to be restored later...\n", (int)backup.size());
    vol.reset();
    //reverting truth if possible;
    if(truth_backup){
      for(int i = vol.getOffset(1,1,1); i <= vol.getOffset(255,255,255); i++){
	vol.mask[i] |= truth_backup[i]?TRU:0;
      };
      delete[] truth_backup; //no need already;
    };
  }else{ //got a backup to restore.
    for(std::vector<backup_point>::iterator p = backup.begin(); p != backup.end(); p++){
      backup_point pnt = *p;
      vol.vol[pnt.pos] = pnt.value;
      vol.mask[pnt.pos] = pnt.mask;
      if(pnt.mask & BDR)vol.markers.push_back(pnt.pos);
    };
    printf("Allegedly restored %d points. \n", (int)backup.size());
    backup.clear(); //next time apply, not restore.
  };
  //set_level(1);
  find_surface();
};

void GlPoints::set_cursor(V3f c){
  cursor = c;
  int idx = vol.getOffset((int)cursor.x, (int)cursor.y, (int)cursor.z);
  tw_mri_value = (float)vol.vol[idx];
  vol.use_tool(idx, tool, tool_size);
};

void GlPoints::pick(int x, int y){
  double nx, ny, nz;
  double fx, fy, fz;
  gluUnProject( x, y, -1, modelview, projection, (GLint *)viewport, &nx, &ny, &nz);
  gluUnProject( x, y, 1, modelview, projection, viewport, &fx, &fy, &fz);
  V3f near((float)nx, (float)ny, (float)nz);
  V3f far((float)fx, (float)fy, (float)fz);
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







