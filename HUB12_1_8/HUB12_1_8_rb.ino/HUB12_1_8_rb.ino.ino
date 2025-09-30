// P4.75 HUB12 1/8 scan 64x16 de un solo color, matrix control.

//ESP32 dev module
#define oe_pin     13 // OE en conector IDC HUB12
#define a_pin      12 // A en conector IDC HUB12
#define b_pin      14 // B en conector IDC HUB12
#define c_pin      33 // C en conector IDC HUB12
#define clk_pin    27 // S o CLK en conector IDC HUB12
#define sclk_pin   26 // L o SCLK o STB o LAT en conector IDC HUB12
#define dato_pinR  25 // R o Data en conector IDC HUB12
#define FILAS 16
#define COLS  64

int fila  = 0;
int aux   = 0;
uint8_t bits[FILAS][COLS];
uint8_t uno[8][16];
uint8_t dos[8][16];
uint8_t tres[8][16];
uint8_t cuatro[8][16];
uint8_t cinco[8][16];
uint8_t seis[8][16];
uint8_t siete[8][16];
uint8_t ocho[8][16];

// Fuente 8x8: cada byte representa una fila de la letra
// (1 = pixel encendido, 0 = apagado)
uint8_t font8x8[][8] = {
    // H
    {0b10000001,
     0b10000001,
     0b10000001,
     0b11111111,
     0b10000001,
     0b10000001,
     0b10000001,
     0b00000000},
    // O
    {0b01111110,
     0b10000001,
     0b10000011,
     0b10000101,
     0b10001001,
     0b10010001,
     0b01111110,
     0b00000000},
    // L
    {0b10000000,
     0b10000000,
     0b10000000,
     0b10000000,
     0b10000000,
     0b10000000,
     0b11111111,
     0b00000000},
    // A
    {0b01111110,
     0b10000001,
     0b10000001,
     0b10000001,
     0b11111111,
     0b10000001,
     0b10000001,
     0b00000000}
};

void subCopiar(){
  copySubMatrix(bits, uno, 0, 0); //copySubMatrix(grande, chica, filaInicial, colInicial)
  copySubMatrix(bits, dos, 8, 0);
  copySubMatrix(bits, tres, 0, 16);
  copySubMatrix(bits, cuatro, 8, 16);
  copySubMatrix(bits, cinco, 0, 32);
  copySubMatrix(bits, seis, 8, 32);
  copySubMatrix(bits, siete, 0, 48);
  copySubMatrix(bits, ocho, 8, 48);
}

// Escribir "HOLA" en la matriz bits[16][64]
void writeHOLA() {
    memset(bits, 0, sizeof(bits)); // limpiar matriz
    
    int letras = 4;
    int ancho = 8;
    int espacio = 2; // separación entre letras
    int startCol = 0;

    for (int k = 0; k < letras; k++) {
        for (int fila = 0; fila < 8; fila++) {
            for (int col = 0; col < 8; col++) {
                // Tomamos el bit directamente del byte
                if (font8x8[k][fila] & (1 << (7-col))) {
                    // fila + 4 → centrado vertical
                    bits[fila + 4][startCol + col] = 1;
                }
            }
        }
        startCol += ancho + espacio; // avanzar columna para la siguiente letra
    }
}

void fillDiagonalsPattern(int spacing) {
    memset(bits, 0, sizeof(bits)); // pone todo en 0
    for (int fila = 0; fila < FILAS; fila++) {
        for (int col = 0; col < COLS; col++) {
            if (((fila + col) % spacing) == 0) {
                bits[fila][col] = 1;
            }
        }
    }
}

void copySubMatrix(uint8_t src[16][64], uint8_t dest[8][16], int startFila, int startCol) {
    for (int fila = 0; fila < 8; fila++) {
        for (int col = 0; col < 16; col++) {
            dest[fila][col] = src[startFila + fila][startCol + col];
        }
    }
}

void rotateVertical(uint8_t bits[FILAS][COLS], int dir) {
    uint8_t temp[COLS];

    if (dir > 0) {
        // Rota hacia abajo
        memcpy(temp, bits[FILAS-1], sizeof(temp));  // guardar última fila
        for (int i = FILAS-1; i > 0; i--) {
            memcpy(bits[i], bits[i-1], sizeof(temp));
        }
        memcpy(bits[0], temp, sizeof(temp));        // poner última arriba
    } else if (dir < 0) {
        // Rota hacia arriba
        memcpy(temp, bits[0], sizeof(temp));        // guardar primera fila
        for (int i = 0; i < FILAS-1; i++) {
            memcpy(bits[i], bits[i+1], sizeof(temp));
        }
        memcpy(bits[FILAS-1], temp, sizeof(temp));  // poner primera abajo
    }
}

void flipVertical(uint8_t bits[FILAS][COLS]) {
    uint8_t temp[COLS];
    for (int i = 0; i < FILAS / 2; i++) {
        memcpy(temp, bits[i], sizeof(temp));                // guardar fila superior
        memcpy(bits[i], bits[FILAS - 1 - i], sizeof(temp)); // copiar fila inferior a superior
        memcpy(bits[FILAS - 1 - i], temp, sizeof(temp));   // copiar fila superior a inferior
    }
}

void eligeFila(bool a,bool b,bool c) {
  digitalWrite(a_pin,a);
  digitalWrite(b_pin,b);
  digitalWrite(c_pin,c);
}

void setup() {  
  Serial.begin(9600);
  pinMode(oe_pin, OUTPUT);
  pinMode(a_pin, OUTPUT);
  pinMode(b_pin, OUTPUT);
  pinMode(c_pin, OUTPUT);
  pinMode(clk_pin, OUTPUT);
  pinMode(sclk_pin, OUTPUT);
  pinMode(dato_pinR, OUTPUT);  
  digitalWrite(oe_pin, 0);
  digitalWrite(a_pin, 0);
  digitalWrite(b_pin, 0);
  digitalWrite(c_pin, 0);
  digitalWrite(sclk_pin, 0);
  digitalWrite(clk_pin, 0);  
  digitalWrite(dato_pinR, 0);
  writeHOLA();
  flipVertical(bits);
  subCopiar();
}

void loop() {  
  if(aux >= 1000) {
    rotateVertical(bits, 1);
    subCopiar();
    aux = 0;
  }
  
  digitalWrite(oe_pin, 0); // habilita la matriz, enciende los LEDs
  if(fila == 1) {eligeFila(1,1,1);} // fila 8 y 16 en la matriz
  if(fila == 2) {eligeFila(0,1,1);}
  if(fila == 3) {eligeFila(1,0,1);}
  if(fila == 4) {eligeFila(0,0,1);}
  if(fila == 5) {eligeFila(1,1,0);}
  if(fila == 6) {eligeFila(0,1,0);}
  if(fila == 7) {eligeFila(1,0,0);}
  if(fila == 0) {eligeFila(0,0,0);}

  for(int j=0;j<16;j++){
    digitalWrite(dato_pinR,uno[fila][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  for(int j=0;j<16;j++){
    digitalWrite(dato_pinR,dos[fila][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  for(int j=0;j<16;j++){
    digitalWrite(dato_pinR,tres[fila][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }    
  for(int j=0;j<16;j++){
    digitalWrite(dato_pinR,cuatro[fila][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  for(int j=0;j<16;j++){
    digitalWrite(dato_pinR,cinco[fila][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  for(int j=0;j<16;j++){
    digitalWrite(dato_pinR,seis[fila][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  for(int j=0;j<16;j++){
    digitalWrite(dato_pinR,siete[fila][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  for(int j=0;j<16;j++){
    digitalWrite(dato_pinR,ocho[fila][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  
  digitalWrite(sclk_pin, 1); // primer movimiento para consolidar el byte
  digitalWrite(sclk_pin, 0); // segundo movimiento para consolidar el byte
  digitalWrite(oe_pin, 1);   // deshabilita la matriz, apaga los LEDs

  fila++; // cambio de fila
  if(fila == 8) fila = 0;
  aux++;   
}
