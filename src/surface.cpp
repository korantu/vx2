/**
A file for surface manipulation;
//loading
//generating from voxels
//rendering
 */

#include "surface.h"

vector<Surface> __surfaces;

//accessor. making sure the actual storage is only defined once.
vector<Surface> * get_active_surfaces(){
  return & __surfaces;
};

int read_int(int fd){
  unsigned char buf[4];
  int res;
  unsigned char * pnt = (unsigned char *)&res;
  if(4 != read(fd, (void *) buf, 4))throw "Unable to read integer";
  for(int i = 0; i < 4; i++){
    pnt[i] = buf[3-i];
  };
  return res;
};

float read_float(int fd){
  unsigned char buf[4];
  float res;
  unsigned char * pnt = (unsigned char *)&res;
  if(4 != read(fd, (void *) buf, 4))throw "Unable to read float";
  for(int i = 0; i < 4; i++){
    pnt[i] = buf[3-i];
  };
  return res;
};

//TODO: unify read_surface_binary

bool read_surface_binary(Surface & surf, std::string name){
  int points;     //for the number of points
  int tris;       //for the number of triangles
  unsigned char buf[1000]; //for data
  bool result = true;
  
  int file = open(name.c_str(), O_RDONLY, 0);
  if(file < 0) return false; //cannot open file.

  try {
    buf[0]=0; read(file, (void *)(buf), 3);
    if(buf[0]==0xff & buf[1]==0xff & buf[2]==0xfe ){
      printf("Format indicator matches.");
    }else{
      throw "Incorrect file";
    };
    
    //searching for 0x0a 0x0a
    int check = 0;
    
    while((check = read(file, (void *) buf, 1)) > 0){
      if(buf[0] == 0x0a){ //check if another one is behind; if so - done.
	read(file, (void *) buf, 1);
	  if(buf[0] == 0x0a)break;
      };
    };
    if(check == 0)throw "Error while looking for the end of signature";
    
    points = read_int(file);
    tris = read_int(file);
    
    printf("Expecting %d points and %d triangles.\n", points, tris);
    
  //reading points and pushing normals
  for(int i = 0; i < points; i++){
    V3f in;
    int dummy;
    in.x = read_float(file);
    in.y = read_float(file);
    in.z = read_float(file);
    in = V3f(-in.x, +in.z, +in.y);
    in+=V3f(128, 127, 128);
    printf("%f, %f, %f\n", in.x, in.y, in.z);
    surf.v.push_back(in);
    surf.n.push_back(V3f(0,0,0));
  };

  for(int i = 0; i < tris; i++){
    int a, b, c, zero; 
    a = 0;
    b = 0;
    c = 0;
    a = read_int(file);
    b = read_int(file);
    c = read_int(file);
    surf.tri.push_back(V3i(a,b,c));
    if(i < 3)printf("((%d %d %d))\n", a, b, c);
  
    V3f n; n.cross(surf.v[b]-surf.v[a], surf.v[c]-surf.v[a]);
    n /= -n.length(); //normal - outside

    surf.n[a] = surf.n[a] + n; 
    surf.n[b] = surf.n[b] + n; 
    surf.n[c] = surf.n[c] + n; 
  };



  } catch (const char * a){
    printf("Problem occured: %s\n", a);
    result = false;
  };

  close(file);

  for(unsigned int i = 0; i < surf.n.size(); i++){
    V3f n = surf.n[i];
    n /= n.length();
    surf.n[i] = n;
  };

  printf("stor size is:%d\n", (int)surf.v.size());
  
  return true;

};

bool read_surface(Surface & surf, std::string name){
  int points;     //for the number of points
  int tris;       //for the number of triangles
  char buf[1000]; //for filename

  
  FILE * f = fopen(name.c_str(), "ro");
  
  try {

  if(f == NULL){
    printf("Cannot open surface file;\n");
    return false;
  };

  fgets( buf, 1000, f); //first line, don't care
  if(feof(f)) throw "eof";
  fscanf(f, "%d %d\n", &points, &tris);
  if(feof(f)) throw "eof";
  printf("Sanity checking\n");
  if(points < 0 || tris < 0)return false;
 

  printf("%d pints, %d triangles, what gives?", points, tris);

  //reading points and pushing normals
  for(int i = 0; i < points; i++){
    V3f in;
    int dummy;
    fscanf(f, "%f  %f  %f  %d\n", &in.x, &in.y, &in.z, &dummy);
    if(feof(f)) throw "eof";
    in = V3f(-in.x, +in.z, +in.y);
    in+=V3f(128, 127, 128);
    surf.v.push_back(in);
    surf.n.push_back(V3f(0,0,0));
  };

  for(int i = 0; i < tris; i++){
    int a, b, c, zero; 
    a = 0;
    b = 0;
    c = 0;
    fscanf(f, "%d %d %d %d\n", &a, &b, &c, &zero);
    if(feof(f)) throw "eof";
  surf.tri.push_back(V3i(a,b,c));
    if(i < 3)printf("((%d %d %d))\n", a, b, c);
  
    V3f n; n.cross(surf.v[b]-surf.v[a], surf.v[c]-surf.v[a]);
    n /= -n.length(); //normal - outside

    surf.n[a] = surf.n[a] + n; 
    surf.n[b] = surf.n[b] + n; 
    surf.n[c] = surf.n[c] + n; 
  };

  }catch(const char *){
    printf("Problem reading surface.\n");
  };

  fclose(f);

  for(unsigned int i = 0; i < surf.n.size(); i++){
    V3f n = surf.n[i];
    n /= n.length();
    surf.n[i] = n;
  };

  printf("stor size is:%d\n", (int)surf.v.size());
  
  return true;
};


V3f find_center_point(const Surface & surf)
{
  V3f res(0,0,0);
  for(vector<V3f>::const_iterator i = surf.v.begin(); i != surf.v.end(); i++)
    res += (*i);
  printf("total.result: %f,%f,%f\n", res.x, res.y, res.z);

  res /= (float)surf.v.size();

  return res;
};


/*
Recursively render a triangle;
if the triangle is larger than a pixel, split it up into 
4 smaller triangles and try again.
 */
void refine_triangle(V3f & v0, V3f & v1, V3f & v2, GlPoints & pnt, V3f n, const RenderingTraits & t)
{
  V3f v[3] = {v0, v1, v2};
  bool good; //mark what voxels are inside as good

  for(int i = 0; i < 3; i++){ ///each vertex
      
    /*     V3f vec((vtx&1)?ceil(v[i].x):floor(v[i].x), //check neighbouring voxels
	      (vtx&2)?ceil(v[i].y):floor(v[i].y),
	      (vtx&4)?ceil(v[i].z):floor(v[i].z));
    */
    
    V3f vec(v[i].x, v[i].y, v[i].z);
      V3f dir = vec-v[i];
      good = (dir.dot(n)<0);
      if(!t.half)good = true;

      int cur = pnt.vol.getOffset((int)floor(v[i].x), (int)floor(v[i].y), (int)floor(v[i].z));
           
      if(cur < 0 || cur > 255*255*255)return;
      if(!(pnt.vol.mask[cur] & TRU)){
	pnt.vol.mask[cur] |= TRU;
	if(good)pnt.vol.mask[cur] |= (t.tru?TRU:MSK);
	//	if((pnt.vol.mask[cur] & MASK) 
	//  && (pnt.vol.vol[cur] > 35))dead++; //count intersecting pixels
      };
    };
  
  //check if the refinement is needed, i.e. tris are too big ( > 0.5 voxels )
  

  if(((v0-v1).length2() > 0.2 ||
      (v1-v2).length2() > 0.2 ||
      (v2-v0).length2() > 0.2)){
    V3f o0 = (v0+v1)/2;
    V3f o1 = (v1+v2)/2;
    V3f o2 = (v2+v0)/2;
   
    //if triangles are too big, render smaller ones...
    refine_triangle( v0, o0, o2, pnt, n, t);
    refine_triangle( o0, v1, o1, pnt, n, t);
    refine_triangle( o1, v2, o2, pnt, n, t);
    refine_triangle( o0, o1, o2, pnt, n, t);
     
}; 

  
};

// weighted average of all corners 
enum lookup_type {
  LOOKUP_DEPTH,
  LOOKUP_VALUE
};

inline float interpolate_lookup(V3f v, GlPoints & pnt, lookup_type what){
  //check all the sides
  const int X = 1; //bit fields
  const int Y = 2;
  const int Z = 4;
  const float epsilon = 0.01f;

  float total_distance = 0.0f;
  float total = 0;

  //each corner
  for(int i = 0 ; i < 8; i++){
    float corner_value = 0.0f;
    //exact corner position
    V3f corner((i & X)?ceilf(v.x):floorf(v.x),
	     (i & Y)?ceilf(v.y):floorf(v.y),
	     (i & Z)?ceilf(v.z):floorf(v.z));
    
    switch(what){
    case LOOKUP_DEPTH:
      corner_value =  (float)pnt.vol.depth[pnt.vol.getOffset(
								(int)corner.x, 
								(int)corner.y, 
								(int)corner.z)];
      break;
    case LOOKUP_VALUE:
      corner_value =  (float)pnt.vol.vol[pnt.vol.getOffset(
								(int)corner.x, 
								(int)corner.y, 
								(int)corner.z)];
      break;
    };      
    //distance between the corner and the point
    float l = (v - corner).length2();
    if(l < epsilon)return corner_value;

    total += 1.0f/l*corner_value;
    total_distance += 1.0f/l;
  };

  return total / total_distance;
};

void analyze_surface(Surface & surf,
		     GlPoints & pnt){
  surf.c.clear(); // remove all previous analysis
  
  V3f n, v;
  float depth;
  float v0, vup, vdown;

  //each vertex
  for(int i = 0; i < surf.v.size(); i++){
    v = surf.v[i];
    n = surf.n[i];
    n /= n.length();
    
    V3f c(0.7f, 0.7f, 0.7f);
    

    //can do the lookup directly, as well. no hits in quality or performance
    depth = interpolate_lookup(v, pnt, LOOKUP_DEPTH);
    if(depth > 10.0) depth = 10.0f; depth = depth/10.0f;

    v0 =   interpolate_lookup(v, pnt, LOOKUP_VALUE);
    vup =   interpolate_lookup(v+n*2, pnt, LOOKUP_VALUE);
    vdown =   interpolate_lookup(v-n*2, pnt, LOOKUP_VALUE);
    int determinator = ((vup < v0)?0:2)+((v0 < vdown)?0:1);
    float difference = fabs(v0-vup)+fabs(v0-vdown);
    V3f cols[4] = {V3f(0.0, 1.0, 0.0), V3f(0.0, 0.0, 1.0),
                V3f(1.0, 0.0, 0.0), V3f(0.1, 1.0, 1.0)};

    float diff = fabs(vup-v0)+fabs(vdown-v0);
    //diff /= 20;
    c = cols[determinator]*diff*(1.0-depth)+V3f(0.7f, 0.7f, 0.7f)*depth;
    
    surf.c.push_back(c); 
  };

};

void rasterize_surface(Surface & surf, 
		    GlPoints & pnt,         //the point set to render 
		       const RenderingTraits & t) //how to render
{
  //ok, now all the correct points in tri.
  //read tris now
  V3f center;
  //V3f m[3];
  //  std::vector<int> tristor; //triangle storage

  if(surf.v.size() < 3)return; ///not enough triangles

  //loop trough every triangle and refine it.
  for(vector<V3i>::const_iterator i = surf.tri.begin(); 
      i != surf.tri.end(); i++){

    V3f a(surf.v[i->x]);
    V3f b(surf.v[i->y]);
    V3f c(surf.v[i->z]);
    /*
    printf("Rendering...\n");
    for(int h = 0; h < 3; h++)
      printf("a:%f, b:%f, c:%f\n", m[h].x, m[h].y, m[h].z);
    */
    refine_triangle(a, b, c, pnt,  surf.n[i->x], t); 
  };
  
  if(t.inside){
  printf("Trying to fill it\n");
  center = find_center_point(surf); //this is the average;
  printf("The seed is %f %f %f; %d in total;  so what? \n", center.x, center.y, center.z, (int)surf.v.size());
   int cur = pnt.vol.getOffset((int)center.x, (int)center.y, (int)center.z);
   pnt.vol.mask[cur] |= BDR;
   pnt.vol.markers.push_back(cur);
  
  pnt.vol.propagate(1000, 0, 1000, 400); //propagate everywhere from centerpoint  

  //remove truth mask as not propagating anymore
  
  if(!t.tru){
  for(int i = 0; i < 256*256*256; i++)
    pnt.vol.mask[i] -= (pnt.vol.mask[i] & TRU);
  }else{
    for(int i = 0; i < 256*256*256; i++){
      if(pnt.vol.mask[i] & MASK)pnt.vol.mask[i] |= TRU;    
      pnt.vol.mask[i] -= pnt.vol.mask[i] & MASK;
    };
  };
};
  
};
//**//


