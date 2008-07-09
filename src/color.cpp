#include "color.h"

#include <string.h>
#include <vector>
#include "v3.h"
#define GLFW_DLL
#include "GL/glfw.h"

#include "clut.h"

/**
   Make sure to call color_init() to set up the color tables. Well..ok.
 */

ColorMapper::ColorMapper(){
  for(unsigned int i = 0; i < 256; i++)memset((void *)(&(col[i*3])), i, 3);

  start = 0;
  from = V3f(0,0,0);
};

ColorMapper & ColorMapper::operator=(const ColorMapper & in){
  memcpy(col, in.col, 256*3);
  return *this;
};

ColorMapper::ColorMapper(const ColorMapper & in){
  this->operator=(in);
};

void ColorMapper::map(void * where, int i){
  if(i>200)i=200;
  if(i<0)i=0;
  memcpy(where, (void *)(&(col[(i%255)*3])), 3);
};
 
void ColorMapper::map(int & r, int & g, int &b, int i){
  if(i>200)i=200;
  if(i<0)i=0;
  r = col[i*3];
  g = col[i*3+1];
  b = col[i*3+2];
};


void ColorMapper::map(int i){
  if(i>200)i=200;
  if(i<0)i=0;
  glColor4b(col[i*3]>>1,col[i*3+1]>>1,col[i*3+2]>>1, 10);
};


void ColorMapper::fill_init(V3f _f){
  start = 0; 
  from = _f;
}; 


void ColorMapper::fill(const int * in){ // "index;R;G;B" 256 times
  for(unsigned int i = 0; i < 256; i++)
    for(int j=0; j < 3; j++)
      col[i*3+j]=in[i*4+1+j];

  start = 0;
  from = V3f(0,0,0);
};

void ColorMapper::fill( int end, V3f to){
  if(end > 255)end = 255;
  V3f dc(to);
  dc -= from;
  dc /=(end-start);
  V3f cur(from);

  for(int i = start; i < end; i++){
    col[i*3] = (unsigned char)cur.x;
    col[i*3+1] = (unsigned char)cur.y;
    col[i*3+2] = (unsigned char)cur.z;
    cur += dc;
  };

  start = end;
  from = to;
};


#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

/**
   An element in a scheme selection 
*/
struct ColorEntry{
  string name;
  ColorMapper map;

  ColorEntry();
  ColorEntry(const ColorEntry &);
  ColorEntry(string, const ColorMapper &);
  ColorEntry(const char *, const ColorMapper &);
  ColorEntry & operator=(const ColorEntry &);
};

ColorEntry::ColorEntry(){
};

ColorEntry::ColorEntry(string _name, const ColorMapper & _map){
  name = _name;
  map = _map;
};

ColorEntry::ColorEntry(const char * _name, const ColorMapper & _map){
  name = string(_name);
  map = _map;
};

ColorEntry & ColorEntry::operator=(const ColorEntry & in){
  name = in.name;
  map = in.map;
  return *this;
};

ColorEntry::ColorEntry(const ColorEntry & in){
  this->operator=(in);
};
  
/**
Initializer
*/
bool color_inited = false;
vector<ColorEntry> entries;


void scheme_fill(ColorMapper & in, int scheme){
  if(!color_inited)return;
  if(scheme < 0 || (unsigned int)scheme >= entries.size())return;
  in = entries[scheme].map;
};


void color_init(){
  color_inited = true;

  ColorMapper a_map;


    a_map.fill_init( V3f(0,0,0) );
    a_map.fill(255, V3f(255,255,255));
    entries.push_back(ColorEntry("OneToOne", a_map));

    a_map.fill_init( V3f(0,0,0) );
    a_map.fill( 20, V3f(30,244,0));
    a_map.fill( 50, V3f(30,0,0));
    a_map.fill( 100, V3f(60,140,0));
    a_map.fill( 255, V3f(0,0,0));
    entries.push_back(ColorEntry("Greeny", a_map));

    a_map.fill_init( V3f(0,0,255) );
    a_map.fill( 50, V3f(30,30,40));
    a_map.fill( 100, V3f(30,200,10));
    a_map.fill( 200, V3f(60,140,10));
    a_map.fill( 255, V3f(0,0,0));
    entries.push_back(ColorEntry("Blueish", a_map));

    a_map.fill_init( V3f(255,255,255) );
    a_map.fill( 50, V3f(30,30,30));
    a_map.fill( 255, V3f(0,0,0));
    entries.push_back(ColorEntry("Inverse", a_map));

    a_map.fill_init( V3f(0,0,0) );
    a_map.fill( 20, V3f(100,0,0));
    a_map.fill( 60, V3f(0,244,0));
    a_map.fill( 80, V3f(0,0,0));
    a_map.fill( 255, V3f(244,244,0));
    entries.push_back(ColorEntry("Contrast", a_map));

    //now loading more stuff from a static clut

    for(int i = 0 ; i < clutSize; i++){
      a_map.fill(aColorTable[i].data);
      entries.push_back(ColorEntry(aColorTable[i].name, a_map));
   };
};

vector<string> color_type(){
  vector<string> out;
  for(unsigned int i = 0; i < entries.size(); i++)
    out.push_back(entries[i].name);
  return out;
};






