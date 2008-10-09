#include <gtest/gtest.h>

#include <zlib.h>
#include <stdlib.h>

#include "loader.h"



//_* Exception handling
Loader::Ex::Ex(std::string why): reason(why){};
Loader::Ex::Ex(const Ex & to_copy): reason(to_copy.reason){};
Loader::Ex Loader::Ex::operator=(const Loader::Ex & to_copy){
  reason = to_copy.reason;
  return * this;
};


//_* Main Loader code
//_ * Constructor - destructor
Loader::Loader(){
  res = NULL;
  total = 0;
  cur_plane = VOLUME_PLANE; //use volume
};

Loader::~Loader(){
  if(res != NULL)delete[] res; //Free memory
};


void get_mgz_info(std::string name){
  //nop
};


int Loader::read(std::string name){ 

  gzFile fd; //file descriptor

  const int BUF_SIZE=16384;
  char buf [BUF_SIZE]; //for reading from the file
  
  std::string contents; //vector of strings, to be composed later into one vec.

  fd = gzopen(name.c_str(),"rb");
  if(fd == NULL) throw Ex("Cannot open file"); //cannot read file.
  
  
  int cnt = 0;

  while((cnt = gzread(fd, (void *) buf, BUF_SIZE))){
	contents+=std::string((char *)buf, cnt);
  };
  ::gzclose(fd);  

  if(!res) delete[] res; //just in case the size is different.
  res = new char[contents.size()];
  if(!res) throw Ex("Unable to create buffer");
  
  //copying the collection into the buffer; the buffer will be used for 
  // further manipulation - storing, writing.
  total = (int)contents.size();
  memcpy((void *)(res), contents.c_str(), contents.size()); 
  printf("Acqured %d bytes.\n", (int)contents.size());

  //getting additional information
  //  get_mgz_info(name);

  return (int)contents.size();  
};

int Loader::write(std::string name){

  printf("file %s\n",  name.c_str());
  /* 
  int fd = open(name.c_str(), O_WRONLY | O_CREAT, 
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

  //trying opening existing file
  if(fd < 0) fd = open(name.c_str(), O_WRONLY | O_TRUNC, 
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  */

  gzFile fd = gzopen(name.c_str(), "wb");
  //still cannot - fail:
  if(fd == NULL) throw Ex("Cannot open file"); //cannot read file.
  
  int cnt = gzwrite(fd, res, total);
  gzflush(fd, Z_FINISH);
  if(cnt != total){
    gzclose(fd);
    throw Ex("Writing failed.");
  };

  gzclose(fd);

  return total;
};

//_ * Parser - deparser:
/* Copy contents in the center of the fast volume
 */

//_  * public interface
  
void Loader::read_volume(FastVolume &vol_out){  
  try{
    parse(res, vol_out, true);
  }catch(const char * in){
    throw Ex(in);
  };
};


void Loader::write_volume(FastVolume &vol_out){
  try{
    parse(res, vol_out, false);
  }catch(const char * in){
    throw Ex(in);
  };
};

//_  * utility functions for parsing

//volume operations

int Loader::get(const FastVolume & in, int x, int y, int z){ 
  switch(cur_plane){
  case MASK_PLANE:
    return (MSK & in.mask[FastVolume::getOffset(x,y,z)]);
  case VOLUME_PLANE:
  default:
    return in.vol[FastVolume::getOffset(x,y,z)];
  };    
}; 

//set a voxel value
void Loader::set(FastVolume & in, int x, int y, int z, int val){
  switch(cur_plane){
  case MASK_PLANE:
    in.mask[FastVolume::getOffset(x,y,z)] = (val > 0)?MSK:0;
  break;
  case VOLUME_PLANE:
  default:
    in.vol[FastVolume::getOffset(x,y,z)] = val;
  };
};


//raw memory copy from a string
void Loader::memcpy_reverse(raw data, char * output, int size, int &pos){
	if(total < pos+size)throw "Not enough bytes to read;";
	for(int i = 0; i < size; i++){output[size-i-1]=data[pos+i];};
	pos+=size;
};

//writing the results back
void Loader::memcpy_reverse_w(raw data, const char * input, int size, int &pos){
	if(total < pos+size)throw "Not enough bytes to write;";
	for(int i = 0; i < size; i++){data[pos+i] = input[size-i-1];};
	pos+=size;
};

// raw operations:
int Loader::get_int(const raw data, int & pos){
  int res; memcpy_reverse(data, (char *)&res, sizeof(int), pos);
  return res;
};

short Loader::get_short(const raw data, int & pos){
  short res; memcpy_reverse(data, (char *)&res, sizeof(short), pos);
  return res;
};

float Loader::get_float(const raw data, int & pos){
  float res; memcpy_reverse(data, (char *)&res, sizeof(float), pos);
  return res;
};

char Loader::get_char(const raw data, int & pos){
  char res = data[pos]; pos++;
  return res;
};

  void Loader::set_int(raw data, int in, int & pos){int tmp = in; 
  	memcpy_reverse_w(data, (char *)&tmp, sizeof(int), pos);};
  void Loader::set_short(raw data, short in, int & pos){short tmp = in; 
  	memcpy_reverse_w(data, (char *)&tmp, sizeof(short), pos);};
  void Loader::set_float(raw data, float in, int & pos){float tmp = in; 
  	memcpy_reverse_w(data, (char *)&tmp, sizeof(float), pos);};
  void Loader::set_char(raw data, char inc, int & pos){data[pos]=inc; pos++;};

//_  * Main parsing function

void Loader::parse(raw data, FastVolume & result, bool read){


  const int MRI_UCHAR   = 0;
  const int MRI_INT     = 1;
  //  const int MRI_LONG    = 2;
  const int MRI_FLOAT   = 3;
  const int MRI_SHORT   = 4;
  // const int MRI_BITMAP  = 5;
  const int MRI_TENSOR  = 6;

  const int UNUSED_SPACE_SIZE = 256;
  const int USED_SPACE_SIZE   = (3*sizeof(float)+4*3*sizeof(float));
  //const int STRLEN = 256;


  int pos = 0; //current read position

  int   /*start_frame, end_frame,*/ width, height, depth, nframes, type, x, y, z,
    /*bpv,*/ dof, /*bytes, version, ival,*/ unused_space_size, good_ras_flag/*, i*/ ;
  //char   unused_buf[UNUSED_SPACE_SIZE+1] ;
  float  /*fval,*/ xsize, ysize, zsize, x_r, x_a, x_s, y_r, y_a, y_s,
    z_r, z_a, z_s, c_r, c_a, c_s ;
  //  short  sval ;
  //  int tag_data_size;
  // char *ext;
  //char command[STRLEN];
  int nread;
  //int tag;

  xsize = ysize = zsize = 0;
  x_r = x_a = x_s = 0;
  y_r = y_a = y_s = 0;
  z_r = z_a = z_s = 0;
  c_r = c_a = c_s = 0;

  printf("reading basic info\n");
  nread = get_int(data, pos);
  if (!nread)throw "Bad version code(?)";

  width = get_int(data, pos) ;
  height = get_int(data, pos) ;
  depth =  get_int(data, pos) ;
  nframes = get_int(data, pos) ;
  type = get_int(data, pos) ;
  dof = get_int(data, pos) ;

  printf("whdntd: %d %d %d\n", width, height, depth);
  result.tr.width = width;
  result.tr.height = height;
  result.tr.depth = depth;

  if(read){ //if we are reading, then create
  printf("Reading...\n");
  // create(result, width, height, depth);
  //printf("created %d\n;", result.size);
  }; 
  
  unused_space_size = UNUSED_SPACE_SIZE-sizeof(short) ;

  good_ras_flag = get_short(data, pos) ;
  if (good_ras_flag > 0){     /* has RAS and voxel size info */
    unused_space_size -= USED_SPACE_SIZE ;
    xsize = get_float(data, pos) ;
    ysize = get_float(data, pos) ;
    zsize = get_float(data, pos) ;

    printf("xyz-size:%f %f %f\n", xsize, ysize, zsize);

    x_r = get_float(data, pos) ; x_a = get_float(data, pos) ; x_s = get_float(data, pos) ;
    y_r = get_float(data, pos) ; y_a = get_float(data, pos) ; y_s = get_float(data, pos) ;

    z_r = get_float(data, pos) ; z_a = get_float(data, pos) ; z_s = get_float(data, pos) ;
    c_r = get_float(data, pos) ; c_a = get_float(data, pos) ; c_s = get_float(data, pos) ;
  };
  /* so stuff can be added to the header in the future */
  ///fread(unused_buf, sizeof(char), unused_space_size, fp) ;
  
  printf("R: x(%f) y(%f) z(%f) c(%f)\n", x_r, y_r, z_r, c_r);
  printf("A: x(%f) y(%f) z(%f) c(%f)\n", x_a, y_a, z_a, c_a);
  printf("S: x(%f) y(%f) z(%f) c(%f)\n", x_s, y_s, z_s, c_s);
  result.tr.c_r = c_r;
  result.tr.c_a = c_a;
  result.tr.c_s = c_s;
  

   pos += unused_space_size;

   int cur_data;

  if(!read){ //do some sanity checks before writing
    //if(depth != result.depth || width != result.width || height != result.height) throw "Dimensions are mismatched";  	
  };
    
//  for(int j = 0; j < result.depth*result.height*result.width; j++)
//	  		if(result.data[j] != 0)printf("<%d:%d>", j, result.data[j]);
 
    
  for (z = 0 ; z < depth ; z++){
    for (y = height-1 ; y >=0 ; y--)
      {
	for (x = 0 ; x < width ; x++)
	  {
	  	if(read){
		  switch (type)
		    {
		    case MRI_INT: set(result, x, y, z, get_int(data, pos)); break;
		    case MRI_SHORT: set(result, x, y, z, get_short(data, pos)); break;
		    case MRI_UCHAR: set(result, x, y, z, get_char(data, pos)); break;
		    case MRI_FLOAT: set(result, x, y, z, (int)get_float(data, pos)); break;
		    case MRI_TENSOR: throw "Unable to read tensors";
		    }
	  	}else{ //write
	  		//if(get(result, x, y, z) != 0)printf("(%d,%d,%d,%d) ", x, y, z, get(result, x, y, z));
		  cur_data = get(result, x, y, z);
		  switch (type)
		    {
		    case MRI_INT: set_int(data, cur_data, pos); break;
		    case MRI_SHORT: set_short(data, cur_data, pos); break;
		    case MRI_UCHAR: set_char(data, cur_data, pos); break;
		    case MRI_FLOAT: set_float(data, (float)cur_data, pos); break;
		    case MRI_TENSOR: throw "Unable to read tensors";
		    };
	  	};
	  };
      };
  };
};

bool MgzSanityCheck(MgzLoader::Header & header){
  return (header.width < 1000) && (header.width > 10) &&
    (header.height < 1000) && (header.width > 10) &&
    (header.depth < 1000) && (header.width > 10);
};

bool MgzReadHeader(Io & io, MgzLoader::Header * header){
  const int UNUSED_SPACE_SIZE = 256;
  const int USED_SPACE_SIZE   = (3*sizeof(float)+4*3*sizeof(float));
  int unused_space_size = UNUSED_SPACE_SIZE-sizeof(short) ;

  int   width, height, depth, nframes, x, y, z, dof;
    short good_ras_flag;

  float  xsize, ysize, zsize, x_r, x_a, x_s, y_r, y_a, y_s;
  float z_r, z_a, z_s, c_r, c_a, c_s ;

  int nread;

  xsize = ysize = zsize = 0;
  x_r = x_a = x_s = 0;
  y_r = y_a = y_s = 0;
  z_r = z_a = z_s = 0;
  c_r = c_a = c_s = 0;

  printf("reading basic info\n");
  io.GetInt(&nread);
  if (!nread) return false;

  io.GetInt(&header->width).GetInt(&header->height)
    .GetInt(&header->depth).GetInt(&nframes);
  header->data_type = (MgzLoader::DataType)io.GetInt();
  io.GetInt(&dof);

  printf("whdntd: %d %d %d\n", header->width, header->height, header->depth);


  io.GetShort(&good_ras_flag);
  if (good_ras_flag > 0){     /* has RAS and voxel size info */
    unused_space_size -= USED_SPACE_SIZE ;
    io.GetFloat(&xsize).GetFloat(&ysize).GetFloat(&zsize);

    printf("xyz-size:%f %f %f\n", xsize, ysize, zsize);

    io.GetFloat(&x_r).GetFloat(&x_a).GetFloat(&x_s)
      .GetFloat(&y_r).GetFloat(&y_a).GetFloat(&y_s)
      .GetFloat(&z_r).GetFloat(&z_a).GetFloat(&z_s)
      .GetFloat(&c_r).GetFloat(&c_a).GetFloat(&c_s);
  
  printf("R: x(%f) y(%f) z(%f) c(%f)\n", x_r, y_r, z_r, c_r);
  printf("A: x(%f) y(%f) z(%f) c(%f)\n", x_a, y_a, z_a, c_a);
  printf("S: x(%f) y(%f) z(%f) c(%f)\n", x_s, y_s, z_s, c_s);
  };

  io.set_position(io.get_position() + unused_space_size);

  int cur_data;

  return io && MgzSanityCheck(*header);
}; 

bool MgzRead(Io & io, const MgzLoader::Header & header, FastVolume * volume){
  for (int z = 0 ; z < header.depth ; ++z){
    for (int y = header.height-1 ; y >=0 ; --y){
      for (int x = 0 ; x < header.width ; ++x){
	switch (header.data_type){
	  case MgzLoader::INT: volume->Set(x,y,z, (short)io.GetInt()); break;
	  case MgzLoader::SHORT: volume->Set(x,y,z,io.GetShort()); break;
	  case MgzLoader::UCHAR: volume->Set(x,y,z, (short)io.GetChar()); break;
	  case MgzLoader::FLOAT: volume->Set(x,y,z, (short)io.GetFloat()); break;
	  default: return false;
	};
      };
    };
  };
  return io.valid();
};

bool MgzWrite(Io & io, 
const MgzLoader::Header & header, 
const FastVolume & volume){
  for (int z = 0 ; z < header.depth ; ++z){
    for (int y = header.height-1 ; y >=0 ; --y){
      for (int x = 0 ; x < header.width ; ++x){
	switch (header.data_type){
	  case MgzLoader::INT: io.PutInt(volume.Get(x,y,z)); break;
	  case MgzLoader::SHORT: io.PutShort(volume.Get(x,y,z)); break;
	  case MgzLoader::UCHAR: io.PutChar(volume.Get(x,y,z)); break;
	  case MgzLoader::FLOAT: io.PutFloat(volume.Get(x,y,z)); break;
	  default: return false;};
      };
    };
  };
  return io.valid();
};

TEST(Mri, Header){
  Io in(ReadGzipFile("brainmask.mgz"));
  MgzLoader::Header header;
  EXPECT_TRUE(MgzReadHeader(in, &header));
  ASSERT_TRUE(MgzSanityCheck(header));
  EXPECT_EQ(256, header.width);
  EXPECT_EQ(256, header.height);
  EXPECT_EQ(256, header.depth);
};



/// Clean implementation of loading/saving MGZ files.

MgzLoader::MgzLoader(FastVolume & volume): volume_(volume) {
};

MgzLoader::~MgzLoader(){
};

MgzLoader & MgzLoader::Save( std::string file_name){
  file_name_ = file_name;
  Io io(file_content_);
  if(!MgzReadHeader(io, &header_)){valid(false); return * this;};
  if(!MgzWrite(io, header_, volume_)){valid(false); return * this;};
  file_content_ = io.content();
  valid(WriteGzipFile(file_name_, file_content_));
  return * this;
};

MgzLoader & MgzLoader::Load( std::string file_name){
  file_name_ = file_name;
  file_content_ = ReadGzipFile(file_name_);
  Io io(file_content_);
  if(!MgzReadHeader(io, &header_)){valid(false); return * this;};
  if(!MgzRead(io, header_, &volume_)){valid(false); return * this;};
  valid(io.valid());
  return * this;
};


TEST(Mri, Read){
  FastVolume volume;
  FastVolume volume_two;
  MgzLoader::Header header;
  Io io(ReadGzipFile("brainmask.mgz"));
  ASSERT_TRUE(MgzReadHeader(io, &header));
  EXPECT_TRUE(MgzRead(io, header, &volume));
  volume.Set(45,65,54, 3);
  io.rewind();
  ASSERT_TRUE(MgzReadHeader(io, &header));
  EXPECT_TRUE(MgzWrite(io, header, volume));
  io.rewind();
  ASSERT_TRUE(MgzReadHeader(io, &header));
  EXPECT_TRUE(MgzRead(io, header, &volume_two));
  EXPECT_EQ(volume.Get(45,65,54), volume_two.Get(45,65,54));
};



TEST(Mri, ReadOriginal){
  FastVolume volume;
  FastVolume volume_two;
  Loader loader;
  loader.read("brainmask.mgz");
  loader.read_volume(volume);
  volume.Set(45,65,54, 3);
  loader.write_volume(volume);
  loader.read_volume(volume_two);
  EXPECT_EQ(volume.Get(45,65,54), volume_two.Get(45,65,54));
};

