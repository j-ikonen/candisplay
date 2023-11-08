
TARGET  = CANDisplay
BINARY  = CANDisplay.bin
CC      = g++
SRCSFX  = .cpp

# SDL2 Flags
CFLAGS  := $(shell sdl2-config --cflags)
LDFLAGS := $(shell sdl2-config --libs)

CFLAGS  += -std=gnu++14 -pedantic -Wall -Wextra -Werror -pthread
LDFLAGS += -lSDL2_image -lSDL2_ttf
DBFLAGS += -g -DDEBUG
REFLAGS += -O3 -DNDEBUG

#CFLAGS  += -I/usr/local/include/SDL2 -D_REENTRANT
#LDFLAGS += -L/usr/local/lib -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -lSDL2
#-lSDL2
# -lSDL2main
LIBS    = 
#-L/usr/lib/x86_64-linux-gnu/ 
#-L/usr/lib -Llib
#`pkg-config sdl2 --cflags --libs`
RPFLAGS   = -march=armv7 -mfpu=vfp -mfloat-abi=hard
RPLDFLAGS = 
RPLIBS		= 

BUILDDIR  = build
SOURCEDIR = src
HEADERDIR = include 

MKDIR = mkdir -p
RM    = rm -rf
ECHO  = echo

SOURCES := $(shell find $(SOURCEDIR) -name '*$(SRCSFX)') 
OBJECTS := $(addprefix $(BUILDDIR)/,$(SOURCES:%$(SRCSFX)=%.o))

.PHONY: all setup clean raspi

# Default build
all: setup $(BINARY)

# Debug flags 
debug: CFLAGS += $(DBFLAGS)
debug: all

# Optimization flags
release: CFLAGS += $(REFLAGS)
release: all

raspi: CFLAGS  += $(RPFLAGS)
raspi: LDFLAGS += $(RPLDFLAGS)
raspi: LIBS    += $(RPLIBS)
raspi: all


$(BINARY): $(OBJECTS)
	$(CC) $(CFLAGS) $(LIBS) $(LDFLAGS) $(sdl2-config --cflags --libs) $(OBJECTS) -o $(BINARY) 

$(BUILDDIR)/%.o: %$(SRCSFX)
	$(CC)  $(CFLAGS) $(LIBS) $(LDFLAGS) -I$(HEADERDIR) -I$(dir $<) -c $< -o $@


setup: 
	$(MKDIR) $(BUILDDIR)/$(SOURCEDIR)

clean:
	$(RM) $(BINARY) $(BUILDDIR)