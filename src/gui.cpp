#include "gui.h"
#include "native.h"

#include <AntTweakBar.h>

struct GuiContainer{
  GuiContainer(slices * _sl, GlPoints * _pnt);
  ~GuiContainer();

  void create(); // populate the container

  slices * sl; //2d crossection
  GlPoints * pnt; // collection of 3d points and volume

  TwBar * bar;

	       //3d options
  int level;
  int threshold;
  int generation;
  int amount;
  int iterations;

  //2d options
  int size;
  int scheme;
  int zoom;
  float coverage;


/* Helper functions */

static void TW_CALL get_level(void * value, void * UserData);
static void TW_CALL set_level(const void * value, void * UserData);
static void TW_CALL load_file( void * UserData);
static void TW_CALL save_file( void * UserData);
static void TW_CALL step( void * UserData);
static void TW_CALL get_size(void * value, void * UserData);
static void TW_CALL set_size(const void * value, void * UserData);
static void TW_CALL get_zoom(void * value, void * UserData);
static void TW_CALL set_zoom(const void * value, void * UserData);
static void TW_CALL get_coverage(void * value, void * UserData);
static void TW_CALL set_coverage(const void * value, void * UserData);
static void TW_CALL get_scheme(void * value, void * UserData);
static void TW_CALL set_scheme(const void * value, void * UserData);


};

/* global structure */

GuiContainer * the_gui = 0;

/* interface */

void gui_start(slices * sl, GlPoints * pnt){
  if( !TwInit(TW_OPENGL, NULL) )
    {
      // A fatal error occured	
      fprintf(stderr, "AntTweakBar initialization failed: %s\n", TwGetLastError());
      //    glfwTerminate();
      //return 1;
      return;
    }
  
  the_gui = new GuiContainer(sl, pnt);
the_gui->create(); //populate the thing.
};


void gui_draw(){
  TwDraw();
};

void gui_resize(int x, int y){
  TwWindowSize(x, y);
};

void gui_stop(){
  if(the_gui)delete the_gui;
  TwTerminate();
};

/* declarations for constructor */

/* Constructor */

GuiContainer::GuiContainer(slices * _sl, GlPoints * _pnt):
  sl(_sl), 
  pnt(_pnt), 
  level(3), 
  threshold(20), 
  generation(3), 
  amount(5), 
  iterations(2),
  size(100),
  scheme(1),
  zoom(1),
  coverage(0.25)
{
  bar = TwNewBar("Options");
};

void GuiContainer::create(){
  ///building bars is a different business..
  
  TwDefine(" GLOBAL help='Voxelbrain Voxel editor.' "); // Message added to the help bar.
  
  
  TwAddVarRW(bar, "", TW_TYPE_DOUBLE, &pnt->tw_pnt, " label='Point size' min=0.2 max=4 step=0.01 keyIncr=d keyDecr=D help='Size of the display points in relation to optimal' ");
    TwAddVarRW(bar, "", TW_TYPE_BOOLCPP, &pnt->tw_pnt_smooth, " label='Smooth points' keyIncr=v keyDecr=V help='Size of the display points. ' ");

    //display cursor
    TwAddSeparator(bar, "Position.", NULL);
    TwAddVarRW(bar, "", TW_TYPE_FLOAT, &pnt->cursor.x, " label='X' help='Cursor X' ");
    TwAddVarRW(bar, "", TW_TYPE_FLOAT, &pnt->cursor.y, " label='Y' help='Cursor Y' ");
    TwAddVarRW(bar, "", TW_TYPE_FLOAT, &pnt->cursor.z, " label='Z' help='Cursor Z' ");
    
    //allow to change
    TwAddSeparator(bar, "Operation.", NULL);
    TwAddVarCB(bar, "", TW_TYPE_FLOAT, GuiContainer::set_level, GuiContainer::get_level, NULL, " min=-1 max=150 step=1 label='Isovalue'");
    pnt->tw_cursor_hit = 3;
    TwAddVarRW(bar, "", TW_TYPE_FLOAT, &pnt->tw_cursor_hit, "  min=2 max=150 step=1 label='Hit point' help='Point where the cursor considered to have hit the surface' ");
    pnt->tw_cursor_depth = 0;
    TwAddVarRW(bar, "", TW_TYPE_FLOAT, &pnt->tw_cursor_depth, " min=0 max=50 step=1 label='Cursor depth' help='How deep cursor goes after an impact.' ");
    TwAddVarRO(bar, "", TW_TYPE_FLOAT, &pnt->tw_mri_value, " label='ValueAtCursor' help='MRI data value at the calcualtor.' ");
    TwAddSeparator(bar, "Editing.", NULL);
    TwAddVarRW(bar, "", TW_TYPE_INT32, &pnt->tool, " min=0 max=1 step=1 label='Editing mode' help='0-nop, 1-mark seeds.' ");
    TwAddVarRW(bar, "", TW_TYPE_INT32, &threshold, " min=3 max=100 step=1 label='Prop. threshold' help='what possible thresholds are avaliable' ");
    TwAddVarRW(bar, "", TW_TYPE_INT32, &amount, " min=1 max=10 step=1 label='Lookahead.' help='Skip this many voxels in search for suitable areas.' ");
    TwAddVarRW(bar, "", TW_TYPE_INT32, &iterations, " min=1 max=200 step=10 label='Iterations.' help='Iterations per button press' ");
    TwAddButton(bar, "", step, NULL, "label='Step' key='g'");
    TwAddSeparator(bar, "File.", NULL);
    TwAddButton(bar, "", load_file, NULL, "label='Load'");
    TwAddButton(bar, "", save_file, NULL, "label='Save'");

    ///and now the slice control:
    sl->tiles_coverage(0.25, 1.0);
    

    TwAddVarCB(bar, "", TW_TYPE_INT32, GuiContainer::set_size, GuiContainer::get_size, NULL, " min=20 max=300 step=5 label='Tile size' group='2D'");
    TwAddVarCB(bar, "", TW_TYPE_INT32, GuiContainer::set_zoom, GuiContainer::get_zoom, NULL, " min=1 max=5 step=1 label='Zoom' group='2D'");
    TwAddVarCB(bar, "", TW_TYPE_FLOAT, GuiContainer::set_coverage, GuiContainer::get_coverage, NULL, " min=0.25 max=1.0 step=0.03 label='Coverage' group='2D'");
    TwAddVarCB(bar, "", TW_TYPE_INT32, GuiContainer::set_scheme, GuiContainer::get_scheme, NULL, " min=0 max=4 step=1 label='Color scheme' group='2D'");


};

/* Destructor */
GuiContainer::~GuiContainer(){
};

/* Helper functions */

void TW_CALL GuiContainer::get_level(void * value, void * UserData){
  //printf("getting level %f\n", (float)level);
  (*((float *)value))=(float)(the_gui->level);
};
void TW_CALL GuiContainer::set_level(const void * value, void * UserData){
  the_gui->level = *((float *)value);
  printf("setting level %f\n", the_gui->level);
  the_gui->pnt->set_level((float)the_gui->level);
};

void TW_CALL GuiContainer::load_file( void * UserData){
  printf("Trying to load a file.\n");
  std::string in = getFile();
  if(in.length() > 0){
    printf("indeed, got %s\n", in.c_str());
    the_gui->pnt->load(in.c_str());
  };
};


void TW_CALL GuiContainer::save_file( void * UserData){
  printf("Trying to load a file.\n");
  std::string in = putFile();
  if(in.length() > 0){
    printf("indeed, got %s to save in.\n", in.c_str());
    the_gui->pnt->save(in.c_str());
  };
};

void TW_CALL GuiContainer::step( void * UserData){
  
  for(int i = 0; i < the_gui->iterations; i++){
    the_gui->pnt->vol.propagate(the_gui->threshold, the_gui->generation, the_gui->amount);
    the_gui->pnt->vol.updated = true;

    the_gui->generation %= 100; the_gui->generation ++; 
  };
  the_gui->pnt->vol.reseed();

};

//slices
void TW_CALL GuiContainer::get_size(void * value, void * UserData){
  (*((int *)value))=(int)(the_gui->size);
};

void TW_CALL GuiContainer::set_size(const void * value, void * UserData){
  the_gui->size = *((int *)value);
  printf("setting level %d\n", the_gui->size);
  the_gui->sl->resize_tile((int)(the_gui->size), (int)(the_gui->size));
};

void TW_CALL GuiContainer::get_zoom(void * value, void * UserData){
  (*((int *)value))=the_gui->sl->zoom;
};

void TW_CALL GuiContainer::set_zoom(const void * value, void * UserData){
  the_gui->sl->zoom = (*((int *)value));
  the_gui->sl->update();
};

//coverage
void TW_CALL GuiContainer::get_coverage(void * value, void * UserData){
  (*((float *)value))=the_gui->coverage;
};

void TW_CALL GuiContainer::set_coverage(const void * value, void * UserData){
  the_gui->coverage = (*((float *)value));
  the_gui->sl->tiles_coverage(the_gui->coverage, 1.0);
  the_gui->sl->update();
  
};

void TW_CALL GuiContainer::get_scheme(void * value, void * UserData){
  (*((int *)value))=(int)(the_gui->scheme);
};

void TW_CALL GuiContainer::set_scheme(const void * value, void * UserData){
  the_gui->scheme = *((int *)value);
  printf("setting scheme %d\n", the_gui->scheme);
  the_gui->sl->scheme = the_gui->scheme;
  the_gui->sl->update();
};






