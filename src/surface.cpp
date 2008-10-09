/**
A file for surface manipulation;
//loading
//generating from voxels
//rendering
 */

#include "surface.h"
#include "misc.h"
#include "io.h"


/*** global surface ***/
Surface __surfaces;

//accessor. making sure the actual storage is only defined once.

Surface * get_active_surfaces(){
  return & __surfaces;
};

void clear(Surface & in){
  in.v.clear(); in.n.clear(); in.tri.clear(); 
  in.seeds.clear();
};


bool ReadPialHeader(Io & data, int * vertices, int * triangles){

  //obviously wrong;
  if(data.size() < 5)return false;

  //check signature
  const char signature[3] = {0xff, 0xff, 0xfe};
  for (int i = 0 ; i < 3; ++i) {
    char item; data.GetChar(&item);
    if (signature[i] != item) return false;
  };

  //look for 0x0a 0x0a
  while (data.valid()) {
    char oxa_wannabe;
    data.GetChar(&oxa_wannabe);
    if (oxa_wannabe == 0x0a) { //check if another one is behind; if so - done.
      data.GetChar(&oxa_wannabe);
      if (oxa_wannabe == 0x0a) {
	break;
      };
    };
  };

  data.GetInt(vertices).GetInt(triangles);
  
  return data.valid();
  
};

bool read_surface_binary(Surface & surf, std::string name){
  int vertices_number;     //for the number of vertices
  int triangles_number;       //for the number of triangles
  bool result = true;

  int start_index = surf.v.size();

  Io data(ReadFile(name));

  if(!ReadPialHeader(data, &vertices_number, &triangles_number))return false;

  V3f seed = V3f(0,0,0);
    //    printf("Expecting %d points and %d triangles.\n", points, tris);
    
  //reading points and pushing normals
  for(int i = 0; i < vertices_number; i++){
    V3f in;
  //int dummy;
    data.GetFloat(&in.x).GetFloat(&in.y).GetFloat(&in.z);
    in = V3f(-in.x, +in.z, +in.y);
    in+=V3f(128, 127, 128);
    //  printf("%f, %f, %f\n", in.x, in.y, in.z);
    surf.v.push_back(in);
    seed += in;
    surf.n.push_back(V3f(0,0,0));
  };

  seed /= vertices_number;
  surf.seeds.push_back(seed);

  for(int i = 0; i < triangles_number; i++){
    int a, b, c;//, zero; 
    data.GetInt(&a).GetInt(&b).GetInt(&c);
    a += start_index;
    b += start_index;
    c += start_index;
    surf.tri.push_back(V3i(a,b,c));
    //    if(i < 3)printf("((%d %d %d))\n", a, b, c);
  
    V3f n; n.cross(surf.v[b]-surf.v[a], surf.v[c]-surf.v[a]);
    n /= -n.length(); //normal - outside

    surf.n[a] = surf.n[a] + n; 
    surf.n[b] = surf.n[b] + n; 
    surf.n[c] = surf.n[c] + n; 
  };

  if(!data.valid())return false;

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

      int cur = pnt.vol.getOffset((int)floor(v[i].x), 
				  (int)floor(v[i].y), 
				  (int)floor(v[i].z));
           
      if(cur < 0 || cur > 255*255*255)return;
      if(!(pnt.vol.mask[cur] & TRU)){
	pnt.vol.mask[cur] |= TRU;
	if(good)pnt.vol.mask[cur] |= (t.tru?TRU:MSK);
	//	if((pnt.vol.mask[cur] & MASK) 
	//  && (pnt.vol.vol[cur] > 35))dead++; //count intersecting pixels
      };
    };
  
  //check if the refinement is needed, i.e. tris are too big ( > 0.5 voxels )
  

  if(((v0-v1).length2() > 0.1 ||
      (v1-v2).length2() > 0.1 ||
      (v2-v0).length2() > 0.1)){
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

struct point_property{

  enum {
    CONFIGURATION = 0,
    DEPTH,
    CURVATURE,
    GRADIENT,
    INTENSITY,
    INTENSITY_PER_GRADIENT,
    LAST_PARAM
  };

  float param[LAST_PARAM+1];

  //getters  
  inline float depth() const {return param[DEPTH];};
  inline float configuration() const {return param[CONFIGURATION];};
  inline float curvature() const {return param[CURVATURE];};
  inline float gradient() const {return param[GRADIENT];};
  inline float intensity() const {return param[INTENSITY];};
  inline float intensity_per_gradient() const {return param[INTENSITY_PER_GRADIENT];};

  //setters
  inline void depth(float in) {param[DEPTH] = in;};
  inline void configuration(float in) { param[CONFIGURATION]=in;};
  inline void curvature(float in)  { param[CURVATURE]=in;};
  inline void gradient(float in)  {param[GRADIENT]=in;};
  inline void intensity(float in)  {param[INTENSITY]=in;};
};

struct point_set_property{
  point_property min;
  point_property max;
  
  static const float EPSILON;// = 0.00001; /// epsilon for properties

  //find min and max
  point_set_property(const vector<point_property> & in){
    for(int i = 0; i < point_property::LAST_PARAM; i++){
      min.param[i] = 1000.0f;
      max.param[i] = -1000.0f;
    };

    for(vector<point_property>::const_iterator cur_property = in.begin(); 
	cur_property != in.end(); 
	cur_property++){
      for(int i = 0; i < point_property::LAST_PARAM; i++){
	if(cur_property->param[i] < min.param[i])min.param[i] = cur_property->param[i];
	if(cur_property->param[i] > max.param[i])max.param[i] = cur_property->param[i];	
      };
    };
    //debug
    for(int i = 0; i < point_property::LAST_PARAM; i++){
      printf("param %d is from %f to %f\n", i, min.param[i], max.param[i]);
    };   
  };
  
  //scale all parametes in the range [0..1]
  void scale(point_property & in){
    for(int i = 0; i < point_property::LAST_PARAM; i++){
      if(max.param[i] - min.param[i] > EPSILON){
      in.param[i] = (in.param[i]-min.param[i])/(max.param[i]-min.param[i]);
      };
    };
  };
};

const float point_set_property::EPSILON = 0.00001f;

//returns color from point property and point set property
V3f analyze_point(const point_property & in, point_set_property & t){
  
  V3f background(0.7f, 0.7f, 0.7f);
  if(in.depth() > 0.9f) return background;


  //  V3f c(in.configuration(), in.gradient(), in.intensity());
  float alert = (in.configuration() < 0.5f || (in.configuration() > 0.5f && in.configuration() < 0.75f))?1.0f:0.0f; 
  V3f c(alert, 0.3f*in.curvature(), (in.intensity_per_gradient())/(1.0f+alert*100.0f));
   return c*(1.0f-in.depth())+ background*in.depth();
};

void analyze_surface(Surface & surf,
		     GlPoints & pnt){
  surf.c.clear(); // remove all previous analysis
  
  V3f n, v;
  float depth;
  float v0, vup, vdown;
  vector<point_property> points;


  //each vertex, determining configuration and gradient first.
  for(size_t i = 0; i < surf.v.size(); i++){
    v = surf.v[i];
    n = surf.n[i];
    n /= n.length();

    point_property cur_point;

    //can do the lookup directly, as well. no hits in quality or performance
    depth = interpolate_lookup(v, pnt, LOOKUP_DEPTH);
	if(depth > 255.0f)depth=0.0f;
    depth = depth/10.0f;
    cur_point.param[point_property::DEPTH] = depth;
    if(depth > 1.0){
      //do nothing, really.
      depth = 1.1f;
    }else{ //ok, shallow, makes sense to process
    v0 =   interpolate_lookup(v, pnt, LOOKUP_VALUE);
    vup =   interpolate_lookup(v+n, pnt, LOOKUP_VALUE);
    vdown =   interpolate_lookup(v-n, pnt, LOOKUP_VALUE);

	vup-=v0*0.15f;
	vdown+=v0*0.15f;
    
    if((vup < v0) && (v0 < vdown))cur_point.configuration(0.0f);
    if((vup > v0) && (v0 < vdown))cur_point.configuration(1.0f);
    if((vup < v0) && (v0 > vdown))cur_point.configuration(2.0f);
    if((vup > v0) && (v0 > vdown))cur_point.configuration(3.0f);

    cur_point.param[point_property::GRADIENT] = (fabs(vup-v0)+fabs(vdown-v0));
    cur_point.param[point_property::INTENSITY] = v0;
    cur_point.param[point_property::CURVATURE] = 0.0f;
    cur_point.param[point_property::INTENSITY_PER_GRADIENT] = cur_point.intensity()/cur_point.gradient();

    };

    points.push_back(cur_point);
    //    c = cols[determinator]/diff*intensity*(1.0-depth)+V3f(0.7f, 0.7f, 0.7f)*depth;
  }; //for(int i = 0; i < surf.v.size(); i++)
  
  //curvature
  for(size_t i = 0; i < surf.tri.size(); i++){
  
    V3i ctri = surf.tri[i];

    if(points[ctri.x].depth() > 1)continue; //do not bother with deep points
    V3f n1 = surf.n[ctri.x];
    V3f n2 = surf.n[ctri.y];
    V3f n3 = surf.n[ctri.z];
    float curv = (n1-n2).length2()+(n2-n3).length2()+(n1-n3).length2();
    //setting maximum curvature to the tri's corners
    points[ctri.x].curvature((points[ctri.x].curvature()<curv)?curv:points[ctri.x].curvature());
    points[ctri.y].curvature((points[ctri.y].curvature()<curv)?curv:points[ctri.y].curvature());
    points[ctri.z].curvature((points[ctri.z].curvature()<curv)?curv:points[ctri.z].curvature());
  };//for(int i = 0; i < .size(); i++)
  
  
  point_set_property t(points);

  for(vector<point_property>::iterator i = points.begin(); i != points.end(); i++){
    surf.c.push_back(analyze_point(*i, t)); 
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
  
   //pnt.vol.propagate(1000, 0, 1000, 400); //propagate everywhere from centerpoint  

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
//* change color of already marked regions*//

void unmark(Surface & in, V3f where, float radius){
	for(int i = 0; i < in.v.size(); i++){
	  float dist = (in.v[i] - where).length();
	  if(dist < radius){
			in.c[i].x = 0.0;
		};
	};
};

