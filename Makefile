# -------- toolchain --------
CXX      := g++
CC       := gcc
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -DLUMEN_HAVE_BULLET=1 -Iinclude -Iinclude/stb -Isrc -Isrc/core -Isrc/render -Isrc/scene
CFLAGS   := -O2 -Wall -Wextra -Iinclude -Iinclude/stb -Isrc -Isrc/core -Isrc/render -Isrc/scene

# -------- platform / linking --------
LDFLAGS :=
LDLIBS  := -lglfw -lGL -ldl -lpthread -lBulletDynamics -lBulletCollision -lLinearMath

UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
    LDLIBS := -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
endif
ifeq ($(OS),Windows_NT)
    LDLIBS := -lglfw3 -lopengl32 -lgdi32 -luser32
endif

# -------- sources --------
SRC    := src/core/asset.cpp src/core/collision.cpp src/main.cpp src/render/framebuffer.cpp src/render/mesh.cpp src/render/pass.cpp src/render/texture.cpp src/scene/glb_loader.cpp src/scene/map.cpp src/scene/probe_grid.cpp src/scene/tri_grid.cpp src/glad.c
OBJ    := $(SRC:.cpp=.o)
OBJ    := $(OBJ:.c=.o)

TARGET := lumen

# -------- rules --------
.PHONY: all clean run debug

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

src/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

debug: CXXFLAGS := -std=c++17 -O0 -g -fsanitize=address -Wall -Wextra -Iinclude -Iinclude/stb -Isrc -Isrc/core -Isrc/render -Isrc/scene
debug: CFLAGS   := -O0 -g -fsanitize=address -Wall -Wextra -Iinclude -Iinclude/stb -Isrc -Isrc/core -Isrc/render -Isrc/scene
debug: LDFLAGS  += -fsanitize=address
debug: clean all

clean:
	rm -f $(OBJ) $(TARGET)
