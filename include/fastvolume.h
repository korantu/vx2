#ifndef __fastvolume_h__
#define __fastvolume_h__

#include <vector>
#include "v3.h"
#include "color.h"

#define BDR 0x80 //active border; can have several generations
#define ZRO 0x40 //zero points, which will be removed from the resulting
#define MASK ( BDR | MSK ) //selected points
#define TRU 0x20 //known-true point; loaded externally.
#define MSK 0x10 //this is the mask itself... gotta hurt :(
#define AUX 0x08 //bit for doing various stuff.
#define HIG 0x04 //highlighting bit.
#define GEN_MAX 0x03 //ok, now only 3 generations avaliable; none really needed
#define FLAGS (0xff - GEN_MAX) //getting flags
#define GEN(X) ((X)&GEN_MAX) //what is the generation


/* 256x256x256 volume storage */
class FastVolume {
 public:  

  
  //   Transform parameters
   
  struct t_tr{
    int width, height, depth;
    float c_r, c_a, c_s;
  }tr;

  bool updated;
  /* Storage item */
  typedef short int t_vox; 

  /* Constructor/Destructor */
  FastVolume();
  ~FastVolume();
  
 /* Main storage; won't fit on stack */
  t_vox * vol;

  ///markers
  std::vector<int> markers;
  std::vector<int> plane;

  unsigned char * mask;
  unsigned char * depth;
  //void add_point(V3f &in); use tool
  void propagate(int threshold, int amount, int depth, int times);
  void propagate_spread(int threshold, int amount, int depth, int times);
  void propagate_jump(int threshold, int amount, int depth, int times);
  void downshift(int flags); //shift all values down;
  void use_tool(int where, int which, int size);
  void scan_for_disconnected_regions();

  int cur_gen;

  /* similarity propagation parameters */
  float half_band_size;
  float band_center;
  void set_band();

  /* Scope of operations. */
  bool use_scope;
  int min_x;
  int max_x;
  int min_y;
  int max_y;
  int min_z;
  int max_z;
  V3f center;
  bool in_scope(int offset);

  /* Cubic arrangement makes for efficient lookup */
  static const int dx = 0x01;
  static const int dy = 0x100;
  static const int dz = 0x10000;

  static const int dim = 0x100;
  static const int max = dim*dim*dim;

  static const int n_voxels = dim*dim*dim;

  //6 first are 6-connected.
  static const int neighbours[26];// = {dx, -dx, dy, -dy, dz, -dz};

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

  /// resets all the buffers
  void reset();

  /// makes sure mask buffer corresponds to the marker buffer; i.e. all neigbouring points are included.
  void reseed();

  /* Find all points neighbouring an outside;
   space: max. value still considered outside the area of interest.
   Bad approach as there are different methods to look for surface. */
  void findSurface(std::vector<int> &, int space);

  /* Derive this class for different purposes; and use different iterator methods
    class Iterator {

  public:
      // Iterate a line; calling for every point is too expensive 
    virtual void line(int start, int finish) = 0;

  };
  */
  //center, dx, dy, pixel/width/height, buffer, zoom (default = 1) 
  // static inline void iterate(Iterator &);
  void raster( V3f o, V3f dx, V3f dy, int w, int h, unsigned char * buf, ColorMapper & mapper, int zoom = 1, bool show_mask = true);

  ///undo facilities
  /// buffer for the new undo infrastructure; test it with undoing truths modifications first.

  void record_operation(int pos, unsigned char before, unsigned char after); //call if you want to remember 
  void record_operation(int pos, unsigned char after);
  void record_done();
  void undo();
  void undo_reset();

};



#endif //__fastvolume_h__









