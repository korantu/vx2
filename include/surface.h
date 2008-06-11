#ifndef __surface_h__
#define __surface_h__


#include "v3.h"
#include "gl_points.h"

#include <vector>
#include <stdio.h>

//**//

//reading a surface from a file

using namespace std;

//data structures

/**
  a collection of vertices, normals and triangle indices;
 */
struct Surface{
  vector<V3f> n;
  vector<V3f> v;

  vector<int> idx; //3 per triangle

  int n_tris;
  
};

vector<Surface> * get_active_surfaces();

struct RenderingTraits{
  int dead;
  bool half;
  bool inside;
  bool tru;
  // RenderingTraits();
};


//interface
bool read_surface(Surface & surf, std::string name);

void rasterize_surface(Surface & surf, 
		    GlPoints & pnt,         //the point set to render 
		    const RenderingTraits &); //how to render

V3f find_center_point(const Surface & surf);

void fill_surface(Surface & surf, GlPoints & pnt, const RenderingTraits);

///recursive refining function

void refine_triangle(V3f & v0, V3f & v1, V3f & v2, GlPoints & pnt, V3f n, const RenderingTraits &);


#endif // __surface_h__
