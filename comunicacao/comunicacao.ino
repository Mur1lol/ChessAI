#include <ArduinoJson.h>

void setup() {
  Serial.begin(115200);  // Comunicação USB com computador
  delay(1000);
  Serial.println("ESP32 - Comunicação via USB iniciada");
}

/*
  Fluxo de comunicação:
  1. Raspberry configura a dificuldade, qual cor quer jogar e então inicia o jogo.
  2. Raspberry envia o tabuleiro para a ESP32.
  3. ESP32 verifica se os sensores estão conectados nas posições corretas de acordo com o tabuleiro recebido.
  4. ESP32 envia a posição do tabuleiro para a Raspberry Pi.
  5. Raspberry Pi envia o comando "movimentos_possiveis" para a ESP32.
  6. ESP32 envia a nova posição de acordo com os "movimentos possíveis" recebidos da Raspberry Pi.
    6.1. Raspberry Pi envia o comando "verifica_vencedor" para a ESP32.
    6.2. ESP32 celebra a vitoria ou derrota do jogador.
  7. Raspberry Pi envia o comando "movimentos_computador" para a ESP32.
  8. ESP32 realiza a jogada que o computador escolheu e envia mensagem de "OK" para a Raspberry Pi.
    8.1. Raspberry Pi envia o comando "verifica_vencedor" para a ESP32.
    8.2. ESP32 celebra a vitoria ou derrota do jogador.
  9. Volta para o passo 2.
*/

void enviarJogada(const String& posicao) {
  /*
   * Comandos:
   * - "jogada": Envia a posição da jogada para o computador via USB
   */
  StaticJsonDocument<100> doc;
  doc["comando"] = "jogada";
  doc["posicao"] = posicao;

  serializeJson(doc, Serial);
  Serial.print('\n');
}

void receberTabuleiro(int tabuleiro[64]) {
    int mensagem_recebida = 0;
    while (!mensagem_recebida) {
        if (Serial.available()) {
            String msg = Serial.readStringUntil('\n');
            StaticJsonDocument<256> resposta; // Aumente o tamanho se necessário
            if (deserializeJson(resposta, msg) == DeserializationError::Ok) {
                String comando = resposta["comando"].as<String>();
                if (comando == "tabuleiro") {
                    JsonArray arr = resposta["resposta"].as<JsonArray>();
                    int i = 0;
                    Serial.print("Tabuleiro recebido: ");
                    for (JsonVariant v : arr) {
                        if (i < 64) {
                            tabuleiro[i] = v.as<int>();
                            Serial.print(tabuleiro[i]);
                            Serial.print(" ");
                            i++;
                        }
                    }
                    Serial.println();
                    mensagem_recebida = 1;
                } else {
                    Serial.println("Comando recebido é diferente do esperado");
                    Serial.println("Comando recebido: " + comando);
                    Serial.println("Comando esperado: tabuleiro");
                }
            } else {
                Serial.println("Erro no JSON recebido");
            }
        } else {
            delay(100);  // Aguarda um pouco antes de tentar novamente
        }
    }
}

void receberDados(const String& comando_esperado) {
  int mensagem_recebida = 0;
  while (!mensagem_recebida) {
    if (Serial.available()) {
      String msg = Serial.readStringUntil('\n');
      StaticJsonDocument<100> resposta;
      if (deserializeJson(resposta, msg) == DeserializationError::Ok) {
        String comando = resposta["comando"].as<String>();
        /*
         * Comandos esperados:
         * - "tabuleiro": Recebe o tabuleiro do jogo
         * - "melhores_movimentos": Recebe os melhores movimentos possíveis
         * - "verifica_vencedor": Verifica se há um vencedor
         * - "movimento_computador": Recebe a jogada do computador
        */
        if (comando == "tabuleiro" && comando == comando_esperado) {
          // A resposta vai ser um array de 64 posições
          JsonArray tabuleiro = resposta["resposta"].as<JsonArray>();
          
          Serial.print("Tabuleiro recebido: ");
          for (JsonVariant v : tabuleiro) {
            Serial.print(v.as<int>());
            Serial.print(" ");
          }
          Serial.println();
          // Aqui você pode processar o tabuleiro recebido como array
          mensagem_recebida = 1;
        }
        else if (comando == "melhores_movimentos" && comando == comando_esperado) {
          Serial.println("Melhores movimentos recebidos: " + resposta["resposta"].as<String>());
          // Aqui você pode processar os melhores movimentos recebidos
          mensagem_recebida = 1;
        } 
        else if (comando == "verifica_vencedor" && comando == comando_esperado) {
          Serial.println("Verificando vencedor: " + resposta["resposta"].as<String>());
          // Aqui você pode verificar se há um vencedor
          mensagem_recebida = 1;
        }
        else if (comando == "movimento_computador" && comando == comando_esperado) {
          Serial.println("Movimento do computador recebido: " + resposta["resposta"].as<String>());
          // Aqui você pode processar o movimento do computador
          mensagem_recebida = 1;
        }
        else {
          Serial.println("Comando recebido é diferente do esperado");
          Serial.println("Comando recebido: " + comando);
          Serial.println("Comando esperado: " + comando_esperado);
        }
      } else {
        Serial.println("Erro no JSON recebido");
      }
    } else {
      delay(100);  // Aguarda um pouco antes de tentar novamente
    }
  }
}

void loop() {

  receberDados("tabuleiro");
  enviarJogada("Tabuleiro recebido");
  delay(2000);
  
  receberDados("melhores_movimentos");
  enviarJogada("Melhores movimentos recebidos");
  delay(2000);

  receberDados("movimento_computador");
  enviarJogada("Movimento do computador recebido");
  delay(2000);
}