#ifndef __fastvolume_h__
#define __fastvolume_h__

#include <vector>

/* 256x256x256 volume storage */
class FastVolume {
 public:  
  /* Storage item */
  typedef short int t_vox; 
  
  /* Constructor/Destructor */
  FastVolume();
  ~FastVolume();
  
 /* Main storage; won't fit on stack */
  t_vox * vol;

  /* Cubic arrangement makes for efficient lookup */
  static const int dx = 0x01;
  static const int dy = 0xff;
  static const int dz = 0xffff;

  static const int dim = 0x100;
  static const int max = dim*dim*dim;

  //static const int neighbours[6] = {dx, -dx, dy, -dy, dz, -dz};

  /* Calculate offset from separate coords */
  static inline int getOffset(int x, int y, int z){
    return x+(y<<8)+(z<<16);
  };

  /* Calculate separate coords from offset */
  static inline void getCoords(int off, int &x, int &y, int &z){
    x=off & 0xff;
    y=(off & 0xff00)>>8;
    z=(off & 0xff0000)>>16;
  };

  /* Copy data from linear volume to current one.
     Takes a volume of x*y*z array and loads it into the 256*256*256 volume */
  void copy(t_vox * , int x, int y, int z);


  /* Find all points neighbouring an outside;
   space: max. value still considered outside the area of interest.
   Bad approach as there are different methods to look for surface. */
  void findSurface(std::vector<int> &, int space);

  /* Derive this class for different purposes; and use different iterator methods*/
  class Iterator {
  public:
    /* Iterate a line; calling for every point is too expensive */
    virtual void line(int start, int finish) = 0;
  };

  static inline void iterate(Iterator &);

  ///o - beginning of the slice
  ///x,y - extent fo the slice
  ///data[x*y] - slice storage

  enum orientation{
    XY, XZ, YZ
  };

  void slice(int x, int y, int z, int width, int height, orientation pos, char * data);
  
};



#endif //__fastvolume_h__









