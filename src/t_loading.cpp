#include "fastvolume.h"
#include "loader.h"

/* Load data from file */
//_* Main

/* Main */
int main(int argc, char ** argv){
  
  if(argc != 3){
    printf("<where to read> <where to write>.\n");
    exit(1);
  };

  Loader t; //test loader;

  FastVolume vol;

  std::vector<int> list;

  try{
    int cnt = t.read(std::string(argv[1]));
    printf("Read %d bytes.\n", cnt);

    t.read_volume(vol);

    vol.findSurface(list, 1);
    printf("found %d points.\n", list.size());

    t.write_volume(vol);

    t.write(std::string(argv[2]));
  }catch(const Loader::Ex & err){
    printf("Problem occured: %s\n", err.reason.c_str());
  };


  return 1;
};


