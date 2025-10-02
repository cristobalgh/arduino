// P4.75 HUB12 1/8 scan 16x64 de un solo color, matrix control.
// ESP32 dev module
#include <letras.h>

#define oe_pin     13 // OE en conector IDC HUB12
#define a_pin      12 // A en conector IDC HUB12
#define b_pin      14 // B en conector IDC HUB12
#define c_pin      33 // C en conector IDC HUB12
#define clk_pin    27 // S o CLK en conector IDC HUB12
#define sclk_pin   26 // L o SCLK o STB o LAT en conector IDC HUB12
#define dato_pinR  25 // R o Data en conector IDC HUB12

#define FILAS 16
#define COLS  64

int fila  = 1;
int aux   = 0;

uint8_t bits[FILAS][COLS]; //matriz de trabajo
uint8_t uno[8][16], dos[8][16], tres[8][16], cuatro[8][16],
        cinco[8][16], seis[8][16], siete[8][16], ocho[8][16]; //sub matrices

void subCopiar(){
  copySubMatrix(bits, uno, 8, 0); //copySubMatrix(grande, chica, filaInicial, colInicial)
  copySubMatrix(bits, dos, 0, 0);
  copySubMatrix(bits, tres, 8, 16);
  copySubMatrix(bits, cuatro, 0, 16);
  copySubMatrix(bits, cinco, 8, 32);
  copySubMatrix(bits, seis, 0, 32);
  copySubMatrix(bits, siete, 8, 48);
  copySubMatrix(bits, ocho, 0, 48);
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

void rotate_horizontal(uint8_t matrix[16][64], int direction) {
    if (direction == 1) { // rotar a la derecha
        for (int r = 0; r < 16; r++) {
            uint8_t last = matrix[r][63]; // guardar última columna
            for (int c = 63; c > 0; c--) {
                matrix[r][c] = matrix[r][c-1];
            }
            matrix[r][0] = last; // colocar última en primera
        }
    } else if (direction == -1) { // rotar a la izquierda
        for (int r = 0; r < 16; r++) {
            uint8_t first = matrix[r][0]; // guardar primera columna
            for (int c = 0; c < 63; c++) {
                matrix[r][c] = matrix[r][c+1];
            }
            matrix[r][63] = first; // colocar primera al final
        }
    }
}

void eligeFila(bool a,bool b,bool c) {
  digitalWrite(a_pin,a);
  digitalWrite(b_pin,b);
  digitalWrite(c_pin,c);
}

// Versión genérica: deduce tamaño de la matriz automáticamente
template <size_t filas, size_t cols>
void imprimirMatriz(uint8_t (&matriz)[filas][cols]) {
  for (size_t i = 0; i < filas; i++) {
    for (size_t j = 0; j < cols; j++) {
      Serial.print(matriz[i][j]);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void escribe(){
  digitalWrite(oe_pin, 0);   // habilita la matriz, enciende los LEDs
  subCopiar();//la grande a 8 chicas
  if(fila == 1) {eligeFila(1,1,1);}
  if(fila == 2) {eligeFila(0,1,1);}
  if(fila == 3) {eligeFila(1,0,1);}
  if(fila == 4) {eligeFila(0,0,1);}
  if(fila == 5) {eligeFila(1,1,0);}
  if(fila == 6) {eligeFila(0,1,0);}
  if(fila == 7) {eligeFila(1,0,0);}
  if(fila == 8) {eligeFila(0,0,0);} //8 la cambio a 0

  for(int j=15;j>=0;j--){
    digitalWrite(dato_pinR,ocho[fila-1][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  for(int j=15;j>=0;j--){
    digitalWrite(dato_pinR,siete[fila-1][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  for(int j=15;j>=0;j--){
    digitalWrite(dato_pinR,seis[fila-1][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }    
  for(int j=15;j>=0;j--){
    digitalWrite(dato_pinR,cinco[fila-1][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  for(int j=15;j>=0;j--){
    digitalWrite(dato_pinR,cuatro[fila-1][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  for(int j=15;j>=0;j--){
    digitalWrite(dato_pinR,tres[fila-1][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  for(int j=15;j>=0;j--){
    digitalWrite(dato_pinR,dos[fila-1][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  for(int j=15;j>=0;j--){
    digitalWrite(dato_pinR,uno[fila-1][j]);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  digitalWrite(sclk_pin, 1); // primer movimiento para consolidar el byte
  digitalWrite(sclk_pin, 0); // segundo movimiento para consolidar el byte
  
  for(int j=0;j<128;j++){   // clear data para no ghosting!
    digitalWrite(dato_pinR,0);
    digitalWrite(clk_pin,1);
    digitalWrite(clk_pin,0);
  }
  digitalWrite(sclk_pin, 1); // primer movimiento para consolidar el byte
  digitalWrite(sclk_pin, 0); // segundo movimiento para consolidar el byte
  
  fila++; // cambio de fila
  if(fila == 9) fila = 1;
  
  digitalWrite(oe_pin, 1);   // deshabilita la matriz, apaga los LEDs
  //delay(2);                  // aca puedo atenuar los leds
}

void setup() {  
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
  //Serial.begin(115200);
  //generarMatriz(bits,1,1); // pone un 1 en fila, col 0,0 y 15,63 arriba izq y abajo der
  write_text(bits,"Maipo");
}

void loop() {  
  if(aux >= 300){
    rotate_horizontal(bits,1);
    rotateVertical(bits, 1);
    aux = 0;
  }
  aux++;
  escribe();
}
