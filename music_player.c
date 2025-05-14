#include "music_player.h"
#include "music_data.h" // For FreqTable, Songs, Song_Names, NUM_SONGS

//-----------------------------------------------------------------------------
// Module-Private Variables
//-----------------------------------------------------------------------------
static PlayerState G_player_state;
// G_current_song_data_ptr points to data in CODE memory. The pointer itself resides in RAM.
static const unsigned char code * G_current_song_data_ptr;
static unsigned int G_current_song_note_index;
static unsigned int G_current_tempo;
static unsigned char G_current_freq_select;
static unsigned int G_note_duration_tick_counter;
static unsigned int G_time_since_last_note_ms;
static unsigned char G_freq_table_offset = 0;


//-----------------------------------------------------------------------------
// Private Helper Functions
//-----------------------------------------------------------------------------
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

static void timer0_init_for_music(void) {
    TMOD &= 0xF0;
    TMOD |= 0x01;
    TL0 = 0x18;
    TH0 = 0xFC;
    TF0 = 0;
    TR0 = 0;
    ET0 = 1;
    PT0 = 0;
}

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
    BUZZER_PIN = 0;
}

void Music_PlaySong(unsigned char song_index) {
    if (song_index >= NUM_SONGS) {
        return;
    }
    Music_Stop();
    G_current_song_data_ptr = Songs[song_index];
    G_current_song_note_index = 0;
    G_player_state = PLAYER_PLAYING;
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
}

void Music_Pause(void) {
    if (G_player_state == PLAYER_PLAYING) {
        TR0 = 0;
        G_player_state = PLAYER_PAUSED;
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

// Definition of Music_GetSongName.
// Its declaration in music_player.h must match this signature.
const char code * Music_GetSongName(unsigned char song_index) {
    if (song_index < NUM_SONGS) {
        return Song_Names[song_index];
    }
    return 0; // NULL pointer
}

void Music_Player_Tick(void) {
    if (G_player_state == PLAYER_PLAYING) {
        G_time_since_last_note_ms++;
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
void Music_Timer0_ISR(void) interrupt 1 using 1 { // 'using 1' selects register bank 1
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
