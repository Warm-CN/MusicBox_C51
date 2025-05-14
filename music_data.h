#ifndef MUSIC_DATA_H
#define MUSIC_DATA_H

// This header provides extern declarations for data defined in music_data.c
// All actual data arrays are stored in CODE memory.

//-----------------------------------------------------------------------------
// External Declarations for Music Data (defined in music_data.c)
//-----------------------------------------------------------------------------

// Frequency table for notes (Timer0 reload values for 16-bit mode)
// Stored in CODE memory.
extern const unsigned int code FreqTable[];

// Song data arrays
// Format: Note, Duration, Note, Duration, ..., SONG_END_MARKER
// Stored in CODE memory.
extern const unsigned char code Song_Farewell[];
extern const unsigned char code Song_Sky[];
// Add declarations for more songs here if you add them to music_data.c
// extern const unsigned char code Song_AnotherSong[];

// Array of pointers to all available songs.
// The array itself and the pointers it contains, and the data they point to,
// are all intended to be in CODE memory.
extern const unsigned char code * const code Songs[];


// Array of song names (pointers to strings in CODE memory).
// The array itself is also in CODE memory.
extern const char code * const code Song_Names[];

// Total number of songs available. This is a const variable defined in music_data.c
extern const unsigned char NUM_SONGS;

#endif // MUSIC_DATA_H
