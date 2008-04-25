#include "loader.h"
#include <zlib.h>

//_* Exception handling
Loader::Ex::Ex(std::string why): reason(why){};
Loader::Ex::Ex(const Ex & to_copy): reason(to_copy.reason){};
Loader::Ex Loader::Ex::operator=(const Loader::Ex & to_copy){
  reason = to_copy.reason;
};


//_* Main Loader code
//_ * Constructor - destructor
Loader::Loader(){
  res = NULL;
  total = 0;
};

Loader::~Loader(){
  if(res != NULL)delete[] res; //Free memory
};



//_ * Read-write
int Loader::read(std::string name){

  gzFile fd; //file descriptor

  const int BUF_SIZE=16384;
  char buf [BUF_SIZE]; //for reading from the file
  
  std::string contents; //vector of strings, to be composed later into one vec.

  fd = gzopen(name.c_str(),"ro");
  if(fd == NULL) throw Ex("Cannot open file"); //cannot read file.
  
  //reading

  
  int cnt = 0;

  while(cnt = gzread(fd, (void *) buf, BUF_SIZE)){
	contents+=std::string((char *)buf, cnt);
  };
  ::gzclose(fd);  

  if(!res) delete[] res; //just in case the size is different.
  res = new char[contents.size()];
  if(!res) throw Ex("Unable to create buffer");
  
  //copying the collection into the buffer; the buffer will be used for 
  // further manipulation - storing, writing.
  total = contents.size();
  memcpy((void *)(res), contents.c_str(), contents.size()); 
  printf("Acqured %d bytes.\n", contents.size());
  return contents.size();  
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
  return in.vol[FastVolume::getOffset(x,y,z)];
}; 

//set a voxel value
void Loader::set(FastVolume & in, int x, int y, int z, int val){
  in.vol[FastVolume::getOffset(x,y,z)] = val;
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
  const int MRI_LONG    = 2;
  const int MRI_FLOAT   = 3;
  const int MRI_SHORT   = 4;
  const int MRI_BITMAP  = 5;
  const int MRI_TENSOR  = 6;

  const int UNUSED_SPACE_SIZE = 256;
  const int USED_SPACE_SIZE   = (3*sizeof(float)+4*3*sizeof(float));
  const int STRLEN = 256;


  int pos = 0; //current read position

  int   start_frame, end_frame, width, height, depth, nframes, type, x, y, z,
    bpv, dof, bytes, version, ival, unused_space_size, good_ras_flag, i ;
  char   unused_buf[UNUSED_SPACE_SIZE+1] ;
  float  fval, xsize, ysize, zsize, x_r, x_a, x_s, y_r, y_a, y_s,
    z_r, z_a, z_s, c_r, c_a, c_s ;
  short  sval ;
  //  int tag_data_size;
  char *ext;
  char command[STRLEN];
  int nread;
  int tag;

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

    x_r = get_float(data, pos) ; x_a = get_float(data, pos) ; x_s = get_float(data, pos) ;
    y_r = get_float(data, pos) ; y_a = get_float(data, pos) ; y_s = get_float(data, pos) ;

    z_r = get_float(data, pos) ; z_a = get_float(data, pos) ; z_s = get_float(data, pos) ;
    c_r = get_float(data, pos) ; c_a = get_float(data, pos) ; c_s = get_float(data, pos) ;
  }
  /* so stuff can be added to the header in the future */
  ///fread(unused_buf, sizeof(char), unused_space_size, fp) ;
  
  pos += unused_space_size;
  
  if(!read){ //do some sanity checks before writing
    //if(depth != result.depth || width != result.width || height != result.height) throw "Dimensions are mismatched";  	
  };
    
//  for(int j = 0; j < result.depth*result.height*result.width; j++)
//	  		if(result.data[j] != 0)printf("<%d:%d>", j, result.data[j]);
 
    
  for (z = 0 ; z < depth ; z++){
    for (i = y = 0 ; y < height ; y++)
      {
	for (x = 0 ; x < width ; x++, i++)
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
	    switch (type)
	      {
		      case MRI_INT: set_int(data, get(result, x, y, z), pos); break;
		      case MRI_SHORT: set_short(data, get(result, x, y, z), pos); break;
		      case MRI_UCHAR: set_char(data, get(result, x, y, z), pos); break;
		      case MRI_FLOAT: set_float(data, get(result, x, y, z), pos); break;
		      case MRI_TENSOR: throw "Unable to read tensors";
	      };
	  	};
	  }
      };
  };
};
  





