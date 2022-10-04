##---------------------------------------------------------------------
## COMPILER	// tested with gcc-12 & clang 14
##---------------------------------------------------------------------
# -mavx2 flag isn't essential

CXX = g++
#CXX = clang++

ifeq ($(CXX), g++)
	CXXFLAGS = -O3 -std=c++20 -static -mavx2 -pedantic -Wall -Wextra -Wformat
endif
ifeq ($(CXX), clang++)
	CXXFLAGS = -O3 -std=c++20 -static -mavx2 -pedantic -Wall -Wextra -Wformat -flto
endif

LIBS = -pthread

##---------------------------------------------------------------------
## SOURCES
##---------------------------------------------------------------------

EXE = bienchen
SOURCES = timer.cpp move.cpp core.cpp gen.cpp search.cpp uci.cpp main.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)

##---------------------------------------------------------------------
## PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	CFLAGS = $(CXXFLAGS)
endif
ifeq ($(OS), Windows_NT)
    ECHO_MESSAGE = "MinGW"
    CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD
##---------------------------------------------------------------------

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
