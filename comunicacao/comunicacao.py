import serial
import json
import time
import serial.tools.list_ports

# Função para encontrar automaticamente a porta do ESP32
def encontrar_porta_esp32():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        # Procura por dispositivos USB que podem ser ESP32
        if 'USB' in port.description or 'CH340' in port.description or 'CP210' in port.description or 'ESP32' in port.description:
            print(f"Porta ESP32 encontrada: {port.device} - {port.description}")
            return port.device
    
    # Se não encontrar automaticamente, lista todas as portas disponíveis
    print("Portas disponíveis:")
    for port in ports:
        print(f"  {port.device} - {port.description}")
    
    # Solicita ao usuário para inserir a porta manualmente
    # porta = input("Digite a porta COM do ESP32 (ex: COM3): ").strip()
    porta = ports[0].device if ports else None
    return porta

# Encontra e conecta à porta do ESP32
porta_esp32 = encontrar_porta_esp32()
try:
    ser = serial.Serial(porta_esp32, 115200, timeout=1)
    print(f"Conectado à porta {porta_esp32}")
except Exception as e:
    print(f"Erro ao conectar à porta {porta_esp32}: {e}")
    exit(1)

time.sleep(2)
ser.reset_input_buffer()

def enviar_comando(cmd, rsp):
    """Envia um comando JSON para o ESP32 via USB"""
    comando_json = json.dumps({"comando": cmd, "resposta": rsp}) + '\n'
    ser.write(comando_json.encode())
    print(f"Enviado: {comando_json.strip()}")

def receber_jogada():
    """Recebe uma jogada do ESP32 via USB"""
    while True:
        raw = ser.readline()
        linha = raw.decode('utf-8', errors='ignore').strip()
        if linha:
            try:
                dado = json.loads(linha)
                print(f"Jogada recebida: {dado}")
                return dado
            except json.JSONDecodeError:
                print(f"Ignorado JSON inválido: {linha}")
        time.sleep(0.1)  # Pequena pausa para evitar uso excessivo de CPU

try:
    print("=== Sistema de Comunicação USB ESP32 ===")
    iniciar = input("Iniciar o jogo? (s/n): ").strip().lower()
    
    if iniciar == 's':
        print("\nIniciando comunicação com ESP32...")
        
        # Exemplo de tabuleiro inicial (64 posições)
        tabuleiro = [1,1,1,1,1,1,1,1,  # Linha 1 - peças pretas
                    1,1,1,1,1,1,1,1,   # Linha 2 - peões pretos
                    0,0,0,0,0,0,0,0,   # Linha 3 - vazia
                    0,0,0,0,0,0,0,0,   # Linha 4 - vazia
                    0,0,0,0,0,0,0,0,   # Linha 5 - vazia
                    0,0,0,0,0,0,0,0,   # Linha 6 - vazia
                    1,1,1,1,1,1,1,1,   # Linha 7 - peões brancos
                    1,1,1,1,1,1,1,1]   # Linha 8 - peças brancas
        
        # Exemplo de melhores movimentos (posições válidas)
        melhores_movimentos = [8, 9, 10, 16, 17, 18]  # Posições onde o jogador pode mover
        
        print("1. Enviando tabuleiro...")
        enviar_comando("tabuleiro", tabuleiro)
        receber_jogada()
        
        print("\n2. Enviando melhores movimentos...")
        enviar_comando("melhores_movimentos", melhores_movimentos)
        receber_jogada()
        
        print("\n3. Verificando vencedor...")
        enviar_comando("verifica_vencedor", "nenhum")
        receber_jogada()
        
        print("\n4. Enviando movimento do computador...")
        enviar_comando("movimento_computador", [15, 31])  # [origem, destino]
        receber_jogada()
        
        print("\nComunicação concluída com sucesso!")
    else:
        print("Programa encerrado pelo usuário.")

except KeyboardInterrupt:
    print("\nPrograma interrompido pelo usuário (Ctrl+C)")
except Exception as e:
    print(f"Erro durante a execução: {e}")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("Conexão serial fechada.")
