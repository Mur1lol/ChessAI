#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

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

// Variáveis globais
bool jogadaEnviada = false;
bool novoJogo = false;
int tabuleiro[64];

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

/*********\
|* SETUP *|
\*********/ 
void setup() {
    delay(3000);  
    Serial.begin(115200);  // Comunicação USB

    // Configura pinos das linhas como SAÍDA
    for (int i = 0; i < NUM_LINHAS; i++) {
        pinMode(pinosLinhas[i], OUTPUT);
        digitalWrite(pinosLinhas[i], HIGH); 
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

//----------------------------------------------

/*************\
|* PISCA LED *|
\*************/ 

void piscar_led(const String& cor, int movimentos[] = NULL, int numMov = 0) {
    // Cores disponíveis
    uint32_t corRGB;
    if (cor == "RED") {
        corRGB = strip.Color(255, 0, 0);
    } else if (cor == "GREEN") {
        corRGB = strip.Color(0, 255, 0);
    } else if (cor == "YELLOW") {
        corRGB = strip.Color(255, 255, 0);
    } else if (cor == "BLUE") {
        corRGB = strip.Color(0, 0, 255);
    } else if (cor == "PURPLE") {
        corRGB = strip.Color(128, 0, 128);
    } else {
        corRGB = strip.Color(255, 255, 255); 
    }

    strip.clear();
    for (int i = 0; i < NUM_TOTAL_LEDS; i++) {
        strip.setPixelColor(i, corRGB);
    }
    
    strip.show(); 
    delay(1000); 
}

//----------------------------------------------

/*****************\
|* ENVIAR JOGADA *|
\*****************/ 

void enviarJogada(const int posicao) {
    StaticJsonDocument<100> doc;
    doc["comando"] = "jogada";
    doc["posicao"] = posicao;
    serializeJson(doc, Serial);
    Serial.print('\n');
}

/*************************\
|* AGUARDAR INICIALIZAÇÃO *|
\*************************/ 

void aguardarInicializacao() {
    // Serial.println("Aguardando comando de inicialização...");
    bool inicializado = false;
    
    while (!inicializado) {
        if (Serial.available()) {
            String msg = Serial.readStringUntil('\n');
            StaticJsonDocument<100> resposta; 
            if (deserializeJson(resposta, msg) == DeserializationError::Ok) {
                String comando = resposta["comando"].as<String>();
                if (comando == "iniciar_partida") {
                    // Enviar confirmação OK
                    StaticJsonDocument<100> confirmacao;
                    confirmacao["comando"] = "ok";
                    confirmacao["resposta"] = "pronto_para_receber_tabuleiro";
                    serializeJson(confirmacao, Serial);
                    Serial.print('\n');
                    
                    inicializado = true;
                } 
                else if(comando == "teste") {
                    // Enviar confirmação OK
                    StaticJsonDocument<100> confirmacao;
                    confirmacao["comando"] = "ok";
                    confirmacao["resposta"] = "teste_recebido";
                    serializeJson(confirmacao, Serial);
                    Serial.print('\n'); 
                }
            }
        } 
        delay(1000);
    }
    novoJogo = true;
}

//----------------------------------------------

/*********************\
|* RECEBER TABULEIRO *|
\*********************/ 

void receberTabuleiro(int tabuleiro[64]) {
    int mensagem_recebida = 0;
    while (!mensagem_recebida) {
        if (Serial.available()) {
            String msg = Serial.readStringUntil('\n');
            StaticJsonDocument<100> resposta; 
            if (deserializeJson(resposta, msg) == DeserializationError::Ok) {
                String comando = resposta["comando"].as<String>();
                if (comando == "tabuleiro") {
                    JsonArray arr = resposta["resposta"].as<JsonArray>();
                    int i = 0;
                    for (JsonVariant v : arr) {
                        if (i < 64) {
                            tabuleiro[i] = v.as<int>();
                            i++;
                        }
                    }
                    
                    // Enviar confirmação OK
                    StaticJsonDocument<100> confirmacao;
                    confirmacao["comando"] = "ok";
                    confirmacao["resposta"] = "tabuleiro_recebido";
                    serializeJson(confirmacao, Serial);
                    Serial.print('\n');
                    
                    mensagem_recebida = 1;
                } 
                else if (comando == "vencedor") {
                    String vencedor = resposta["resposta"].as<String>();
                    verificar_vencedor(vencedor);
                    mensagem_recebida = 1;
                }
            }
        } else {
            delay(100); 
        }
    }
}

bool verificar_tabuleiro(int tabuleiro[64]) {

    strip.clear();
        bool certo = true;
        for (int i = 0; i < NUM_LINHAS; i++) {
          digitalWrite(pinosLinhas[i], LOW);
          delayMicroseconds(50);

          for (int j = 0; j < NUM_COLUNAS; j++) {
              int leitura = digitalRead(pinosColunas[j]);
              int posicao = i * NUM_COLUNAS + j;

              // Peça na posição correta
              if (leitura == LOW && tabuleiro[posicao] == 1) {
                strip.setPixelColor(verdadeiros_locais[posicao]*2, strip.Color(0,255,0));
                strip.setPixelColor(verdadeiros_locais[posicao]*2+1, strip.Color(0,255,0));
              }
              // Falta uma peça nessa posição
              else if (leitura == HIGH && tabuleiro[posicao] == 1) {
                strip.setPixelColor(verdadeiros_locais[posicao]*2, strip.Color(255,0,0));
                strip.setPixelColor(verdadeiros_locais[posicao]*2+1, strip.Color(255,0,0));
                certo = false;
              }
              // Peça na posição errada
              else if (leitura == LOW && tabuleiro[posicao] == 0) { 
                  strip.setPixelColor(verdadeiros_locais[posicao]*2, strip.Color(128, 0, 128));
                  strip.setPixelColor(verdadeiros_locais[posicao]*2+1, strip.Color(128, 0, 128));
                  certo = false;
              }
            }
            digitalWrite(pinosLinhas[i], HIGH);
        }
        strip.show();

    return certo; 
}   

//----------------------------------------------

/*******************************\
|* RECEBER MELHORES MOVIMENTOS *|'
\*******************************/ 

int processar_origem_usuario(int tabuleiro[64]) {
    Serial.println("Jogador");
    while(true) {
        for (int i = 0; i < NUM_LINHAS; i++) {
            digitalWrite(pinosLinhas[i], LOW);
            delayMicroseconds(50);

            for (int j = 0; j < NUM_COLUNAS; j++) {
                int leitura = digitalRead(pinosColunas[j]);
                int posicao = i * NUM_COLUNAS + j;

                if (leitura == HIGH && tabuleiro[posicao] == 1) { // Sensor Hall desativado em uma casa ocupada
                    enviarJogada(posicao);
                    delay(100); 
                    return posicao; 
                }
            }
            digitalWrite(pinosLinhas[i], HIGH);
        }
    }
}

bool aguarda_resposta(int origem, int tabuleiro[64]) {
    while (true) {
        strip.clear();
        if (Serial.available()) {
            String msg = Serial.readStringUntil('\n');
            StaticJsonDocument<100> resposta; 
            if (deserializeJson(resposta, msg) == DeserializationError::Ok) {
                String comando = resposta["comando"].as<String>();
                if (comando == "confirmacao") {
                    String resp = resposta["resposta"].as<String>();
                    if (resp == "sim") {
                        tabuleiro[origem] = 0; // Marca a casa de origem como vazia
                        return true;
                    }
                    else {
                        strip.setPixelColor(verdadeiros_locais[origem]*2, strip.Color(255,0,0)); // Primeiro movimento em verde
                        strip.setPixelColor(verdadeiros_locais[origem]*2+1, strip.Color(255,0,0)); // Primeiro movimento em verde
                        strip.show();
                        return false;
                    }
                } 
            }
        } 
        else {
            delay(100); 
        }
    }
}

int receberMelhoresMovimentos(int best_moves[]) {
    while (true) {
        if (Serial.available()) {
            String msg = Serial.readStringUntil('\n');
            StaticJsonDocument<100> resposta; 
            if (deserializeJson(resposta, msg) == DeserializationError::Ok) {
                String comando = resposta["comando"].as<String>();
                if (comando == "melhores_movimentos") {
                    JsonArray arr = resposta["resposta"].as<JsonArray>();
                    int qtde = resposta["qtde"].as<int>();
                    int i=0;
                    for (JsonVariant v : arr) {
                        best_moves[i] = v.as<int>();
                        i++;
                    }

                    return qtde; 
                }
            }
        }
    }
}

void processar_destino_usuario(int origem, int tabuleiro[64], int movimentos[] = NULL, int numMov = 0) {
    jogadaEnviada = false;
    bool peca_origem_levantada = false;
    while(jogadaEnviada == false) {
        strip.clear();

        //Acende a casa de origem de branco
        strip.setPixelColor(verdadeiros_locais[origem]*2, strip.Color(255,255,255));
        strip.setPixelColor(verdadeiros_locais[origem]*2+1, strip.Color(255,255,255));

        // Melhor movimento em verde
        strip.setPixelColor(verdadeiros_locais[movimentos[0]]*2, strip.Color(0,255,0));
        strip.setPixelColor(verdadeiros_locais[movimentos[0]]*2+1, strip.Color(0,255,0));
        
        // Demais movimentos em azul
        for(int i=1; i<numMov; i++) {
            strip.setPixelColor(verdadeiros_locais[movimentos[i]]*2, strip.Color(0,255,255));
            strip.setPixelColor(verdadeiros_locais[movimentos[i]]*2+1, strip.Color(0,255,255));
        }
        
        
        for (int i = 0; i < NUM_LINHAS; i++) {
            digitalWrite(pinosLinhas[i], LOW);
            delayMicroseconds(50);

            for (int j = 0; j < NUM_COLUNAS; j++) {
                int leitura = digitalRead(pinosColunas[j]);
                int posicao = i * NUM_COLUNAS + j;

                for (int k = 0; k < numMov; k++) {
                    if (leitura == HIGH && posicao == origem) {
                        peca_origem_levantada = true;
                    }
                    else if (leitura == LOW && posicao == origem) {
                        peca_origem_levantada = false;
                        // Peça voltou para a casa de origem
                        strip.setPixelColor(verdadeiros_locais[posicao]*2, strip.Color(255,10,0));
                        strip.setPixelColor(verdadeiros_locais[posicao]*2+1, strip.Color(255,10,0));
                    }

                    if(peca_origem_levantada) {
                        if (leitura == LOW && movimentos[k] == posicao) {
                            // Apaga os outros LEDs e deixa apenas a casa escolhida Amarela
                            strip.clear();
                            strip.setPixelColor(verdadeiros_locais[posicao]*2, strip.Color(255,255,0));
                            strip.setPixelColor(verdadeiros_locais[posicao]*2+1, strip.Color(255,255,0));
                            enviarJogada(posicao);
                            
                            // Atualiza o tabuleiro
                            tabuleiro[posicao] = 1;
                            delay(100); 
                            jogadaEnviada = true; 
                            break;
                        }
                        else if (leitura == LOW && movimentos[k] != posicao && tabuleiro[posicao] == 0) {
                            // Posição não está dentro dos movimentos possiveis
                            strip.setPixelColor(verdadeiros_locais[posicao]*2, strip.Color(255,0,0));
                            strip.setPixelColor(verdadeiros_locais[posicao]*2+1, strip.Color(255,0,0));
                            
                            jogadaEnviada = false;
                        }
                    }
                }
                if(jogadaEnviada == true) {
                    break;
                }
            }
            digitalWrite(pinosLinhas[i], HIGH);
            if(jogadaEnviada == true) {
                break;
            }
        }
        strip.show();
    }
}

//----------------------------------------------

/*********************************\
|* RECEBER MOVIMENTOS COMPUTADOR *|
\*********************************/ 

void receberMovimentosComputador(int movimentos[2]) {
    int mensagem_recebida = 0;
    while (!mensagem_recebida) {
        if (Serial.available()) {
            String msg = Serial.readStringUntil('\n');
            StaticJsonDocument<100> resposta; 
            if (deserializeJson(resposta, msg) == DeserializationError::Ok) {
                String comando = resposta["comando"].as<String>();
                if (comando == "melhores_movimentos") {
                    JsonArray arr = resposta["resposta"].as<JsonArray>();
                    movimentos[0] = arr[0].as<int>(); // Origem
                    movimentos[1] = arr[1].as<int>(); // Destino
                    mensagem_recebida = 1;
                    break;
                } 
                else if (comando == "vencedor") {
                    String vencedor = resposta["resposta"].as<String>();
                    verificar_vencedor(vencedor);
                    mensagem_recebida = 1;
                    break;
                }
            }
        } else {
            delay(100); 
        }
    }
}


void processar_destino_computador(int origem, int tabuleiro[64], int movimentos[] = NULL, int numMov = 2) {
    jogadaEnviada = false;
    bool peca_origem_levantada = false;
    while(jogadaEnviada == false) {
        strip.clear();

        // Origem em Azul
        strip.setPixelColor(verdadeiros_locais[origem]*2, strip.Color(0,0,255));
        strip.setPixelColor(verdadeiros_locais[origem]*2+1, strip.Color(0,0,255));

        // Destino em Verde
        strip.setPixelColor(verdadeiros_locais[movimentos[1]]*2, strip.Color(0,255,0));
        strip.setPixelColor(verdadeiros_locais[movimentos[1]]*2+1, strip.Color(0,255,0));
        
        for (int i = 0; i < NUM_LINHAS; i++) {
            digitalWrite(pinosLinhas[i], LOW);
            delayMicroseconds(50);

            for (int j = 0; j < NUM_COLUNAS; j++) {
                int leitura = digitalRead(pinosColunas[j]);
                int posicao = i * NUM_COLUNAS + j;

                if (leitura == HIGH && posicao == origem) {
                    peca_origem_levantada = true;
                }
                else if (leitura == LOW && posicao == origem) {
                    peca_origem_levantada = false;
                }

                if(peca_origem_levantada) {
                    if (leitura == LOW && movimentos[1] == posicao) {
                        // Apaga os outros LEDs e deixa apenas a casa escolhida Amarela
                        strip.clear();
                        strip.setPixelColor(verdadeiros_locais[posicao]*2, strip.Color(255,255,0));
                        strip.setPixelColor(verdadeiros_locais[posicao]*2+1, strip.Color(255,255,0));
                            
                        enviarJogada(posicao);
                        delay(100);
                        jogadaEnviada = true;
                        break;
                    }
                    else if (leitura == LOW && movimentos[1] != posicao && tabuleiro[posicao] == 0) {
                        // Posição não está dentro dos movimentos possiveis
                        strip.setPixelColor(verdadeiros_locais[posicao]*2, strip.Color(255,10,0));
                        strip.setPixelColor(verdadeiros_locais[posicao]*2+1, strip.Color(255,10,0));
                        
                        jogadaEnviada = false;
                    }
                }
                
            }
            digitalWrite(pinosLinhas[i], HIGH);
            if(jogadaEnviada) {
                break;
            }
        }
        strip.show();
    }
}

//----------------------------------------------

/**********************\
|* VERIFICAR VENCEDOR *|
\**********************/ 

void verificar_vencedor(const String& vencedor) {
    if (vencedor == "1-0") {
        for (int i=0; i<5; i++) {
            piscar_led("GREEN");
            strip.clear();
            strip.show();
            delay(500); 
        }
    }
    else if (vencedor == "0-1") {
        for (int i=0; i<5; i++) {
            piscar_led("RED");
            strip.clear();
            strip.show();
            delay(500); 
        }
    }
    else {
        for (int i=0; i<5; i++) {
            piscar_led("YELLOW");
            strip.clear();
            strip.show();
            delay(500); 
        }
    }

    novoJogo = false;
}

//----------------------------------------------

/******************\
|* LOOP PRINCIPAL *|
\******************/ 

void loop() {
    piscar_led("PURPLE"); // Piscar LED roxo para indicar que o jogo está pronto
    delay(1000); 

    aguardarInicializacao();

    while(novoJogo) {
        strip.clear(); 
        receberTabuleiro(tabuleiro);
        delay(1000);

        if (!novoJogo) {
            break;
        }

        while(verificar_tabuleiro(tabuleiro) == false) {
            delay(100);
        }

        delay(2000);
        strip.clear();
        strip.show();
        

        /*************\ 
        |*  JOGADOR  *|
        \*************/

        int best_moves[64] = {0};
        int origem = processar_origem_usuario(tabuleiro);
        while(aguarda_resposta(origem, tabuleiro) == false) {
            origem = processar_origem_usuario(tabuleiro);
        }
        int numMov = receberMelhoresMovimentos(best_moves);

        processar_destino_usuario(origem, tabuleiro, best_moves, numMov);

        /****************\ 
        |*  COMPUTADOR  *|
        \****************/

        int movimentos[2] = {0, 0};
        receberMovimentosComputador(movimentos); 
        processar_destino_computador(movimentos[0], tabuleiro, movimentos, 2);
        if (!novoJogo) {
            break;
        }
        
        delay(100);
    }
}