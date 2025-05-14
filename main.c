#include <reg52.h>
#include "music_player.h"
#include "music_data.h"
#include "keypad.h"

// --- LCD related includes and defines would go here ---
// #include "lcd1602.h" // Example: sbit LCD_RS = P2^0; ...

// --- Global variables for application state ---
unsigned char G_selected_song_index = 0; // Start with the first song (index 0)

// --- Keypad debouncing variables ---
static unsigned char s_key_current_reading = KEY_NONE;
static unsigned char s_key_debounced_state = KEY_NONE;
static unsigned char s_key_last_debounced_state = KEY_NONE;
static unsigned char s_key_debounce_count = 0;
#define KEY_DEBOUNCE_CHECKS 3 // Number of consecutive identical readings for a stable state

// Simple delay for main loop if needed for debouncing timing
void delay_ms_main(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 120; j++); // Adjust for crystal frequency (e.g., for ~1ms at 11.0592MHz)
    }
}

//-----------------------------------------------------------------------------
// Timer1 ISR for 1ms Tick (to call Music_Player_Tick)
//-----------------------------------------------------------------------------
#define TIMER1_RELOAD_VAL_1MS 0xFC66 // For 11.0592MHz, for 1ms interrupt

void Timer1_Init_1ms(void) {
    TMOD &= 0x0F; // Clear Timer1 bits
    TMOD |= 0x10; // Set Timer1 to Mode 1 (16-bit timer)
    TH1 = TIMER1_RELOAD_VAL_1MS / 256; // Load high byte for 1ms
    TL1 = TIMER1_RELOAD_VAL_1MS % 256; // Load low byte for 1ms
    TF1 = 0; // Clear Timer1 overflow flag
    ET1 = 1; // Enable Timer1 interrupt
    TR1 = 1; // Start Timer1
}

void Timer1_ISR(void) interrupt 3 { // Timer1 Interrupt vector (usually 3 for 8051)
    TH1 = TIMER1_RELOAD_VAL_1MS / 256; // Reload Timer1 for next 1ms interrupt
    TL1 = TIMER1_RELOAD_VAL_1MS % 256;
    Music_Player_Tick(); // Drive the music player rhythm
}

//-----------------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------------
void main() {
    unsigned char processed_key_press = KEY_NONE; // To store the key event
    PlayerState current_music_state; // To store current music player state

    // --- System Initializations ---
    // LCD_Init(); // Initialize your LCD if you have one
    Keypad_Init();      // Initialize keypad I/O
    Music_Init(500);    // Initialize music player (tempo 300 is an example)
    Timer1_Init_1ms();  // Initialize Timer1 for music ticks

    EA = 1; // Enable Global Interrupts *AFTER* all other initializations

    // --- Initial Display & Autoplay ---
    // LCD_ShowString(0, 0, "Music Box Autoplay"); // Example LCD message
    // LCD_ShowString(1, 0, "Playing: ");
    // LCD_ShowString(1, 9, Music_GetSongName(G_selected_song_index)); // Display initial song name

    Music_PlaySong(G_selected_song_index); // Autoplay the first song on power-on

    s_key_last_debounced_state = Keypad_GetImmediateKeyState(); // Initialize last keypad state for edge detection

    while (1) {
        // --- Keypad Debouncing and Edge Detection ---
        s_key_current_reading = Keypad_GetImmediateKeyState();

        if (s_key_current_reading == s_key_debounced_state) {
            s_key_debounce_count = 0; // Reset counter if reading is stable
        } else {
            s_key_debounce_count++;
            if (s_key_debounce_count >= KEY_DEBOUNCE_CHECKS) {
                s_key_debounce_count = 0;
                s_key_debounced_state = s_key_current_reading;

                if (s_key_debounced_state != KEY_NONE && s_key_last_debounced_state == KEY_NONE) {
                    processed_key_press = s_key_debounced_state;
                }
                s_key_last_debounced_state = s_key_debounced_state;
            }
        }

        // --- Process Key Press Event ---
        if (processed_key_press != KEY_NONE) {
            current_music_state = Music_GetState(); // Get current player state before processing key

            switch (processed_key_press) {
                case KEY_S13: // Play/Pause (mapped to S13)
                    if (current_music_state == PLAYER_PLAYING) {
                        Music_Pause();
                        // if (LCD_IsInitialized()) { LCD_ShowString(0, 0, "Paused!         "); }
                    } else if (current_music_state == PLAYER_PAUSED) {
                        Music_Resume(); // <<<--- CORRECTED: Resume from paused state
                        // if (LCD_IsInitialized()) { LCD_ShowString(0, 0, "Resuming...     "); }
                    } else { // PLAYER_STOPPED or other initial states
                        Music_PlaySong(G_selected_song_index);
                        // if (LCD_IsInitialized()) {
                        //     LCD_ShowString(0, 0, "Playing:        ");
                        //     LCD_ShowString(1, 9, Music_GetSongName(G_selected_song_index));
                        // }
                    }
                    break;

                case KEY_S14: // Next Song (mapped to S14)
                    G_selected_song_index++;
                    if (G_selected_song_index >= Music_GetTotalSongs()) {
                        G_selected_song_index = 0;
                    }
                    Music_PlaySong(G_selected_song_index);
                    // if (LCD_IsInitialized()) {
                    //     LCD_ShowString(0, 0, "Next Song:      ");
                    //     LCD_ShowString(1, 9, Music_GetSongName(G_selected_song_index));
                    // }
                    break;

                case KEY_S15: // Previous Song (mapped to S15)
                    if (G_selected_song_index == 0) {
                        G_selected_song_index = Music_GetTotalSongs() - 1;
                    } else {
                        G_selected_song_index--;
                    }
                    Music_PlaySong(G_selected_song_index);
                    // if (LCD_IsInitialized()) {
                    //     LCD_ShowString(0, 0, "Prev Song:      ");
                    //     LCD_ShowString(1, 9, Music_GetSongName(G_selected_song_index));
                    // }
                    break;

                case KEY_S16: // Stop (mapped to S16)
                    Music_Stop();
                    // if (LCD_IsInitialized()) { LCD_ShowString(0,0,"Stopped.        "); }
                    break;

                default:
                    break;
            }
            processed_key_press = KEY_NONE; // Clear the event after processing
        }
        delay_ms_main(10);
    }
}
