CXX = g++
CXXFLAGS = $(shell pkg-config --cflags ncurses sndfile portaudio-2.0 jsoncpp) -I/opt/homebrew/include -Wall -Wextra -Wconversion -Wunreachable-code -std=c++17 -O3 -g
LDFLAGS = $(shell pkg-config --libs ncurses sndfile portaudio-2.0 jsoncpp) -I/opt/homebrew/lib
BINARY = agbdump

SRC_FILES := $(wildcard agbplay/src/*.cpp)
SRC_FILES := $(filter-out agbplay/src/agbplay.cpp, $(SRC_FILES))

OBJ_FILES = $(addprefix agbplay/obj/,$(notdir $(SRC_FILES:.cpp=.o)))

all: $(BINARY)

$(BINARY): $(OBJ_FILES) agbdump.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $^

agbplay/obj/%.o: agbplay/src/%.cpp agbplay/src/*.h
	mkdir -p agbplay/obj
	$(CXX) -c -o $@ $(CXXFLAGS) $(IMPORT) $<
