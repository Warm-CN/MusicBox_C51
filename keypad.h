#ifndef KEYPAD_H
#define KEYPAD_H

#include <reg52.h>

//-----------------------------------------------------------------------------
// Keypad Pin Definitions (based on image_303460.png)
//-----------------------------------------------------------------------------
// Row lines - Output
sbit KEYPAD_ROW0 = P1^7; // Connected to S1-S4 row
sbit KEYPAD_ROW1 = P1^6; // Connected to S5-S8 row
sbit KEYPAD_ROW2 = P1^5; // Connected to S9-S12 row
sbit KEYPAD_ROW3 = P1^4; // Connected to S13-S16 row

// Column lines - Input
sbit KEYPAD_COL0 = P1^3; // Connected to S1,S5,S9,S13 column
sbit KEYPAD_COL1 = P1^2; // Connected to S2,S6,S10,S14 column
sbit KEYPAD_COL2 = P1^1; // Connected to S3,S7,S11,S15 column
sbit KEYPAD_COL3 = P1^0; // Connected to S4,S8,S12,S16 column

//-----------------------------------------------------------------------------
// Key Value Definitions
//-----------------------------------------------------------------------------
#define KEY_NONE 0xFF // No key currently pressed

// Define values for each key S1-S16 if needed, or use row/col mapping.
// For simplicity, we'll map them 0-15 or use specific defines.
#define KEY_S1   0x01
#define KEY_S2   0x02
#define KEY_S3   0x03
#define KEY_S4   0x04 // Or use column index 0 for S4 if mapping is 0-3 for cols

#define KEY_S5   0x05
#define KEY_S6   0x06
#define KEY_S7   0x07
#define KEY_S8   0x08

#define KEY_S9   0x09
#define KEY_S10  0x0A
#define KEY_S11  0x0B
#define KEY_S12  0x0C

#define KEY_S13  0x0D
#define KEY_S14  0x0E
#define KEY_S15  0x0F
#define KEY_S16  0x00 // Example: S16 as 0, or use 0x10

//-----------------------------------------------------------------------------
// Public Function Prototypes
//-----------------------------------------------------------------------------

/**
 * @brief Initializes the I/O ports required for the keypad.
 */
void Keypad_Init(void);

/**
 * @brief Scans the keypad and returns the currently pressed key's value.
 * This function is non-blocking and does not include debouncing or wait-for-release.
 * It reflects the instantaneous state of the keypad.
 * @return unsigned char Key value (e.g., KEY_S13), or KEY_NONE if no key is physically down.
 */
unsigned char Keypad_GetImmediateKeyState(void);

#endif // KEYPAD_H
