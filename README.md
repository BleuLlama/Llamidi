# Llamidi
A low memory footprint MIDI file reader

2017-10-27 This is a work in progress.  It is initially for a project
for Rochester Mini Maker Faire, November 2017.  It is initially for
a project for Rochester Mini Maker Faire, November 2017.

Llamidi is a standard MIDI file (SMF) reader for use in embedded
systems... well, Arduinos, for reading in SMFs, and sending out
their content in appropriate time intervals.  It is written in C,
and made for use with Arduino, but could easily be used for other
platform applications if you set your mind to it.

The basic idea was that we don't need or want to load the SMF into
memory.  Instead we work on it as a kind of stream, only reading
in the bytes necessary for "now" during playback of the SMF.  Because
of this we only need to maintain minimal data about the contents
of the file, and the current position in each track stored in the
file.

The current implementation limits parsing to 16 tracks.

The basic codebase in this project is a command line utility (for
OS X) that gets built using standard tools, but also to be included
soon is a simple Arduino project that plays a SMF using the standard
SD library.

If I have time, I'll wrap this in an Arduino library repository so
that it can easily be added to your projects, but until then it
will just be one Arduino project.

Future possibilities would be to record midi data to SMF, but that
functionality is not planned.

# Building

You should be able to just type 'make' to have it build.  There are
a few test targets 'test1', 'test2', 'test3' which rely on having
some midi files.  If you type 'make midifiles' and have Curl
installed, it will retrieve a bunch of MIDI files from the net to
operate on.
