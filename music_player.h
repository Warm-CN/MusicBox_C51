#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include <reg52.h>
#include <intrins.h> // For _nop_()

//-----------------------------------------------------------------------------
// Hardware Configuration
//-----------------------------------------------------------------------------
sbit BUZZER_PIN = P2^4; 

//-----------------------------------------------------------------------------
// Note Definitions (Indices for FreqTable)
//-----------------------------------------------------------------------------
#define NOTE_P  0  // Pause

#define NOTE_L1 1  // Low Do
#define NOTE_L1_ 2 // Low Do#
#define NOTE_L2 3  // Low Re
#define NOTE_L2_ 4 // Low Re#
#define NOTE_L3 5  // Low Mi
#define NOTE_L4 6  // Low Fa
#define NOTE_L4_ 7 // Low Fa#
#define NOTE_L5 8  // Low Sol
#define NOTE_L5_ 9 // Low Sol#
#define NOTE_L6 10 // Low La
#define NOTE_L6_ 11// Low La#
#define NOTE_L7 12 // Low Ti

#define NOTE_M1 13 // Middle Do
#define NOTE_M1_ 14// Middle Do#
#define NOTE_M2 15 // Middle Re
#define NOTE_M2_ 16// Middle Re#
#define NOTE_M3 17 // Middle Mi
#define NOTE_M4 18 // Middle Fa
#define NOTE_M4_ 19// Middle Fa#
#define NOTE_M5 20 // Middle Sol
#define NOTE_M5_ 21// Middle Sol#
#define NOTE_M6 22 // Middle La
#define NOTE_M6_ 23// Middle La#
#define NOTE_M7 24 // Middle Ti

#define NOTE_H1 25 // High Do
#define NOTE_H1_ 26// High Do#
#define NOTE_H2 27 // High Re
#define NOTE_H2_ 28// High Re#
#define NOTE_H3 29 // High Mi
#define NOTE_H4 30 // High Fa
#define NOTE_H4_ 31// High Fa#
#define NOTE_H5 32 // High Sol
#define NOTE_H5_ 33// High Sol#
#define NOTE_H6 34 // High La
#define NOTE_H6_ 35// High La#
#define NOTE_H7 36 // High Ti

#define SONG_END_MARKER 0xFF // Marker for the end of a song array

//-----------------------------------------------------------------------------
// Public Data Types
//-----------------------------------------------------------------------------
typedef enum {
    PLAYER_STOPPED,
    PLAYER_PLAYING,
    PLAYER_PAUSED
} PlayerState;

//-----------------------------------------------------------------------------
// Public Function Prototypes
//-----------------------------------------------------------------------------

void Music_Init(unsigned int initial_tempo);
void Music_PlaySong(unsigned char song_index);
void Music_Stop(void);
void Music_Pause(void);
void Music_Resume(void);
void Music_SetTempo(unsigned int new_tempo);
// Add to public function prototypes
unsigned long Music_GetCurrentSongElapsedTimeMs(void);
PlayerState Music_GetState(void);
unsigned char Music_GetTotalSongs(void);

/**
 * @brief Gets the name of a song by its index.
 * @param song_index Index of the song.
 * @return const char code* Pointer to the song name string in CODE memory,
 * or NULL (0) if index is invalid.
 */
const char code * Music_GetSongName(unsigned char song_index);

void Music_Player_Tick(void);

// Timer0 ISR is usually defined in music_player.c and not prototyped here
// unless it needs to be called from elsewhere (which is rare for an ISR).
// void Music_Timer0_ISR(void) interrupt 1; // Example if needed, but typically not

#endif // MUSIC_PLAYER_H
