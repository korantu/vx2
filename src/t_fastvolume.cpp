#include "fastvolume.h"
#include <stdio.h>
#include <time.h>
#include <vector>

using namespace std;

//Convinience for measuring time
struct Timer {
  Timer();
  void done(char *);

private:
  int time;
};

Timer::Timer():time(clock()){
};

void Timer::done(char * msg){
  printf("Done %s in %f sec.", msg, (float)(clock()-time)/(float)CLOCKS_PER_SEC);
};

//Populate an array with test data
FastVolume::t_vox * generateData(FastVolume::t_vox * data){
  for(int i = 0; i < 0x100; i++)
    for(int j = 0; j < 0x100; j++)
      for(int k = 0; k < 0x100; k++){
	data[i+0x100*j+0x100*0x100*k] = 100*100-((128-i)*(128-i)+(128-j)*(128-j)+(128-k)*(128-k));
      };
  return data;
};

int main(int argc, char ** argv)
{
  printf("Allocating test data.\n");
  FastVolume::t_vox * data = new FastVolume::t_vox[0x1000000];
  if(!data){
    printf("Unable to allocate volume\n"); exit(1);
  };
  printf("Generating test data.\n");
  //generateData(data);

  printf("Creating volume.\n");
  FastVolume vol;
  vol.copy(generateData(data), 0x100, 0x100, 0x100);

  printf("Removing test data.\n");
  delete[] data;
  vector<int> out;

  Timer find_surface;
  vol.findSurface(out, 1);
  printf("Found %d points\n", out.size());
  find_surface.done("looking for points");
    
  printf("Test succeeded again!\n");
  return 0;
};
