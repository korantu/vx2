CXXFLAGS := -DUSE_TW -g -O2 -Wall -I../include  -I../deps/anttweakbar/include -I../deps/glfw/include -I../deps/glfw/include/GL 
TEST := ../deps/unit--/unit--.o
T_VOLUME := t_fastvolume.o fastvolume.o loader.o  
T_LOADER :=  loader.o t_loading.o fastvolume.o  
T_V3TOOLS :=  t_v3tools.o v3tools.o   
T_LOADER_Z :=  t_loader_z.o loader.o fastvolume.o  
T_GUI := navigator.o t_gui.o
VX2 := navigator.o v3tools.o fastvolume.o loader.o gl_points.o gl_wrapper.o vx2.o 

t_generic: t_generic.o $(TEST)
	$(CXX) t_generic.o $(TEST) -o t_generic

t_fastvolume: $(T_VOLUME)
	$(CXX) $(T_VOLUME) -o t_volume

t_loader: $(T_LOADER)
	$(CXX) $(T_LOADER) -lz -o t_loader

t_v3tools: $(T_V3TOOLS)
	$(CXX) $(T_V3TOOLS) -lz -o t_v3tools

t_loader_z: $(T_LOADER_Z)
	$(CXX) $(T_LOADER_Z) -lz -o t_loader_z

t_gui: $(T_GUI)
	g++ $(T_GUI) $(LINKFLAGS) -o t_gui

vx2: $(VX2)
	g++ $(VX2) $(LINKFLAGS) -o vx2

.PHONY: clean
clean:
	rm *.o  

tags:
	etags -e *.cpp ../include/*.h

tags_all:
	find ../ -name "*.cpp" -or -name "*.h" -or -name "*.c" | xargs etags -e







