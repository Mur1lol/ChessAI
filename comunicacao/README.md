# Comunicação

Esta pasta contém os arquivos responsáveis pela comunicação entre o Raspberry Pi 3 e o ESP32.

## Arquivos

- **comunicacao.ino** - Código Arduino para o ESP32 que gerencia a comunicação serial
- **comunicacao.py** - Código Python para testes de comunicação com o ESP32

## Funcionalidade

O sistema implementa comunicação bidirecional via USB entre:
- Raspberry Pi 3 (Python) → ESP32 (Arduino)
- ESP32 (Arduino) → Raspberry Pi 3 (Python)

### Protocolo de Comunicação

- **Formato**: JSON
- **Baudrate**: 115200
- **Comandos suportados**:
  - `teste` - Teste de comunicação
  - `tabuleiro` - Envio do estado do tabuleiro
  - `melhores_movimentos` - Envio dos movimentos possíveis
  - `movimento_computador` - Envio da jogada do computador
  - `jogada` - Recebimento da jogada do usuário

## Como usar

1. Carregue o arquivo `comunicacao.ino` no ESP32
2. Execute o arquivo `comunicacao.py` no Raspberry Pi 3
3. Teste a comunicação através do menu de opções
