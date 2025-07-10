# Sensores

Esta pasta contém o código para controle e leitura dos sensores do tabuleiro de xadrez.

## Arquivos

- **sensores.ino** - Código Arduino para leitura dos sensores de posição das peças

## Funcionalidade

Sistema de detecção de peças no tabuleiro físico através de sensores.

### Características

- **Detecção de peças**: Identifica presença/ausência de peças em cada casa
- **Mapeamento 8x8**: Cobre todas as 64 casas do tabuleiro
- **Interface digital**: Comunicação com o sistema principal via pinos digitais

## Configuração

1. Conecte os sensores aos pinos especificados no código
2. Carregue o arquivo `sensores.ino` no ESP32
3. Teste a detecção em cada casa do tabuleiro

## Integração

Este módulo se integra com o sistema principal para:
- Detectar movimentos do jogador
- Validar posições das peças
- Confirmar execução de jogadas
