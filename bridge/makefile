# mingw32-make -j $(nproc)

# 1. Compiler and flags settings
ASLOV    := ../../aslov/aslov/src/aslov
PERMUTATIONS := ../../permutations/permutations/src/permutations
CXX      := g++
#-O2 -Wall -Wextra -Wpedantic
CXXFLAGS := -O3 -DNDEBUG -DFINAL_RELEASE -Wall -std=c++23 -I$(ASLOV) -I$(PERMUTATIONS) $(shell pkg-config --cflags gtk+-3.0)
LIBS     := $(shell pkg-config --libs gtk+-3.0) -lcurl -lminizip-ng
TARGET   := bridge

ifeq ($(OS),Windows_NT)
    LDFLAGS += -mwindows
endif

# 2. Automatic file discovery
# Finds all .cpp files in the src directory, its subdirectories, and the ASLOV path
SRCS     := $(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard $(ASLOV)/*.cpp) $(wildcard $(PERMUTATIONS)/*.cpp)
# Replaces .cpp extensions with .o to generate the object files list
OBJS     := $(SRCS:.cpp=.o)

EXTRA_OBJS := $(TARGET)/icon/app.o

# 3. Main target (executed by default via `make`)
all: $(TARGET)

# Links the executable from the object files
$(TARGET): $(OBJS) $(EXTRA_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(EXTRA_OBJS) $(LDFLAGS) $(LIBS)

# Rule to compile each .cpp file into a .o file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 4. Auxiliary commands
# Cleans the project by removing temporary build files (run: `make clean`)
clean:
	rm -f $(OBJS) $(TARGET).exe

# Declares targets that do not represent actual files on disk
.PHONY: all clean
