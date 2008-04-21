#include "gl_slicer.h"
#include "glfw.h"

struct GlSlicer: public GlSlicerI {
  unsigned char * data;
  PFNGLTEXIMAGE3DPROC glTexImage3D;
  unsigned int texname;

  virtual void setup3dTexture();

  GlSlicer(const FastVolume & in);
  virtual ~GlSlicer();
};

///creating the structure
GlSlicerI * GlSlicerI::create(const FastVolume & in){
  return new GlSlicer(in);
};

GlSlicer::GlSlicer(const FastVolume & in){
  
  glTexImage3D = (PFNGLTEXIMAGE3DPROC) glfwGetProcAddress("glTexImage3D");
  if(!glTexImage3D)throw("Cannot obtain glTexImage3d extension\n");

  data = new unsigned char [in.max*3]; //in case it still wants rgb/ tODO fix.

  if(!data)throw("Cannot allocate memory.\n");

 
  //copy the stuff over;
  for(int i = 0; i < in.max; i++)
    data[i] = i%255;//(5+in.vol[i])%255;
};


GlSlicer::~GlSlicer(){
  delete data;
};

void GlSlicer::setup3dTexture(){
  glDisable(GL_LIGHTING);
  glEnable(GL_COLOR);
  glEnable(GL_TEXTURE_3D);
  glGenTextures(1, &texname);
  glBindTexture(GL_TEXTURE_3D, texname);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
  glTexImage3D(GL_TEXTURE_3D, 
	       0, // level of detail 
	       1, // byte per point, 
	       256, 256, 256, //dimensions
	       0, // border
	       GL_GREEN, GL_UNSIGNED_BYTE, //all is green, unsigned, conserving memory...
	       data);
};
			      








