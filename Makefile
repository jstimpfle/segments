GLSLPROC = ../glsl-processor/glsl-processor

AUTOGEN_FILES = \
	autogenerated/shaders.h \
	autogenerated/shaders.c

GLSL_FILES = \
	glsl/line.vert \
	glsl/line.frag

C_FILES = \
	src/main.c \
	src/memoryalloc.c \
	src/opengl.c \
	src/openglwindow_x11.c \
	src/window.c \
	src/logging.c \

OBJECTS = $(C_FILES:%=BUILD/%.o) BUILD/autogenerated/shaders.c.o

CFLAGS += -D_POSIX_C_SOURCE=200809L
CFLAGS += -std=c99
CFLAGS += -Iautogenerated
CFLAGS += -Iinclude
CFLAGS += -Wall
CFLAGS += -g

CFLAGS += $(shell pkg-config --cflags x11)
CFLAGS += $(shell pkg-config --cflags gl)
CFLAGS += $(shell pkg-config --cflags glu)

LDFLAGS += -lm
LDFLAGS += $(shell pkg-config --libs x11)
LDFLAGS += $(shell pkg-config --libs gl)
LDFLAGS += $(shell pkg-config --libs glu)

all: directories segments

clean:
	rm -rf segments BUILD autogenerated/

directories:
	@mkdir -p BUILD/src
	@mkdir -p BUILD/autogenerated

$(AUTOGEN_FILES): shaders.link $(GLSL_FILES)
	$(GLSLPROC) shaders.link autogenerated/

BUILD/%.c.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

segments: $(AUTOGEN_FILES) $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

