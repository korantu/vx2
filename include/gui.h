#ifndef __GUI_H__
#define __GUI_H__

#include "slices.h"
#include "gl_points.h"

void gui_start(slices *, GlPoints *);
void gui_draw();
void gui_resize(int w, int h);
void gui_stop();


void read_voxels(std::string in, GlPoints * pnt);


#endif // __GUI_H__
