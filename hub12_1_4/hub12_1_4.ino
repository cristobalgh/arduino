// --- HUB12 Test 2: 1/8 Scan (8 Address Rows) ---

#define OE_PIN      13
#define A_PIN       12
#define B_PIN       14
#define C_PIN       27
#define CLK_PIN     26
#define SCLK_PIN    25
#define DAT_PIN_R1  33 // Red Data (R)
#define DAT_PIN_R2  32 // Green Data (G - maybe R2)

const int PANEL_WIDTH = 32;
const int PANEL_HEIGHT = 16;
const int NUM_ROW_ADDRESSES = 8; // Addresses 0 through 7

uint8_t frame_buffer[PANEL_HEIGHT][PANEL_WIDTH];

void setRowAddress(int row_addr) {
    digitalWrite(A_PIN, (row_addr & 0b001) ? HIGH : LOW);
    digitalWrite(B_PIN, (row_addr & 0b010) ? HIGH : LOW);
    digitalWrite(C_PIN, (row_addr & 0b100) ? HIGH : LOW); // C pin is now used
}

void shiftDataBit(uint8_t data_r1, uint8_t data_r2) {
    digitalWrite(DAT_PIN_R1, data_r1); 
    digitalWrite(DAT_PIN_R2, data_r2); 

    digitalWrite(CLK_PIN, HIGH);
    digitalWrite(CLK_PIN, LOW);
}

void drivePanel() {
    digitalWrite(OE_PIN, HIGH);
    digitalWrite(SCLK_PIN, LOW);

    for (int addr = 0; addr < NUM_ROW_ADDRESSES; addr++) { // Loop 0 to 7
        setRowAddress(addr);

        for (int col = 0; col < PANEL_WIDTH; col++) {
            // R1 Data for the Top Half: row 'addr'
            uint8_t data_r1 = frame_buffer[addr][col];
            // R2 Data for the Bottom Half: row 'addr + 8' (This is now redundant since addr goes up to 7)
            // For a 1/8 scan, a single address 'addr' corresponds to TWO active rows, 
            // one for the R1 line and one for the R2 line.
            // Often, it's Row 'addr' and Row 'addr + 8'.
            // Let's assume the R1 line drives Row 'addr' and R2 drives Row 'addr + 8'.
            
            // This is the common pattern for 1/8 scan on a 16-row panel (sometimes called 1/4 dual-line).
            // R1 feeds Row 'addr', R2 feeds Row 'addr+8'.
            uint8_t data_r2 = frame_buffer[addr + 8][col]; 

            shiftDataBit(data_r1, data_r2);
        }

        digitalWrite(SCLK_PIN, HIGH);
        digitalWrite(SCLK_PIN, LOW);

        digitalWrite(OE_PIN, LOW);
        delayMicroseconds(200); 
        digitalWrite(OE_PIN, HIGH);
    }
}

void setup() {
    // Initialize pins
    pinMode(OE_PIN, OUTPUT); pinMode(A_PIN, OUTPUT); pinMode(B_PIN, OUTPUT); 
    pinMode(C_PIN, OUTPUT); pinMode(CLK_PIN, OUTPUT); pinMode(SCLK_PIN, OUTPUT); 
    pinMode(DAT_PIN_R1, OUTPUT); pinMode(DAT_PIN_R2, OUTPUT);

    // Initial state
    digitalWrite(OE_PIN, HIGH); 
    digitalWrite(SCLK_PIN, LOW); 
    digitalWrite(CLK_PIN, LOW); 

    // Set a test pattern: A solid horizontal bar across the middle of the display.
    // Rows 7 and 8 (the border between the two halves)
    for (int c = 0; c < PANEL_WIDTH; c++) {
        frame_buffer[7][c] = 1;
        frame_buffer[8][c] = 1;
    }
}

void loop() {
    drivePanel();
}
