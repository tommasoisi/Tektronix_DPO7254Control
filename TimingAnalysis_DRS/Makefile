ROOT=`root-config --cflags --libs`

LIB_DIRECTORY=TimingAnalysis
LIB_NAME=TimingAnalysis
CHECKFORUPDATE=$(LIB_DIRECTORY)/$(LIB_NAME).a $(LIB_DIRECTORY)/include/$(LIB_NAME).h $(LIB_DIRECTORY)/include/timingAlgorithm.h

progs=$(LIB_DIRECTORY)/$(LIB_NAME).a analyzeDataVsMCP

all:$(progs)

$(LIB_DIRECTORY)/$(LIB_NAME).a: $(objects:%=$(LIB_DIRECTORY)/src/%) $(LIB_DIRECTORY)/include/timingAlgorithm.h
	$(MAKE) -C $(LIB_DIRECTORY)

analyzeDataVsMCP:%:%.cxx $(CHECKFORUPDATE)
	g++ -o $@ $< -I$(LIB_DIRECTORY)/include  $(LIB_DIRECTORY)/$(LIB_NAME).a $(ROOT) -z muldefs -O3 -std=c++1y

clean:
	rm -r analyzeDataVsMCP $(LIB_DIRECTORY)/*.a $(LIB_DIRECTORY)/src/*.o
