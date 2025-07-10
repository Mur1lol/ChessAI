#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h> // Adiciona a biblioteca NeoPixel

// Definições da Matriz e Pinos dos Sensores
#define NUM_LINHAS 8
#define NUM_COLUNAS 8

int pinosLinhas[NUM_LINHAS] = {15,4,5,18,19,21,22,23};
int pinosColunas[NUM_COLUNAS] = {17,16,32,25,26,27,14,13}; 

// Definições do NeoPixel
#define LED_PIN 33       
#define NUM_TOTAL_LEDS 128 
#define BRIGHTNESS 50     

// Objeto NeoPixel
Adafruit_NeoPixel strip(NUM_TOTAL_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

int verdadeiros_locais[64] = {
    7,6,5,4,3,2,1,0,
    8,9,10,11,12,13,14,15,
    23,22,21,20,19,18,17,16,
    24,25,26,27,28,29,30,31,
    39,38,37,36,35,34,33,32,
    40,41,42,43,44,45,46,47,
    55,54,53,52,51,50,49,48,
    56,57,58,59,60,61,62,63
};

void setup() {
    Serial.begin(115200);  // Debug via USB

    // Configura pinos das linhas como SAÍDA
    for (int i = 0; i < NUM_LINHAS; i++) {
        pinMode(pinosLinhas[i], OUTPUT);
        digitalWrite(pinosLinhas[i], HIGH); // Desativa todas as linhas inicialmente (PNP: HIGH = OFF)
    }

    // Configura pinos das colunas como ENTRADA com PULL-UP interno
    for (int j = 0; j < NUM_COLUNAS; j++) {
        pinMode(pinosColunas[j], INPUT_PULLUP);
    }

    // Inicializa a fita NeoPixel
    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    strip.clear();
    strip.show(); 
}

void loop() {
    strip.clear(); 

    for (int i = 0; i < NUM_LINHAS; i++) {
        digitalWrite(pinosLinhas[i], LOW);
        delayMicroseconds(50);

        for (int j = 0; j < NUM_COLUNAS; j++) {
            int leitura = digitalRead(pinosColunas[j]);
            int posicao = i * NUM_COLUNAS + j;

            if (leitura == LOW) {
                // Acende os dois LEDs da posição mapeada
                strip.setPixelColor(verdadeiros_locais[posicao]*2, strip.Color(0, 255, 0));
                strip.setPixelColor(verdadeiros_locais[posicao]*2 + 1, strip.Color(0, 255, 0));
            }
        }

        digitalWrite(pinosLinhas[i], HIGH);
    }

    strip.show();
}