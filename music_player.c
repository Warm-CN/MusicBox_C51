#include "music_player.h"
#include "music_data.h"

//-----------------------------------------------------------------------------
// Module-Private Variables
//-----------------------------------------------------------------------------
static PlayerState G_player_state;
static const unsigned char code * G_current_song_data_ptr;
static unsigned int G_current_song_note_index;
static unsigned int G_current_tempo;
static unsigned char G_current_freq_select;
static unsigned int G_note_duration_tick_counter;
static unsigned int G_time_since_last_note_ms; // Incremented by Music_Player_Tick
static unsigned char G_freq_table_offset = 0;

// Variable to store total elapsed milliseconds for the current song
static unsigned long G_song_total_elapsed_ms = 0;


//-----------------------------------------------------------------------------
// Private Helper Functions
//-----------------------------------------------------------------------------

// This delay function is currently uncalled as per linker warnings.
// It can be removed if not needed for other purposes.
/*
static void delay_ms_software(unsigned int ms) {
    unsigned char i, j;
    while (ms--) {
        _nop_();
        i = 2;
        j = 199;
        do {
            while (--j);
        } while (--i);
    }
}
*/

/**
 * @brief Initializes Timer0 for sound generation.
 * Configures Timer0 in Mode 1 (16-bit timer).
 * Interrupt is enabled, but the timer is initially stopped.
 * Timer0 priority is set to HIGH.
 */
static void timer0_init_for_music(void) {
    TMOD &= 0xF0; // Clear Timer0 bits (T0M1=0, T0M0=0, T0_GATE=0, T0_CT=0)
    TMOD |= 0x01; // Set Timer0 to Mode 1 (16-bit timer: T0M1=0, T0M0=1)

    TL0 = 0x18; // Default initial load, ISR will overwrite
    TH0 = 0xFC;

    TF0 = 0; // Clear Timer0 overflow flag
    TR0 = 0; // Stop Timer0 initially
    ET0 = 1; // Enable Timer0 interrupt
    PT0 = 1; // <<<--- SET TIMER 0 INTERRUPT PRIORITY TO HIGH
}

/**
 * @brief Loads and starts playing the next note in the current song.
 */
static void play_next_note(void) {
    unsigned char duration_multiplier;
    unsigned int timer_reload_val;

    if (G_player_state != PLAYER_PLAYING || G_current_song_data_ptr == 0) {
        Music_Stop();
        return;
    }

    G_current_freq_select = G_current_song_data_ptr[G_current_song_note_index];
    
    if (G_current_freq_select == SONG_END_MARKER) {
        Music_Stop();
        return;
    }

    G_current_song_note_index++;
    duration_multiplier = G_current_song_data_ptr[G_current_song_note_index];
    G_current_song_note_index++;

    G_note_duration_tick_counter = (G_current_tempo / 4) * duration_multiplier;
    G_time_since_last_note_ms = 0;

    if (G_current_freq_select == NOTE_P) {
        TR0 = 0;
        BUZZER_PIN = 0;
    } else {
        if (!TR0) {
            timer_reload_val = FreqTable[G_current_freq_select + G_freq_table_offset];
            TL0 = timer_reload_val % 256;
            TH0 = timer_reload_val / 256;
            TR0 = 1;
        }
    }
}


//-----------------------------------------------------------------------------
// Public Function Implementations
//-----------------------------------------------------------------------------

void Music_Init(unsigned int initial_tempo) {
    timer0_init_for_music();
    G_current_tempo = initial_tempo > 0 ? initial_tempo : 500;
    G_player_state = PLAYER_STOPPED;
    G_current_song_data_ptr = 0;
    G_current_song_note_index = 0;
    G_current_freq_select = NOTE_P;
    G_note_duration_tick_counter = 0;
    G_time_since_last_note_ms = 0;
    G_song_total_elapsed_ms = 0; // Initialize elapsed time
    BUZZER_PIN = 0;
}

void Music_PlaySong(unsigned char song_index) {
    if (song_index >= NUM_SONGS) {
        return;
    }

    Music_Stop(); // This will also reset G_song_total_elapsed_ms

    G_current_song_data_ptr = Songs[song_index];
    G_current_song_note_index = 0;
    G_player_state = PLAYER_PLAYING;
    // G_song_total_elapsed_ms = 0; // Redundant, Music_Stop() already does this.
    
    play_next_note();
}

void Music_Stop(void) {
    TR0 = 0;
    BUZZER_PIN = 0;
    G_player_state = PLAYER_STOPPED;
    G_current_song_data_ptr = 0;
    G_current_song_note_index = 0;
    G_current_freq_select = NOTE_P;
    G_note_duration_tick_counter = 0;
    G_time_since_last_note_ms = 0;
    G_song_total_elapsed_ms = 0; // Reset elapsed time when stopped
}

void Music_Pause(void) {
    if (G_player_state == PLAYER_PLAYING) {
        TR0 = 0;
        G_player_state = PLAYER_PAUSED;
        // Elapsed time G_song_total_elapsed_ms is NOT reset on pause
    }
}

void Music_Resume(void) {
    unsigned int timer_reload_val;
    if (G_player_state == PLAYER_PAUSED) {
        G_player_state = PLAYER_PLAYING;
        if (G_current_freq_select != NOTE_P) {
            timer_reload_val = FreqTable[G_current_freq_select + G_freq_table_offset];
            TL0 = timer_reload_val % 256;
            TH0 = timer_reload_val / 256;
            TR0 = 1;
        }
    }
}

void Music_SetTempo(unsigned int new_tempo) {
    if (new_tempo > 0) {
        G_current_tempo = new_tempo;
    }
}

PlayerState Music_GetState(void) {
    return G_player_state;
}

unsigned char Music_GetTotalSongs(void) {
    return NUM_SONGS;
}

const char code * Music_GetSongName(unsigned char song_index) {
    if (song_index < NUM_SONGS) {
        return Song_Names[song_index];
    }
    return 0;
}

unsigned long Music_GetCurrentSongElapsedTimeMs(void) {
    return G_song_total_elapsed_ms;
}

void Music_Player_Tick(void) {
    if (G_player_state == PLAYER_PLAYING) {
        G_song_total_elapsed_ms++; // Increment total elapsed milliseconds for the song
        G_time_since_last_note_ms++; // Increment time for the current note/pause

        if (G_time_since_last_note_ms >= G_note_duration_tick_counter) {
            if (G_current_song_data_ptr != 0 && G_current_song_data_ptr[G_current_song_note_index] != SONG_END_MARKER) {
                 TR0 = 0; 
                 BUZZER_PIN = 0;
            }
            play_next_note();
        }
    }
}

//-----------------------------------------------------------------------------
// Timer0 Interrupt Service Routine
//-----------------------------------------------------------------------------
void Music_Timer0_ISR(void) interrupt 1 using 1 { // Timer0 Interrupt vector, using register bank 1
    unsigned int reload_value;
    if (G_player_state == PLAYER_PLAYING && G_current_freq_select != NOTE_P) {
        reload_value = FreqTable[G_current_freq_select + G_freq_table_offset];
        TL0 = reload_value % 256;
        TH0 = reload_value / 256;
        BUZZER_PIN = !BUZZER_PIN;
    } else {
        BUZZER_PIN = 0;
    }
}
