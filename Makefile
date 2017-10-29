# LlaMidi
#
#	Scott Lawrence
#	yorgle@gmail.com

# 
# Copyright (C) 2017 Scott Lawrence
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject
# to the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# 


############################################################
# target and source filenames

TARG := lmt 

SRCS := src/main.c \
	src/llamidi.c 


####################
# general build flags

LDFLAGS += -g
DEFS += 
CFLAGS += $(DEFS) -Wall -pedantic
LIBS += 
INCS += 


############################################################
# the primary target
# stay on target

all: $(TARG)
.PHONY: all


############################################################
# build rules

# automatically create the object file list
OBJS := $(SRCS:src/%.c=build/%.o)

# link
$(TARG): $(OBJS)
	@echo link $@
	@$(CC) $(CFLAGS) $^ $(LDFLAGS) -v $(LIBS) -o $@


# all of the source files get their object files built
# into the "build" directory

# compile source to object files
build/%.o: src/%.c
	@echo build -- $(CC) $<	
	@$(CC) $(CFLAGS) $(DEFS) $(INCS) -c -o $@ $<

$(OBJS): build


# and the directory we put object files into
build:
	@echo Making Build Directiory
	@mkdir build/


############################################################
# grab some midi files to test with 
# - I'm doing it this way, rather than including them in the
#   repository to mitigate/eliminate legal entanglements

midifiles: doomsongs mariosongs tetrisongs
.PHONY: midifiles


####################
# from http://karthik82.tripod.com/mus_midi_doom.htm

tetrisongs:
	@mkdir -p songs/tetris
	cd songs/tetris ; curl -O https://files.khinsider.com/midifiles/gameboy/tetris/music-a-2-.mid
	cd songs/tetris ; curl -O https://files.khinsider.com/midifiles/gameboy/tetris/music-b.mid
	cd songs/tetris ; curl -O https://files.khinsider.com/midifiles/gameboy/tetris/music-c.mid
	cd songs/tetris ; curl -O https://files.khinsider.com/midifiles/gameboy/tetris/title-screen.mid
.PHONY: tetrisongs


####################

mariosongs:
	@mkdir -p songs/mario
	cd songs/mario; curl -O http://www.midishrine.com/midipp/gmb/Super_Mario_Land/overworld.mid
	cd songs/mario; curl -O http://www.midishrine.com/midipp/gmb/Super_Mario_Land/overworld2.mid
	cd songs/mario; curl -O http://www.midishrine.com/midipp/gmb/Super_Mario_Land/pipe.mid
.PHONY: mariosongs


####################

doomsongs: songs/doom-doom2-midi.zip
.PHONY: doomsongs

songs/doom-doom2-midi.zip:
	@mkdir -p songs/
	cd songs; curl -O http://karthik82.tripod.com/doom-doom2-midi.zip
	cd songs; unzip -o doom-doom2-midi.zip
	@mkdir -p songs/doom
	cd songs/doom ; unzip -o ../doom-midi.zip ; rm -f ../doom-midi.zip
	@mkdir -p songs/doom2
	cd songs/doom2 ; unzip -o ../doom2-midi.zip ; rm -f ../doom2-midi.zip

RETRIEVEDFILES += songs/doom-doom2-midi.zip


############################################################
# utility targets

clean: 
	@echo removing all build files.
	@rm -rf build $(OBJS) $(TARG) $(TARGS)
.PHONY: clean

clobber: clean
	@echo removing all retrieved files.
	@echo (leaving all .MID files though...)
	@rm -rf $(RETRIEVEDFILES)
.PHONY: clobber


# for the test targets, make sure the songs exist
# first by running 'make midifiles'
test: test1
.PHONY: test

test1: $(TARG)
	./$(TARG) songs/mario/overworld.mid
.PHONY: test1

test2: $(TARG)
	./$(TARG) songs/doom/e1m1.mid 
.PHONY: test2

test3: $(TARG)
	./$(TARG) songs/doom2/MAP01.MID
.PHONY: test3

