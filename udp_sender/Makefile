INCLUDES = 
LIBS = 
DEBUG = -g

CXX = g++
CXXFLAGS = -O2 -Wall -m64 -pthread -D_REENTRANT $(INCLUDES)

all: udp_sender 

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

.cpp:
	$(CXX) $(CXXFLAGS) $< -o $@ 

BOOK_SRC = main.cpp 
BOOK_OBJ = $(addsuffix .o, $(basename $(BOOK_SRC)))

udp_sender: $(BOOK_OBJ)
	$(CXX) $(CXXFLAGS) -masm=intel -o $@ $(BOOK_OBJ)

clean:
	rm -f $(BOOK_OBJ) udp_sender
