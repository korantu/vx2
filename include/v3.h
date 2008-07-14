/*
 Voxelbrain Project. MRI editing software.
 This file is (c) 2007,2008
 Nanyang Technological University

 author: Konstantin Levinski
 description:

  Vector operations.

*/

#ifndef __vector_h__
#define __vector_h__

/*

Basic misc definitions

*/

#include <math.h>

  //common vector definition, will be instantiated for integers and floats.
  template<class T>
    struct V3 {
      T x; 
      T y; 
      T z;

      V3(){};
      V3(T _x, T _y, T _z):x(_x),y(_y),z(_z){};
      V3(const V3 & v):x(v.x),y(v.y),z(v.z){};
      V3 & operator=(const V3 & v){ x = v.x; y = v.y; z = v.z; return *this;};
      V3 & set(T _x, T _y, T _z){ x = _x; y = _y; z = _z; return *this;};
      V3 & set(double _x, double _y, double _z){ x = (T)_x; y = (T)_y; z = (T)_z; return *this;};

      bool operator==(const V3 & v) const { return (x == v.x && y == v.y && z == v.z);};

      //temporary var is required but more readable;
      V3 operator-(const V3 & v) const { return V3(x-v.x, y-v.y, z-v.z);};
      V3 operator+(const V3 & v) const { return V3(x+v.x, y+v.y, z+v.z);};
      V3 operator*(float m) const { return V3((T)(x*m), (T)(y*m), (T)(z*m));};
      V3 operator/(float m) const { return V3((T)(x/m), (T)(y/m), (T)(z/m));};
	//for msvc; TODO: review
	  V3 operator*(double m) const { return V3((T)(x*m), (T)(y*m), (T)(z*m));};
      V3 operator/(double m) const { return V3((T)(x/m), (T)(y/m), (T)(z/m));};
      V3 operator*(int m) const { return V3((T)(x*m), (T)(y*m), (T)(z*m));};
      V3 operator/(int m) const { return V3((T)(x/m), (T)(y/m), (T)(z/m));};
 
      //no temporary vars
      V3 & operator-=(const V3 & v){ x-=v.x; y-=v.y, z-= v.z; return *this;};
      V3 & operator+=(const V3 & v){ x+=v.x; y+=v.y, z+= v.z; return *this;};
      V3 & operator*=(float m){ x*=m; y*=m, z*=m; return *this;};
      V3 & operator/=(float m){ x/=m; y/=m, z/=m; return *this;};
	//for msvc; might also make sense. TODO: review
	  V3 & operator*=(double m){ x*=(T)m; y*=(T)m, z*=(T)m; return *this;};
      V3 & operator/=(double m){ x/=(T)m; y/=(T)m, z/=(T)m; return *this;};
      V3 & operator*=(int m){ x*=(T)m; y*=(T)m, z*=(T)m; return *this;};
      V3 & operator/=(int m){ x/=(T)m; y/=(T)m, z/=(T)m; return *this;};

      T length2(){ return (x*x+y*y+z*z);};
      float length(){ return sqrtf((float)length2());};
  
      T distance2(const V3 & v) {return (v.x-x)*(v.x-x)+(v.y-y)*(v.y-y)+(v.z-z)*(v.z-z);};
      float distance(const V3 & v) {return sqrtf((float)distance2());};

      //float vector definition
      V3 & cross(const V3 & a, const V3 & b){ x = a.y*b.z-a.z*b.y;
                                           y = a.z*b.x-a.x*b.z;
					   z = a.x*b.y-a.y*b.x;
                                           return *this;};  
      float dot(const V3 & a) const { return x*a.x+y*a.y+z*a.z;};

      T operator[] (int i) const {return ((const T *)(&x))[i];};
    
    };

  
  typedef V3<int> V3i; //integer vector
  typedef V3<float> V3f; // float vector

  
#endif















