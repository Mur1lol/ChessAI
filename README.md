# Sistema de Xadrez Inteligente

Projeto de tabuleiro de xadrez físico com inteligência artificial integrada.

## Descrição

Sistema completo que combina um tabuleiro físico com engine de xadrez, permitindo que usuários joguem contra o computador (Stockfish) em um tabuleiro real com detecção automática de movimentos.

## Componentes

- **Raspberry Pi 3**: Engine Stockfish + Interface de usuário
- **ESP32**: Controle do tabuleiro físico + Comunicação
- **Sensores**: Detecção de peças nas casas do tabuleiro
- **Tabuleiro físico**: Interface tangível para o jogo

## Características

✅ **Jogo contra IA**: Powered by Stockfish
✅ **Múltiplas dificuldades**: Fácil, Médio, Difícil  
✅ **Detecção automática**: Sensores detectam movimentos
✅ **Sugestões inteligentes**: IA sugere melhores jogadas
✅ **Validação completa**: Apenas movimentos legais
✅ **Interface amigável**: Menu simples via terminal

## Requisitos

### Hardware
- ESP32 Dev Module
- 64 sensores (Hall/óticos/pressão)
- Tabuleiro de xadrez físico
- Peças de xadrez
- Cabos e componentes eletrônicos

### Software
- Python 3.7+
- Arduino IDE
- Bibliotecas: python-chess, pyserial, ArduinoJson
- Engine Stockfish

## Instalação Rápida

1. **Instale as dependências Python**:
   ```bash
   pip install python-chess pyserial
   ```

2. **Configure o Arduino IDE** para ESP32

3. **Carregue os códigos**:
   - `projeto/xadrez.ino` → ESP32
   - Execute `projeto/xadrez.py` → Raspberry Pi 3

4. **Teste a comunicação** no menu principal

## Como Jogar

1. Inicie o sistema
2. Selecione a dificuldade desejada
3. Comece uma nova partida
4. Mova as peças no tabuleiro físico
5. O sistema detecta e valida seus movimentos
6. A IA responde com sua jogada
7. Continue até o fim da partida