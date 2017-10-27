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


#######################################################################
# Build options

TARG := lmt 

SRCS := src/main.c \
	src/llamidi.c 

####################
# general build rules

LDFLAGS += -g
DEFS += 
CFLAGS += $(DEFS) -Wall -pedantic
LIBS += 
INCS += 


################################################################################

all: $(TARG)
.PHONY: all

################################################################################

OBJS := $(SRCS:src/%.c=build/%.o)

$(TARG): $(OBJS)
	@echo link $@
	@$(CC) $(CFLAGS) $^ $(LDFLAGS) -v $(LIBS) -o $@


################################################################################

build/%.o: src/%.c
	@echo build -- $(CC) $<	
	@$(CC) $(CFLAGS) $(DEFS) $(INCS) -c -o $@ $<

$(OBJS): build

build:
	@echo Making Build Directiory
	@mkdir build/

################################################################################
# grab the midi files to test with from http://karthik82.tripod.com/mus_midi_doom.htm

midifiles: doomsongs mariosongs tetrisongs

####################

tetrisongs:
	@mkdir -p songs/tetris
	cd songs/tetris ; curl -O https://files.khinsider.com/midifiles/gameboy/tetris/music-a-2-.mid
	cd songs/tetris ; curl -O https://files.khinsider.com/midifiles/gameboy/tetris/music-b.mid
	cd songs/tetris ; curl -O https://files.khinsider.com/midifiles/gameboy/tetris/music-c.mid
	cd songs/tetris ; curl -O https://files.khinsider.com/midifiles/gameboy/tetris/title-screen.mid



####################

mariosongs:
	@mkdir -p songs/mario
	cd songs/mario; curl -O http://www.midishrine.com/midipp/gmb/Super_Mario_Land/overworld.mid
	cd songs/mario; curl -O http://www.midishrine.com/midipp/gmb/Super_Mario_Land/overworld2.mid
	cd songs/mario; curl -O http://www.midishrine.com/midipp/gmb/Super_Mario_Land/pipe.mid

####################

doomsongs: songs/doom-doom2-midi.zip

songs/doom-doom2-midi.zip:
	@mkdir -p songs/
	cd songs; curl -O http://karthik82.tripod.com/doom-doom2-midi.zip
	cd songs; unzip -o doom-doom2-midi.zip
	@mkdir -p songs/doom
	cd songs/doom ; unzip -o ../doom-midi.zip ; rm -f ../doom-midi.zip
	@mkdir -p songs/doom2
	cd songs/doom2 ; unzip -o ../doom2-midi.zip ; rm -f ../doom2-midi.zip

RETRIEVEDFILES += songs/doom-doom2-midi.zip

songs:
	@mkdir songs/

.PHONY: midifiles


################################################################################

clean: 
	@echo removing all build files.
	@rm -rf build $(OBJS) $(TARG) $(TARGS)

clobber: clean
	@echo removing all retrieved files.
	@rm -rf $(RETRIEVEDFILES)

test1: $(TARG)
	./$(TARG) songs/mario/overworld.mid

test2: $(TARG)
	./$(TARG) songs/doom/e1m1.mid 

test3: $(TARG)
	./$(TARG) songs/doom2/MAP01.MID
