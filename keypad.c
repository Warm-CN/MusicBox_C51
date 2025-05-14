#include "keypad.h"

// Helper delay function for debouncing
// static void delay_ms_keypad(unsigned int ms) {
//     unsigned int i, j;
//     for (i = 0; i < ms; i++) {
//         for (j = 0; j < 120; j++); // Adjust based on crystal frequency, approximately 1ms
//     }
// }
// This delay is not used by Keypad_GetImmediateKeyState, it was from a previous blocking version.
// Debouncing is now handled in main.c using delay_ms_main.

void Keypad_Init(void) {
    // For 8051, to use P1 pins as inputs, their corresponding latch bits should be '1'.
    // This enables internal pull-ups (if the specific 8051 derivative has them and they are enabled by default)
    // and configures them as high-impedance inputs.

    // Set column pins (P1.0 - P1.3) as inputs by writing '1' to their latches.
    // These correspond to KEYPAD_COL3, KEYPAD_COL2, KEYPAD_COL1, KEYPAD_COL0 respectively.
    KEYPAD_COL0 = 1; // P1.3
    KEYPAD_COL1 = 1; // P1.2
    KEYPAD_COL2 = 1; // P1.1
    KEYPAD_COL3 = 1; // P1.0

    // Set row pins (P1.4 - P1.7) initially high (not driving any row low).
    // They will be driven low one by one during scanning.
    // These correspond to KEYPAD_ROW0, KEYPAD_ROW1, KEYPAD_ROW2, KEYPAD_ROW3.
    KEYPAD_ROW0 = 1; // P1.4
    KEYPAD_ROW1 = 1; // P1.5
    KEYPAD_ROW2 = 1; // P1.6
    KEYPAD_ROW3 = 1; // P1.7

    // Alternatively, if all pins P1.0-P1.7 are used only by the keypad as defined:
    // P1 = 0xFF; // Sets all P1 pins to '1', configuring them as inputs with pull-ups.
    // Then, row pins are driven low as needed during scan.
}

unsigned char Keypad_GetImmediateKeyState(void) {
    unsigned char key_code = KEY_NONE;

    // Scan Row 0 (S1-S4), connected to KEYPAD_ROW0 (P1.4)
    KEYPAD_ROW0 = 0; KEYPAD_ROW1 = 1; KEYPAD_ROW2 = 1; KEYPAD_ROW3 = 1;
    if (KEYPAD_COL0 == 0) { key_code = KEY_S1;  }       // P1.3
    else if (KEYPAD_COL1 == 0) { key_code = KEY_S2;  }  // P1.2
    else if (KEYPAD_COL2 == 0) { key_code = KEY_S3;  }  // P1.1
    else if (KEYPAD_COL3 == 0) { key_code = KEY_S4;  }  // P1.0
    KEYPAD_ROW0 = 1; // Deactivate row 0
    if (key_code != KEY_NONE) return key_code;

    // Scan Row 1 (S5-S8), connected to KEYPAD_ROW1 (P1.5)
    KEYPAD_ROW0 = 1; KEYPAD_ROW1 = 0; KEYPAD_ROW2 = 1; KEYPAD_ROW3 = 1;
    if (KEYPAD_COL0 == 0) { key_code = KEY_S5;  }
    else if (KEYPAD_COL1 == 0) { key_code = KEY_S6;  }
    else if (KEYPAD_COL2 == 0) { key_code = KEY_S7;  }
    else if (KEYPAD_COL3 == 0) { key_code = KEY_S8;  }
    KEYPAD_ROW1 = 1; // Deactivate row 1
    if (key_code != KEY_NONE) return key_code;

    // Scan Row 2 (S9-S12), connected to KEYPAD_ROW2 (P1.6)
    KEYPAD_ROW0 = 1; KEYPAD_ROW1 = 1; KEYPAD_ROW2 = 0; KEYPAD_ROW3 = 1;
    if (KEYPAD_COL0 == 0) { key_code = KEY_S9;  }
    else if (KEYPAD_COL1 == 0) { key_code = KEY_S10; }
    else if (KEYPAD_COL2 == 0) { key_code = KEY_S11; }
    else if (KEYPAD_COL3 == 0) { key_code = KEY_S12; }
    KEYPAD_ROW2 = 1; // Deactivate row 2
    if (key_code != KEY_NONE) return key_code;

    // Scan Row 3 (S13-S16), connected to KEYPAD_ROW3 (P1.7) - Your target keys
    KEYPAD_ROW0 = 1; KEYPAD_ROW1 = 1; KEYPAD_ROW2 = 1; KEYPAD_ROW3 = 0;
    if (KEYPAD_COL0 == 0) { key_code = KEY_S13; }
    else if (KEYPAD_COL1 == 0) { key_code = KEY_S14; }
    else if (KEYPAD_COL2 == 0) { key_code = KEY_S15; }
    else if (KEYPAD_COL3 == 0) { key_code = KEY_S16; }
    KEYPAD_ROW3 = 1; // Deactivate row 3
    // No need to check key_code here again, just return it or KEY_NONE from previous checks

    return key_code; // Will be KEY_NONE if no key was found, or the code of the first detected key
}
