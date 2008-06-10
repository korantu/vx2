/**
A file for manipulation with surfaces;
//loading
//generating from voxels
//rendering
 */

#include "surface.h"

/*RenderingTraits::RenderingTraits():dead(0),
				   half(false), 
				   inside(true), 
				   tru(false)
{
  //nothing here
};
*/

bool read_surface(Surface & surf, std::string name){
  int points;     //for the number of points
  int tris;       //for the number of triangles
  char buf[1000]; //for filename

  FILE * f = fopen(name.c_str(), "ro");
  if(f == NULL){
    printf("Cannot open surface file;\n");
    return false;
  };

  fgets( buf, 1000, f); //first line, don't care
  fscanf(f, "%d %d\n", &points, &tris);
  printf("Sanity checking\n");
  if(points < 0 || tris < 0)return false;
  //640k should be enough for everyone.
  if(points > 1000000 || tris > 1000000) return false; 

  printf("%d pints, %d triangles, what gives?", points, tris);

  //reading points and pushing normals
  for(int i = 0; i < points; i++){
    V3f in;
    int dummy;
    fscanf(f, "%f  %f  %f  %d\n", &in.x, &in.y, &in.z, &dummy);
    in = V3f(-in.x, -in.z, +in.y);
    in+=V3f(128, 128, 128);
    surf.v.push_back(in);
    surf.n.push_back(V3f(0,0,0));
  };

  for(int i = 0; i < tris; i++){
    int a, b, c, zero; 
    //int m[3];
    fscanf(f, "%d %d %d %d\n", &a, &b, &c, &zero);
    //Life sucks...and the answer is in fact 43.
    surf.idx.push_back(a);
    surf.idx.push_back(b);
    surf.idx.push_back(c);
    if(i < 3)printf("((%d %d %d))\n", a, b, c);
  
    V3f n; n.cross(surf.v[b]-surf.v[a], surf.v[c]-surf.v[a]);
    n /= -n.length(); //normal - outside

    surf.n[a] = surf.n[a] + n; 
    surf.n[b] = surf.n[b] + n; 
    surf.n[c] = surf.n[c] + n; 
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

  res /= surf.v.size();

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
    
    V3f vec((int)v[i].x, (int)v[i].y, (int)v[i].z);
      V3f dir = vec-v[i];
      good = (dir.dot(n)<0);
      if(!t.half)good = true;

      int cur = pnt.vol.getOffset(floor(v[i].x), floor(v[i].y), floor(v[i].z));
           
      if(cur < 0 || cur > 255*255*255)return;
      if(!(pnt.vol.mask[cur] & TRU)){
	pnt.vol.mask[cur] |= TRU;
	if(good)pnt.vol.mask[cur] |= (t.tru?TRU:MSK);
	//	if((pnt.vol.mask[cur] & MASK) 
	//  && (pnt.vol.vol[cur] > 35))dead++; //count intersecting pixels
      };
    };
  
  //check if the refinement is needed, i.e. tris are too big ( > 0.5 voxels )
  
  #define FUCK_OFF true
  if(FUCK_OFF && ((v0-v1).length2() > 1 ||
     (v1-v2).length2() > 1 ||
		  (v2-v0).length2() > 1)){
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


void rasterize_surface(Surface & surf, 
		    GlPoints & pnt,         //the point set to render 
		       const RenderingTraits & t) //how to render
{
  //ok, now all the correct points in tri.
  //read tris now
  V3f center;
  V3f m[3];
  //  std::vector<int> tristor; //triangle storage

  //loop trough every triangle and refine it.
  for(unsigned int i = 0; i < surf.v.size(); i+=3){

    m[0]=surf.v[i];
    m[1]=surf.v[i+1];
    m[2]=surf.v[i+2];

    refine_triangle(m[0], m[1], m[2], pnt,  surf.n[i], t); 
  };
 
  if(t.inside){
  printf("Trying to fill it\n");
  center = find_center_point(surf); //this is the average;
  printf("The seed is %f %f %f; %d in total;  so what? \n", center.x, center.y, center.z, (int)surf.v.size());
   int cur = pnt.vol.getOffset(center.x, center.y, center.z);
   pnt.vol.mask[cur] |= BDR;
   pnt.vol.markers.push_back(cur);
  
  pnt.vol.propagate(1000, 0, 1000, 400); //propagate everywhere from centerpoint  

  //remove truth mask as not propagating anymore
  
  if(!t.tru){
  for(int i = 0; i < 256*256*256; i++)
    pnt.vol.mask[i] -= (pnt.vol.mask[i] & TRU);
  }else{
  for(int i = 0; i < 256*256*256; i++)
    if(pnt.vol.mask[i] & MASK)pnt.vol.mask[i] |= TRU;    
  };
};
};
//**//




