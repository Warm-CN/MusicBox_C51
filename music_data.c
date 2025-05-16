#include "music_player.h" // For NOTE_* defines and SONG_END_MARKER
#include "music_data.h"   // For consistency

//-----------------------------------------------------------------------------
// Frequency Table (Timer0 reload values for 16-bit mode)
//-----------------------------------------------------------------------------
// Calculated for F_OSC = 11.0592 MHz
// Machine Cycle Freq = 11.0592 MHz / 12 = 921600 Hz
// Timer Reload Value = 65536 - (MachineCycleFreq / (2 * NoteFreq))
// Note Frequencies (Approximate):
// C4 (M1): 261.63 Hz | D4 (M2): 293.66 Hz | E4 (M3): 329.63 Hz | F4 (M4): 349.23 Hz
// G4 (M5): 392.00 Hz | A4 (M6): 440.00 Hz | B4 (M7): 493.88 Hz
// L_ = Octave Lower, H_ = Octave Higher. Each step is F_new = F_old * 2^(1/12) for sharps.
// For simplicity, this table provides basic natural notes. Sharps/flats would need more entries.

const unsigned int code FreqTable[] = {
    // Pause
    0, // NOTE_P (Timer will not be reloaded by ISR if FreqSelect is this)

    // Low Octave (Approx C3-B3) - Values are 65536 - (921600 / (2 * Freq))
    65536 - (921600 / (2 * 130.81)), // L1 (C3) ~130.81 Hz -> 65536 - 3522 = 62014
    0,                               // L1_ (Placeholder for C#3)
    65536 - (921600 / (2 * 146.83)), // L2 (D3) ~146.83 Hz -> 65536 - 3138 = 62398
    0,                               // L2_ (Placeholder for D#3)
    65536 - (921600 / (2 * 164.81)), // L3 (E3) ~164.81 Hz -> 65536 - 2796 = 62740
    65536 - (921600 / (2 * 174.61)), // L4 (F3) ~174.61 Hz -> 65536 - 2639 = 62897
    0,                               // L4_ (Placeholder for F#3)
    65536 - (921600 / (2 * 196.00)), // L5 (G3) ~196.00 Hz -> 65536 - 2351 = 63185
    0,                               // L5_ (Placeholder for G#3)
    65536 - (921600 / (2 * 220.00)), // L6 (A3) ~220.00 Hz -> 65536 - 2095 = 63441
    0,                               // L6_ (Placeholder for A#3)
    65536 - (921600 / (2 * 246.94)), // L7 (B3) ~246.94 Hz -> 65536 - 1866 = 63670

    // Middle Octave (Approx C4-B4)
    65536 - (921600 / (2 * 261.63)), // M1 (C4) ~261.63 Hz -> 65536 - 1761 = 63775
    0,                               // M1_ (Placeholder for C#4)
    65536 - (921600 / (2 * 293.66)), // M2 (D4) ~293.66 Hz -> 65536 - 1569 = 63967
    0,                               // M2_ (Placeholder for D#4)
    65536 - (921600 / (2 * 329.63)), // M3 (E4) ~329.63 Hz -> 65536 - 1398 = 64138
    65536 - (921600 / (2 * 349.23)), // M4 (F4) ~349.23 Hz -> 65536 - 1319 = 64217
    0,                               // M4_ (Placeholder for F#4)
    65536 - (921600 / (2 * 392.00)), // M5 (G4) ~392.00 Hz -> 65536 - 1176 = 64360
    0,                               // M5_ (Placeholder for G#4)
    65536 - (921600 / (2 * 440.00)), // M6 (A4) ~440.00 Hz -> 65536 - 1047 = 64489
    0,                               // M6_ (Placeholder for A#4)
    65536 - (921600 / (2 * 493.88)), // M7 (B4) ~493.88 Hz -> 65536 - 933  = 64603

    // High Octave (Approx C5-B5)
    65536 - (921600 / (2 * 523.25)), // H1 (C5) ~523.25 Hz -> 65536 - 881  = 64655
    0,                               // H1_ (Placeholder for C#5)
    65536 - (921600 / (2 * 587.33)), // H2 (D5) ~587.33 Hz -> 65536 - 785  = 64751
    0,                               // H2_ (Placeholder for D#5)
    65536 - (921600 / (2 * 659.25)), // H3 (E5) ~659.25 Hz -> 65536 - 699  = 64837
    65536 - (921600 / (2 * 698.46)), // H4 (F5) ~698.46 Hz -> 65536 - 660  = 64876  -- Note: Freq for F5 is 698.46, (921600/(2*698.46)) = 659.8 ~ 660
    0,                               // H4_ (Placeholder for F#5)
    65536 - (921600 / (2 * 783.99)), // H5 (G5) ~783.99 Hz -> 65536 - 588  = 64948
    0,                               // H5_ (Placeholder for G#5)
    65536 - (921600 / (2 * 880.00)), // H6 (A5) ~880.00 Hz -> 65536 - 524  = 65012 // (921600/(2*880)) = 523.6 ~ 524
    0,                               // H6_ (Placeholder for A#5)
    65536 - (921600 / (2 * 987.77))  // H7 (B5) ~987.77 Hz -> 65536 - 466  = 65070
};

//-----------------------------------------------------------------------------
// Song Data: "Farewell"
//-----------------------------------------------------------------------------
const unsigned char code Song_Farewell[] = {
    // Prelude
    NOTE_P,12,   NOTE_M5,4,  NOTE_M3,2,  NOTE_M5,2,  NOTE_H1,6,  NOTE_M7,2,
    NOTE_M6,4,  NOTE_H1,4,  NOTE_M5,8,
    NOTE_M5,4,  NOTE_M2,2,  NOTE_M3,2,  NOTE_M4,6,  NOTE_L7,2, // NOTE_L7 might be too low with this table, adjust if needed
    NOTE_M1,16,

    // Verse 1
    NOTE_M5,4,  NOTE_M3,2,  NOTE_M5,2,  NOTE_H1,8,
    NOTE_M6,4,  NOTE_H1,4,  NOTE_M5,8,
    NOTE_M5,4,  NOTE_M1,2,  NOTE_M2,2,  NOTE_M3,4,  NOTE_M2,2,  NOTE_M1,2,
    NOTE_M2,8,  NOTE_P,8,
    NOTE_M5,4,  NOTE_M3,2,  NOTE_M5,2,  NOTE_H1,6,  NOTE_M7,2,
    NOTE_M6,4,  NOTE_H1,4,  NOTE_M5,8,
    NOTE_M5,4,  NOTE_M2,2,  NOTE_M3,2,  NOTE_M4,6,  NOTE_L7,2,
    NOTE_M1,8,  NOTE_P,8,
    NOTE_M6,4,  NOTE_H1,4,  NOTE_H1,8,
    NOTE_M7,4,  NOTE_M6,2,  NOTE_M7,2,  NOTE_H1,8,
    NOTE_M6,2,  NOTE_M7,2,  NOTE_H1,2,  NOTE_M6,2,  NOTE_M6,2,  NOTE_M5,2,  NOTE_M3,2,  NOTE_M1,2,
    NOTE_M2,8,  NOTE_P,8,
    NOTE_M5,4,  NOTE_M3,2,  NOTE_M5,2,  NOTE_H1,6,  NOTE_M7,2,
    NOTE_M6,4,  NOTE_H1,4,  NOTE_M5,8,
    NOTE_M5,4,  NOTE_M2,2,  NOTE_M3,2,  NOTE_M4,6,  NOTE_L7,2,
    NOTE_M1,8,  NOTE_P,8,
    SONG_END_MARKER
};

//-----------------------------------------------------------------------------
// Song Data: "Sky" (City of Sky)
//-----------------------------------------------------------------------------
const unsigned char code Song_Sky[] = {
    NOTE_P, 4,  NOTE_P, 4,  NOTE_P, 4,  NOTE_M6, 2,  NOTE_M7, 2,
    NOTE_H1, 4+2, NOTE_M7, 2, NOTE_H1, 4,  NOTE_H3, 4,
    NOTE_M7, 4+4+4, NOTE_M3, 2,  NOTE_M3, 2,
    NOTE_M6, 4+2, NOTE_M5, 2, NOTE_M6, 4,  NOTE_H1, 4,
    NOTE_M5, 4+4+4, NOTE_M3, 4,
    NOTE_M4, 4+2, NOTE_M3, 2, NOTE_M4, 4,  NOTE_H1, 4, // NOTE_M4_ used in original, replace with NOTE_M4 or define sharp
    NOTE_M3, 4+4, NOTE_P, 2,  NOTE_H1, 2,  NOTE_H1, 2,  NOTE_H1, 2,
    NOTE_M7, 4+2, NOTE_M4, 2, NOTE_M4, 4, NOTE_M7, 4, // Replaced NOTE_M4_
    NOTE_M7, 8,   NOTE_P, 4,  NOTE_M6, 2,  NOTE_M7, 2,
    NOTE_H1, 4+2, NOTE_M7, 2, NOTE_H1, 4,  NOTE_H3, 4,
    NOTE_M7, 4+4+4, NOTE_M3, 2,  NOTE_M3, 2,
    NOTE_M6, 4+2, NOTE_M5, 2, NOTE_M6, 4,  NOTE_H1, 4,
    NOTE_M5, 4+4+4, NOTE_M2, 2,  NOTE_M3, 2,
    NOTE_M4, 4,   NOTE_H1, 2,  NOTE_M7, 2+2, NOTE_H1, 2+4,
    NOTE_H2, 2,   NOTE_H2, 2,  NOTE_H3, 2,  NOTE_H1, 2+4+4,
    NOTE_H1, 2,   NOTE_M7, 2,  NOTE_M6, 2,  NOTE_M6, 2,  NOTE_M7, 4,  NOTE_M5, 4, // Replaced NOTE_M5_
    NOTE_M6, 4+4+4, NOTE_H1, 2,  NOTE_H2, 2,
    NOTE_H3, 4+2, NOTE_H2, 2, NOTE_H3, 4,  NOTE_H5, 4,
    NOTE_H2, 4+4+4, NOTE_M5, 2,  NOTE_M5, 2,
    NOTE_H1, 4+2, NOTE_M7, 2, NOTE_H1, 4,  NOTE_H3, 4,
    NOTE_H3, 4+4+4+4,
    NOTE_M6, 2,   NOTE_M7, 2,  NOTE_H1, 4,  NOTE_M7, 4,  NOTE_H2, 2,  NOTE_H2, 2,
    NOTE_H1, 4+2, NOTE_M5, 2+4+4,
    NOTE_H4, 4,   NOTE_H3, 4,  NOTE_H3, 4,  NOTE_H1, 4,
    NOTE_H3, 4+4+4, NOTE_H3, 4,
    NOTE_H6, 4+4, NOTE_H5, 4,  NOTE_H5, 4,
    NOTE_H3, 2,   NOTE_H2, 2,  NOTE_H1, 4+4, NOTE_P, 2,  NOTE_H1, 2,
    NOTE_H2, 4,   NOTE_H1, 2,  NOTE_H2, 2,  NOTE_H2, 4,  NOTE_H5, 4,
    NOTE_H3, 4+4+4, NOTE_H3, 4,
    NOTE_H6, 4+4, NOTE_H5, 4+4,
    NOTE_H3, 2,   NOTE_H2, 2,  NOTE_H1, 4+4, NOTE_P, 2,  NOTE_H1, 2,
    NOTE_H2, 4,   NOTE_H1, 2,  NOTE_H2, 2+4, NOTE_M7, 4,
    NOTE_M6, 4+4+4, NOTE_P, 4,
    SONG_END_MARKER
};

//-----------------------------------------------------------------------------
// Song Data: "Ode to Joy"
//-----------------------------------------------------------------------------
const unsigned char code Song_OdeToJoy[] = {
	  NOTE_P, 12,
	  // Phrase 1 (E E F G | G F E D | C C D E | E. D D_half)
    NOTE_M3, 4, NOTE_M3, 4, NOTE_M4, 4, NOTE_M5, 4,  // Mi Mi Fa So
    NOTE_M5, 4, NOTE_M4, 4, NOTE_M3, 4, NOTE_M2, 4,  // So Fa Mi Re
    NOTE_M1, 4, NOTE_M1, 4, NOTE_M2, 4, NOTE_M3, 4,  // Do Do Re Mi
    NOTE_M3, 6, NOTE_M2, 2, NOTE_M2, 8,              // Mi. Re Re-

    // Phrase 2 (E E F G | G F E D | C C D E | D. C C_half)
    NOTE_M3, 4, NOTE_M3, 4, NOTE_M4, 4, NOTE_M5, 4,  // Mi Mi Fa So
    NOTE_M5, 4, NOTE_M4, 4, NOTE_M3, 4, NOTE_M2, 4,  // So Fa Mi Re
    NOTE_M1, 4, NOTE_M1, 4, NOTE_M2, 4, NOTE_M3, 4,  // Do Do Re Mi
    NOTE_M2, 6, NOTE_M1, 2, NOTE_M1, 8,              // Re. Do Do-

    // Phrase 3 (D D E C | D E F G E C | D E F G E D | C D G_low_half)
    NOTE_M2, 4, NOTE_M2, 4, NOTE_M3, 4, NOTE_M1, 4,  // Re Re Mi Do
    NOTE_M2, 4, NOTE_M3, 2, NOTE_M4, 2, NOTE_M3, 4,  NOTE_M1, 4, // Re Mi Fa Mi Do
    NOTE_M2, 4, NOTE_M3, 4, NOTE_M3, 4, NOTE_M2, 4, // Re Mi Mi Re 
    NOTE_M1, 4, NOTE_M2, 4, NOTE_L5, 8,              // Do Re Sol(low)-

    // Phrase 4 (Repeat of Phrase 2 logic)
    NOTE_M3, 4, NOTE_M3, 4, NOTE_M4, 4, NOTE_M5, 4,  // Mi Mi Fa So
    NOTE_M5, 4, NOTE_M4, 4, NOTE_M3, 4, NOTE_M2, 4,  // So Fa Mi Re
    NOTE_M1, 4, NOTE_M1, 4, NOTE_M2, 4, NOTE_M3, 4,  // Do Do Re Mi
    NOTE_M2, 6, NOTE_M1, 2, NOTE_M1, 8,              // Re. Do Do-
    
    SONG_END_MARKER
};
const unsigned char code * const code Songs[] = {
    Song_Farewell,
    Song_Sky,
		Song_OdeToJoy	  
};

const char code * const code Song_Names[] = {
    "Farewell",
    "Sky City",
	  "OdeToJoy"
};

const unsigned char NUM_SONGS = sizeof(Songs) / sizeof(Songs[0]);
