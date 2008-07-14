#ifndef __surface_h__
#define __surface_h__


#include "v3.h"
#include "gl_points.h"

#include <vector>
#include <stdio.h>

//reading a surface from a file

using namespace std;

//data structures

/** 
 A simple surface representation 
*/
struct Surface{
  vector<V3f> n; ///Normals
  vector<V3f> v; ///Vectors (have to be of the same size as normals;  

  vector<V3i> tri; ///indices

  int n_tris; 
  
};

int surface_tris(Surface &);
void surface_points(int n, int &, int &, int &);
/// sort triangles in a direction
void surface_sort(V3f direction);

vector<Surface> * get_active_surfaces();

bool read_surface(Surface & surf, std::string name); 
bool read_surface_binary(Surface & surf, std::string name); 

/*
   Structure to control the rasterization process
*/
struct RenderingTraits{
  int dead;     /// 
  bool half;    /// Include partial occupancy voxels.
  bool inside;  /// Fill in the vilume
  bool tru;     /// Use tru bit for rasterizaion
  // RenderingTraits();
};

//bool read_surface_binary(Surface & surf, std::string name);
void rasterize_surface(Surface & surf, 
		    GlPoints & pnt,         //the point set to render 
		    const RenderingTraits &); //how to render

// 
V3f find_center_point(const Surface & surf);

void fill_surface(Surface & surf, GlPoints & pnt, const RenderingTraits);

///recursive refining function

void refine_triangle(V3f & v0, V3f & v1, V3f & v2, GlPoints & pnt, V3f n, const RenderingTraits &);


#endif // __surface_h__




