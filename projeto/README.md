# Projeto Principal

Esta pasta contém os arquivos principais do sistema de xadrez.

## Arquivos

- **xadrez.ino** - Código principal do ESP32 para controle do tabuleiro físico
- **xadrez.py** - Código principal do Raspberry Pi 3 com engine de xadrez (Stockfish)

## Funcionalidade

O sistema completo de xadrez que integra:
- Engine de xadrez Stockfish (Python)
- Controle do tabuleiro físico (ESP32)
- Interface de usuário via terminal
- Comunicação serial entre dispositivos

### Recursos

- **Múltiplas dificuldades**: Fácil, Médio, Difícil
- **Sugestões de jogadas**: Powered by Stockfish
- **Validação de movimentos**: Apenas jogadas legais são aceitas
- **Detecção de fim de jogo**: Xeque-mate, empate, etc.

## Dependências

### Python (Raspberry Pi 3)
```bash
pip install python-chess
pip install pyserial
```

### Arduino (ESP32)
- Biblioteca ArduinoJson
- ESP32 Dev Module

## Como usar

1. Carregue `xadrez.ino` no ESP32
2. Execute `python xadrez.py` no Raspberry Pi 3
3. Selecione a dificuldade no menu
4. Inicie uma partida de xadrez
