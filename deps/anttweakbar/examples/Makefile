####### Compiler, tools and options

#---- MinGW
#MINGWFLAGS	= -mno-cygwin


#---- Release
CXXCFG   	= -O3
LFLAGS		= ../../glfw/lib/macosx/libglfw.a ../src/libant.a -framework AGL -framework OpenGL -framework Carbon -lz 
OUT_DIR		= bin
#---- Debug
#CXXCFG   	= -g -D_DEBUG
#LFLAGS		= -Wl -L../lib/debug
#OUT_DIR	= debug


CXX      	= g++
#CXXFLAGS 	= $(CXXCFG) $(MINGWFLAGS) -pipe -Wall -ffast-math -fPIC -fno-strength-reduce -march=i586 
CXXFLAGS 	= $(CXXCFG) -Wall -march=i386
INCPATH  	= -I../include -I/usr/local/include -I/usr/X11R6/include -I/usr/include
LIBS     	= -lpthread -lm

DEL_FILE 	= rm -f
DEL_DIR  	= rmdir
NO_STDERR	= 2> /dev/null
EXP_PATH	= "export LD_LIBRARY_PATH=\"../../lib:$(LD_LIBRARY_PATH)\""


####### Files


SRC_FILES	= TwSimpleGLFW.c TwSimpleGLUT.c TwSimpleSDL.c TwAdvanced1.cpp TwString.cpp


####### Build rules


#first:	depend all
first:  all

all: 	Makefile $(SRC_FILES)

	@echo "===== Build TwSimpleGLFW ===="
	$(CXX) $(CXXFLAGS) $(INCPATH) TwSimpleGLFW.c $(LFLAGS)  $(LIBS) -o $(OUT_DIR)/TwSimpleGLFW.out
	@echo "$(EXP_PATH) ; ./TwSimpleGLFW.out" > $(OUT_DIR)/TwSimpleGLFW

#	@echo "===== Build TwSimpleGLUT ===="
#	$(CXX) $(CXXFLAGS) $(INCPATH) TwSimpleGLUT.c $(LFLAGS) -lglut $(LIBS) -o $(OUT_DIR)/TwSimpleGLUT.out
#	@echo "$(EXP_PATH) ; ./TwSimpleGLUT.out" > $(OUT_DIR)/TwSimpleGLUT

#	@echo "===== Build TwSimpleSDL ===="
#	$(CXX) $(CXXFLAGS) $(INCPATH) TwSimpleSDL.c $(LFLAGS) -lSDL $(LIBS) -o $(OUT_DIR)/TwSimpleSDL.out
#	@echo "$(EXP_PATH) ; ./TwSimpleSDL.out" > $(OUT_DIR)/TwSimpleSDL

	@echo "===== Build TwAdvanced1 ===="
	$(CXX) $(CXXFLAGS) $(INCPATH) TwAdvanced1.cpp $(LFLAGS)  $(LIBS) -o $(OUT_DIR)/TwAdvanced1.out
	@echo "$(EXP_PATH) ; ./TwAdvanced1.out" > $(OUT_DIR)/TwAdvanced1

	@echo "===== Build TwString ===="
	$(CXX) $(CXXFLAGS) $(INCPATH) TwString.cpp $(LFLAGS) $(LIBS) -o $(OUT_DIR)/TwString.out
	@echo "$(EXP_PATH) ; ./TwString.out" > $(OUT_DIR)/TwString

# append dependencies to this Makefile
#depend:
#	@echo "===== Make dependencies ====="
#	makedepend -Y
#	makedepend -a -Y -- $(CXXFLAGS) $(INCPATH) -- $(SRC_FILES) $(NO_STDERR)


# clean temporary files
clean:
	@echo "===== Clean ====="
	-$(DEL_FILE) *.o
	-$(DEL_FILE) *~ core *.core *.stackdump
	-$(DEL_FILE) debug/*
	-$(DEL_DIR) debug

	
####### DEPENDENCIES

TwSimpleGLFW.o: ../include/AntTweakBar.h
TwSimpleGLUT.o: ../include/AntTweakBar.h
TwSimpleSDL.o:  ../include/AntTweakBar.h
TwAdvanced1.o:  ../include/AntTweakBar.h
TwString.o:     ../include/AntTweakBar.h
