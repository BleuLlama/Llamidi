#include <stdio.h>

/* ref: http://www.ccarh.org/courses/253/handout/smf/
 * ref: https://www.csie.ntu.edu.tw/~r92092/ref/midi/
 * ref: http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html
*/

/*
    ==== Variable width values ====

	Delta time. 


	for the file, this is prepended to any midi 
	message to be sent out.  deltat
	

	(byte) data with or without 0x80 high bit set
		    0x80 - continues next byte
		    0x00 - end

	0x7f            0111 1111
			-111 1111
			0111 1111
			    -> 0x7F (127)

	0x81 0x7f       1000 0001  0111 1111
			-000 0001  -111 1111
			  00 0000  1111 1111
			    -> 0xFF (255)

	0x82 0x80 0x00  1000 0010  1000 0000  0000 0000
			-000 0010  -000 0000  -000 0000
			   0 0000  1000 0000  0000 0000
			    -> 0x8000 (32768)
		    

    -=[ MIDI EVENTS ]=-

	I think this is complete... as far as events for files anyway

	For this project, I don't need to parse any of this, or generate
	any of it.  But i do need to know how big the events are, and 
	they aren't all self-descriptive, so that's why we have some
	hardcoded bits in here.

    ==== meta events ====

	0xFF 0x00 0xLL (0xBB)	Sequence number
				    L=2, BB = 16 bit track ID 

	0xFF 0x01 0xLL (0xBB)	Text event (text)
	0xFF 0x02 0xLL (0xBB)	Copyright Notice (text)
	0xFF 0x03 0xLL (0xBB)	Sequence/Track Name (text)
	0xFF 0x04 0xLL (0xBB)	Instrument Name (text)
	0xFF 0x05 0xLL (0xBB)	Lyric Text (text)
	0xFF 0x06 0xLL (0xBB)	Marker Text (text)
	0xFF 0x07 0xLL (0xBB)	Cue Point (text)

	0xFF 0x20 0xLL (0xBB)	MIDI channel prefix assignment
				    L=1, BB =0..16 which midi channel

	0xFF 0x2F 0xLL (0xBB)	End of track
				    Required

	0xFF 0x51 0xLL (0xBB)	Tempo Setting
				    L=3
				    microseconds per quarter note
				    120BPM = 50 00 00

	0xFF 0x54 0xLL (0xBB)	SMPTE offset
				    L=5
				    HH MM SS FR FF

	0xFF 0x58 0xLL (0xBB)	Time Signature
				    L=4
				    NN DD CC BB
				    NN/DD = time signature 3/4 etc
				    CC = midi clocks/metronome tick
				    BB = nr 1/32nd notes per 24 midi clks
					    (8 = standard)

	0xFF 0x59 0xLL (0xBB)	Key signature
				    L=2
				    sf = nr sharps or flats
					-7 = 7 flats
					 0 = key of C
					+7 = 7 sharps
				    mi => 0=major, 1=minor

	0xFF 0x7F 0xLL (0xBB)	Sequencer specific event
				    L = ID+Data
				    BB = ID, DATA
				    ID 

    ==== System exclusive event ====
	0xF0 0xLL (0xBB) 		send F0 + data

	0xF7 0xLL (0xBB) 		send data

    ==== Channel Voice ====
	0x8n kk vv	Note Off
			    n = channel 0..F (0=ch 1)
			    kk = key released
				    3C = middle C
				    1 = 1/2 step
			    vv = velocity
				    00 = min, 40 = default, 7F = max
				    1 = ppp, 40 = mf, 7F = fff
    
	0x9n kk vv	Note On
			    (see 0x8n)
			    V=0 => same as (8n kk 40)

	0xAn kk ww	Aftertouch, kk=note on key, ww=pressure

	0xBn cc nn	Controller Change - footswitch/pedal/slider 
			    cc = controller numnber
			    nn = value 0..7F

	0xCn pp		Program Change (instrument change
			    0 = 1st program

	0xDn ww		Key Pressure/Aftertouch, ww=pressure

	0xEn lsb msb	Pitch Bend
			    center => lsb=0, msb=40

    ==== Channel Mode ====

	0xBn 0x78 0x00	Turn off all sound generation
	0xBn 0x79 0x00	Reset all controllers to defaults

	0xBn 0x7A 0x00	Disconnect keyboard to sound generator
	0xBn 0x7A 0x7F	Reconnect keyboard to sound generator

	0xBn 0x7B 0x00	All notes off (ignored if omni on)

	0xBn 0x7C 0x00	Omni mode off
	0xBn 0x7D 0x00	Omni mode on

	0xBn 0x7E  mm	Mono mode on 0=use n..16, 1=use 1, x10 = use 16
	0xBn 0x7F 0x00	Poly mode on
*/

/* bytes for transmission: (or for recording, i suppose)

	$command = read1

	>> 0xFF Meta commands are: (FF + Cmd + Counter + Data)
		send $command
		read 1 byte -> send
		read 1 byte -> send , store as $counter
		read $counter bytes -> send
	    
	>> 0xF0 System Exclusive: (F0 + Counter + Data)
		send $command
		read 1 byte -> send , store as $counter
		read $counter bytes -> send

	>> 0xF7 System Exclusive: (FF + Counter + Data)
		(do not send $command)
		read 1 byte -> send , store as $counter
		read $counter bytes -> send

	>> Channel Voice-2 Commands: Cx, Dx: (Yx + Byte)
		send $command
		read 1 byte -> send

	>> Channel Voice-3 Commands: 8x, 9x, Ax, Bx, Ex: (Yx + Byte + Byte)
		send $command
		read 1 byte -> send
		read 1 byte -> send

	>> Channel Mode-3 Commands: Bx (fold into Voice-3) (Yx + Byte + Byte)
		send $command
		read 1 byte -> send
		read 1 byte -> send
*/

	/* commands we DO Need to pay attention to: */
/*
	0xFF 0x2F 0xLL (0xBB)	(FF 2F 00)  - end of track
	0xFF 0x51 0xLL (0xBB)	Tempo Setting
	0xFF 0x58 0xLL (0xBB)	Time Signature (for display)
	0xFF 0x59 0xLL (0xBB)	Key signature (for display)
*/


/* --- File IO tools ------------------------------------------ */ 

// Macro version:

#define MIDI_READ1( F ) \
	(fgetc(F) & 0x00ff)

#define MIDI_READ2( F ) \
	((MIDI_READ1(F) << 8) + MIDI_READ1(F))

#define MIDI_READ4( F ) \
	((MIDI_READ2(F) << 16) + MIDI_READ2(F))
	


/*
// Function version:

unsigned char MIDI_Read1( FILE * fp )
{
    // or:  (fgetc( fp ) & 0x00FF)
    unsigned char v = 0;
    (void)fread( &v, 1, 1, fp );
    return v;
}

unsigned int MIDI_Read2( FILE * fp )
{
    unsigned int v = 0;
    (void)fread( &v, 1, 1, fp );
    v <<= 8;
    (void)fread( &v, 1, 1, fp );
    return v;
}

unsigned long MIDI_Read4( FILE * fp )
{
    unsigned long v = 0;
    (void)fread( &v, 1, 1, fp );
    v <<= 8;
    (void)fread( &v, 1, 1, fp );
    v <<= 8;
    (void)fread( &v, 1, 1, fp );
    v <<= 8;
    (void)fread( &v, 1, 1, fp );

    return v;
}
*/

/* let's use the macro versions... */
#define MIDI_Read1 	MIDI_READ1
#define MIDI_Read2 	MIDI_READ2
#define MIDI_Read4 	MIDI_READ4



/* Fpeek
 *	Look at the next character in the file
 */
int MIDI_Fpeek( FILE * fp )
{
    int c;
    c = fgetc( fp );
    ungetc( c, fp );
    return c;
}

/* MIDI_ReadV
 *
 *	read in a midi variable length value.
 *	the high bit set means there's another byte.
 *	the low 7 bits are the valid value data
 */
unsigned int MIDI_ReadV( FILE * fp )
{
    unsigned int acc = 0;
    unsigned char val = 0;

    while( 1 ) {
	/* error if we hit EOF */
	if( EOF == MIDI_Fpeek( fp ) ) {
	    return acc;
	}

	/* read a byte */
    	val = MIDI_Read1( fp );

	/* add it to the accumulator */
	acc |= (val & 0x7F );

	/* if the high bit is clear, 
		this is the last byte, return the accumulator 
	*/
	if( 0 == (val & 0x80) ) {
	    return acc;
	}
	/* otherwise, shift the accumulator over, and continue */
	acc <<= 7;
	/* and for good measure, clear the bottom 7 for the next byte */
	//acc &= 0x7f;
    }
}


/* --- MIDI File processor ------------------------------------ */ 

char tbuf[32];

char * MIDI_ToString( unsigned char command )
{
    unsigned char voice = command & 0x0F;
    sprintf( tbuf, "? (x%02x)", command );

    switch( command & 0xF0 ) {
    case( 0x80 ): sprintf( tbuf, "v%02d Note Off   3 ", voice ); break;
    case( 0x90 ): sprintf( tbuf, "v%02d Note On    3 ", voice ); break;
    case( 0xa0 ): sprintf( tbuf, "v%02d Aftertouch 3 ", voice ); break;
    case( 0xb0 ): sprintf( tbuf, "v%02d Ctrl/Mode  3 ", voice ); break;
    case( 0xc0 ): sprintf( tbuf, "v%02d Prog Chng  2 ", voice ); break;
    case( 0xd0 ): sprintf( tbuf, "v%02d Chan Pres  2 ", voice ); break;
    case( 0xe0 ): sprintf( tbuf, "v%02d Pitch Whl  3 ", voice ); break;
    case( 0xF0 ): sprintf( tbuf, "System Exclusive V " ); break;
    }
    return tbuf;
}



/* --- MIDI File processor ------------------------------------ */ 

/* Files are stored as:
	[Header Chunk] [Track Chunk] ([Track Chunk] ...)

    Chunks are like IFF chunks.

    A chunk conisists of:
	4 byte header (eg 'MThd')
	4 byte length of data (not including header and length)
	N bytes of data (N can = 0)
*/

/* block types from the MIDI SMF Spec */
#define kMIDIChunk_Header	(0x4d546864)	/* MThd */
#define kMIDIChunk_Track	(0x4d54726b)	/* MTrk */


#define kLMF_NumTracks	(16)

#define kLMF_ERR_UnsupportedTime 	(-6)
#define kLMF_ERR_UnsupportedFile	(-5)
#define kLMF_ERR_BadChunk	(-4)
#define kLMF_ERR_BadFile	(-3)
#define kLMF_ERR_File     (-2)
#define kLMF_ERR_FileRead (-1)
#define kLMF_ERR_None     (0)
#define kLMF_ERR_Ready    (1)

typedef struct llamidifile {
    /* file parse error */
    int error;	

    FILE * fp; /* file - adjust for SD library */

    unsigned long trackStarts [kLMF_NumTracks]; /* track starts in file pos */

    /* runtime variables */
    unsigned long trackPositions [kLMF_NumTracks]; /* current fp pos/track */
    unsigned long trackTime [kLMF_NumTracks]; /* set via parsed & millis() */

    int divis; /* file timing */
    unsigned char fileTracks;
} LMF;

char * LMF_Error( LMF * lmf )
{
    if( !lmf ) return "Null handle";
    switch( lmf->error ){
    case( kLMF_ERR_None ): 		return "None";
    case( kLMF_ERR_Ready ): 		return "Ready";
    case( kLMF_ERR_FileRead ): 		return "File read";
    case( kLMF_ERR_File ): 		return "File";
    case( kLMF_ERR_BadFile ): 		return "Bad file data";
    case( kLMF_ERR_BadChunk ): 		return "Bad file chunk";
    case( kLMF_ERR_UnsupportedFile ):	return "Unsupported SMF";
    case( kLMF_ERR_UnsupportedTime ):	return "Unsupported timing";
    }
    return "Uknown?";
}

int LMF_Okay( LMF * lmf )
{
    if( !lmf ) return 0;
    if( lmf->error >= kLMF_ERR_None ) return 1;
    return 0;
}



/* our process routine:
    OpenLMF()
	- open file, populate llamidifile struct
	- parse in header
 	- seek to each track, store track positions
	- set trackTime to first items on each track (now + (readValue))
	- set trackTime to FFFFFFFF for unused tracks
*/

LMF * LMF_Initialize( LMF * lmf )
{
    int i;
    if( !lmf ) return NULL;

    lmf->error = kLMF_ERR_None;
    lmf->fp = NULL;

    for( i=0 ; i < kLMF_NumTracks ; i++ ) {
	lmf->trackStarts[i] = 0l;
	lmf->trackPositions[i] = 0l;
	lmf->trackTime[i] = 0l;
    }

    lmf->divis = 0;

    return lmf;
}

void LMF_SeekFilePos( LMF * lmf, int track )
{
    if( !LMF_Okay( lmf )) return;
    if( track <0 || track >= kLMF_NumTracks ) return; 

    fseek( lmf->fp, lmf->trackPositions[ track ], SEEK_SET );
}

void LMF_StoreFilePos( LMF * lmf, int track )
{
    if( !LMF_Okay( lmf )) return;
    if( track <0 || track >= kLMF_NumTracks ) return; 

    lmf->trackPositions[ track ] = ftell( lmf->fp );
}

void LMF_Rewind( LMF * lmf )
{
    int i;

    if( !LMF_Okay( lmf )) return;

    for( i=0 ; i<kLMF_NumTracks ; i++ ) {

	if( lmf->trackStarts[i] > 0 ) {
	    // rewind the pointers
	    lmf->trackPositions[i] = lmf->trackStarts[i] + 8;
	    // +8 to skip the track header

	    // preload time delta
	    LMF_SeekFilePos( lmf, i );
	    lmf->trackTime[i] = MIDI_ReadV( lmf->fp );
	    LMF_StoreFilePos( lmf, i );
	}
    }
}

LMF * LMF_Open( LMF * lmf, const char * filename )
{
    unsigned long chunk_identifier = 0L;
    unsigned long chunk_length = 0L;
    unsigned long chunk_nextPos = 0L;
    unsigned int  data2;
    //unsigned char data1;
    unsigned char track = 0;

    if( !lmf ) return NULL;

    /* initialize the struct */
    lmf = LMF_Initialize( lmf );

    /* simple checks */
    if( !filename ) {
	lmf->error = kLMF_ERR_File;
	return lmf;
    }

    /* open the file, if fail, just return */
    lmf->fp = fopen( filename, "r" );
    if( !lmf->fp ) {
	lmf->error = kLMF_ERR_FileRead;
	return lmf;
    }
    
    lmf->error = kLMF_ERR_Ready;

    /* okay.  let's zip through the header to confirm we're ok */
    fseek( lmf->fp, 0, SEEK_SET );

    /* read the chunk identifier and size (chunk header) */
    chunk_identifier = MIDI_Read4( lmf->fp );
    chunk_length = MIDI_Read4( lmf->fp );
    chunk_nextPos = ftell( lmf->fp) + chunk_length; // start of tracks

    printf( "++ Chunk ID: 0x%08lx  (%ld bytes)\n", 
	    chunk_identifier, chunk_length );

    // so now, if the chunk_identifier needs to be 
    if( chunk_identifier != kMIDIChunk_Header ) {
	lmf->error = kLMF_ERR_BadFile;
	return lmf;
    }

    if( chunk_length != 6 ) {
	lmf->error = kLMF_ERR_BadChunk;
	return lmf;
    }

    // midi format
    data2 = MIDI_Read2( lmf->fp );
	// 0 = single track - OK
	// 1 = multiple track - OK
	// 2 = multiple song - NOT SUPPORTED
    if( data2 != 0 && data2 != 1 ) {
	lmf->error = kLMF_ERR_UnsupportedFile;
	return lmf;
    }

    // number of track chunks
    lmf->fileTracks = MIDI_Read2( lmf->fp );
    printf( "++ %d Track Chunks in the file\n", data2 );
    if( lmf->fileTracks > kLMF_NumTracks ) {
	printf( "++ NOTICE only %d will be read.\n", kLMF_NumTracks );
	lmf->fileTracks = kLMF_NumTracks;
    }
    
    // divis - timing
	// >0 = units per beat
	// <0 = SMPTE units (unsupported)
    lmf->divis = (signed) MIDI_Read2( lmf->fp );
    if( lmf->divis < 0 ) {
	lmf->error = kLMF_ERR_UnsupportedTime;
	return lmf;
    }
    printf( "++ %d units per beat\n", lmf->divis );

    // okay.  now let's populate the track info

    // kickstart it with the first track position...
    lmf->trackStarts[0] = chunk_nextPos;

    // and prep our counter
    track = 0;
    do {
	// go to the beginning of this track
	fseek( lmf->fp, (int)lmf->trackStarts[ track ], SEEK_SET );

	// read the chunk identifier 
	chunk_identifier = MIDI_Read4( lmf->fp );

	// make sure we've seeked to anotehr chunk
	if( chunk_identifier != kMIDIChunk_Track ) {
	    break;
	}

	// read the chunk length
	chunk_length = MIDI_Read4( lmf->fp );

	// calculate the start of the next chunk position
	chunk_nextPos = ftell( lmf->fp) + chunk_length;
	// (the end of this chunk)
	
	// inc the position to store it...
	track++;
	if( track < lmf->fileTracks ) {
	    // let's store it, and repeat for the next one.
	    lmf->trackStarts[track] = chunk_nextPos;
	}
	
    } while( track < kLMF_NumTracks );

    /* call this to reset the file pointers,
	prep positions, and delays */
    LMF_Rewind( lmf );

    return lmf;
}

LMF * LMF_Close( LMF * lmf )
{
    if( lmf && lmf->fp ) fclose( lmf->fp );
    return LMF_Initialize( lmf );
}


void LMF_Dump( LMF * lmf )
{
    int i;
    printf( "\n" );
    if( !lmf ) {
	printf( "LMF: Null.\n" );
	return;
    }
    printf( "LMF     fp: %s\n", (lmf->fp)?"open":"closed" );
    printf( "LMF  error: %s (%d)\n", LMF_Error( lmf ), lmf->error );
    printf( "LMF   okay: %s\n", LMF_Okay( lmf )?"Yes":"No" );
    printf( "LMF tracks: %d\n", lmf->fileTracks );
    printf( "LMF  divis: %d\n", lmf->divis );
    printf( "LMF  \t%10s\t%10s\t%10s\n", "start", "pos", "time" );

    for( i=0 ; i<kLMF_NumTracks ; i++ ) {
	if( lmf->trackStarts[i] > 0 ) {
	    printf( "%5d\t%10ld\t%10ld\t%10ld\n",
		    i, 
		    lmf->trackStarts[i],
		    lmf->trackPositions[i],
		    lmf->trackTime[i] );
	}
    }
    printf( "LMF  current opcodes:\n" );
    for( i=0 ; i<kLMF_NumTracks ; i++ ) {
	if( lmf->trackStarts[i] > 0 ){
	    unsigned char opcode = 0;

	    LMF_SeekFilePos( lmf, i );
	    opcode = MIDI_Read1( lmf->fp );
	    printf( "     %02d: %s\n", i, MIDI_ToString( opcode ));
	}
    }
}

void test( const char * filename )
{
    LMF lmf;
    LMF *lmfh;
    
    lmfh = LMF_Open( &lmf, filename );

    if( LMF_Okay( lmfh ) ) {
	printf( "Okay: %s\n", LMF_Error( lmfh ));
    } else {
	printf( "Fail: %s\n", LMF_Error( lmfh ));
    }
    LMF_Dump( lmfh );

    // do stuff here 

    lmfh = LMF_Close( lmfh );
}

/*
    PlayPoll()
	- foreach track
	    - while( trackTime <= current )
		- parse/send event
		- if "end of track" set trackTime = 0xFFFFFFFF ;
		- parse next time event
*/

int process( const char * filename )
{
    long len;
    unsigned long chunk_identifier;
    unsigned long chunk_length;
    unsigned int  val16;
    unsigned char val8;
    unsigned int  deltaTime;
    unsigned char command;
    unsigned int  byteCount;
    long chunk_nextPos = 0;

    FILE * fp = fopen( filename, "rb" );

    if( !fp ) {
	printf( "Unable to open file.\n" );
	return 0;
    }

    fseek( fp, 0, SEEK_END );
    len = ftell( fp );
    fseek( fp, 0, SEEK_SET );

    printf( "File is %ld bytes\n", len );

    do {
	fseek( fp, chunk_nextPos, SEEK_SET );
	printf( "\n" );

	if( EOF == MIDI_Fpeek( fp ) ) {
	    printf( "EOF.\n" );
	    return 0;
	}
	
	/* read the chunk identifier and size (chunk header) */
	chunk_identifier = MIDI_Read4( fp );
	chunk_length = MIDI_Read4( fp );

	printf( "Chunk ID: 0x%08lx  (%ld bytes)\n", 
		chunk_identifier, chunk_length );

	// set the next pos so we can easily skip, in case...
	chunk_nextPos = ftell(fp) + chunk_length;

	if( chunk_identifier == kMIDIChunk_Header ) { 
	    printf( " -- Header\n" ); 
	    /*
		Header:
		MThd	4 bytes - identifier - always 0x4d546864
		   6	4 bytes - block size - always 6
		fmt 	2 bytes	- file format
				0 - single track
				1 - multiple track
				2 - multiple song
		n	2 bytes - number of track chunks that follow
		divis	2 bytes	- unit of time for delta timing
				positive - units per beat.
				negative - SMPTE units 
	    */
	    
	    val16 = MIDI_Read2( fp );
	    printf( "    Format: %d - ", val16 );
	    if( val16 == 0 ) printf( "single track format\n" );
	    if( val16 == 1 ) printf( "multiple track format\n" );
	    if( val16 == 2 ) printf( "multiple song format\n" );

	    val16 = MIDI_Read2( fp );
	    printf( "    Track Chunks: %d\n", val16 );

	    val16 = MIDI_Read2( fp );
	    printf( "    Division: %d ", (signed) val16 );
	    if( ((signed) val16) > 0 ) {
		printf( "(units per beat)\n" );
	    } else {
		printf( "(SMPTE compatible units)\n" );
	    }

	    val8 = MIDI_Read1( fp );
	}
	else if ( chunk_identifier == kMIDIChunk_Track ) {
	    printf( " -- Track\n  " ); 

	    while( 1 ) {
		// timestamp
		printf( "\n" );

		printf( "(%ld) ", ftell( fp ));
		deltaTime = MIDI_ReadV( fp );  printf( "0x%04x: ", deltaTime );
		
		command = MIDI_Read1( fp ); 

		printf( "%02x %s: ", command, MIDI_ToString( command ));

		switch( command & 0xF0 ) {
		case( 0xF0 ):
		    // FF   send cmd   read & send c2   read & send counter+data
		    // F0   send cmd                    read & send counter+data
		    // F7                               read & send counter+data
		    printf( "%02x", command );
		    if( command == 0xff || command == 0xF0 ) {
			// forward command 
		    }
		    if( command == 0xff ) {
			// get and forward c2
			val8 = MIDI_Read1( fp );
			printf( "%02x ", val8 );
		    }

		    // get and forward counter
			
		    byteCount = MIDI_Read1( fp );
		    printf( "(c=%02x) ", byteCount );
		    printf( "|" );

		    // for each data byte
		    while( byteCount > 0 ) {
			// get and forward a byte of data
			val8 = MIDI_Read1( fp );
			printf( "%c", val8 );
			byteCount--;
		    }
		    printf( "|" );
		    break;

		case( 0xC0 ):
		case( 0xD0 ):
		    // Voice-2 Commands (cmd, val1 )

		    // forward command

		    // get and forward byte
		    val8 = MIDI_Read1( fp ); printf( "%02x ", val8);
		    break;

		case( 0x80 ):
		case( 0x90 ):
		case( 0xA0 ):
		case( 0xB0 ):
		case( 0xE0 ):
		    // Voice-3, Mode-3 Commands (cmd, val1, val2)

		    // forward command

		    // get and forward byte
		    val8 = MIDI_Read1( fp ); printf( "%02x ", val8);
		    // get and forward byte
		    val8 = MIDI_Read1( fp ); printf( "%02x ", val8);
		    break;

		default:
		    // invalid?
		    break;
		}

	    }
	}
	   
    } while( 1 );
    
    fclose( fp );
    return 0;
}


/* --- Command line interface --------------------------------- */ 

void usage( char * av0 )
{
    printf( "%s [MIDI SMF]\n", av0 );
}

int main( int argc, char ** argv )
{
    char * filename = NULL;

    /* make sure we have just one option */
    if( argc < 1 ) {
	usage( argv[0] );
	return 0;
    }

    /* and attempt to parse it in... */
    filename = argv[1];

    printf( "Working with file %s\n", filename );
    //process( filename );
    test( filename );
}
