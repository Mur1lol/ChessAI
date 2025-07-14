import serial
import json
import time
import serial.tools.list_ports
import chess
import chess.engine

# Configurações
BAUDRATE = 115200
TIMEOUT = 10

class XadrezESP32:
    def __init__(self):
        self.ser = None
        self.dificuldade = "Facil"
        self.dificuldade_depth = 3
        self.STOCKFISH_PATH = "/usr/games/stockfish"
        self.board = None
        self.conectar_esp32()
    
    #####################
    # CONEXÃO COM ESP32 #
    #####################
    
    def encontrar_porta_esp32(self):
        """Encontra automaticamente a porta do ESP32"""
        while True:
            ports = serial.tools.list_ports.comports()
            
            if not ports:
                print("Nenhuma porta serial encontrada!")
                print("Verifique se o ESP32 está conectado via USB")
                continue

            print(f"Portas seriais encontradas: {len(ports)}")
            
            esp32_ports = []
            for port in ports:
                print(f"  {port.device} - {port.description}")
                if any(keyword in port.description.upper() for keyword in 
                    ['USB', 'CH340', 'CP210', 'ESP32', 'SILICON LABS', 'USB-SERIAL CH340', 'USB SERIAL']):
                    esp32_ports.append(port)
                    print(f"    Possível ESP32 detectado!")
            
            if not esp32_ports:
                print("⚠️ Nenhuma porta com padrão de ESP32 detectada.")
                continue
            
            porta_escolhida = esp32_ports[0].device
            print(f"Porta ESP32 selecionada: {porta_escolhida} - {esp32_ports[0].description}")
            return porta_escolhida
        
    
    def conectar_esp32(self):
        """Conecta à porta do ESP32"""
        porta_esp32 = self.encontrar_porta_esp32()
        
        if not porta_esp32:
            print("Nenhuma porta disponível para conexão")
            exit(1)
        
        print(f"Tentando conectar à porta {porta_esp32}...")
        
        try:
            self.ser = serial.Serial(
                port=porta_esp32,
                baudrate=BAUDRATE,
                timeout=TIMEOUT,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                bytesize=serial.EIGHTBITS
            )
            
            time.sleep(2)
            self.ser.reset_input_buffer()
            
            if self.ser.is_open:
                print(f"Conectado à porta {porta_esp32}")
                print(f"Configuração: {BAUDRATE} baud, timeout {TIMEOUT}s")
                self.testar_comunicacao_inicial()
            else:
                print(f"Falha ao abrir porta {porta_esp32}")
                exit(1)
                
        except serial.SerialException as e:
            print(f"Erro de comunicação serial: {e}")
            print("Possíveis soluções:")
            print("   - Verifique se o ESP32 está conectado")
            print("   - Verifique se nenhum outro programa está usando a porta")
            print("   - Tente desconectar e reconectar o cabo USB")
            print("   - Verifique se os drivers estão instalados")
            exit(1)
        except Exception as e:
            print(f"Erro inesperado ao conectar: {e}")
            exit(1)
    
    ###############
    # COMUNICAÇÃO #
    ###############

    def testar_comunicacao_inicial(self):
        """Testa a comunicação inicial com o ESP32"""
        print("Testando comunicação inicial...")
        
        while True:
            try:
                self.ser.reset_input_buffer()
                self.ser.reset_output_buffer()
                time.sleep(2)
                
                if self.enviar_comando("teste", "ping"):
                    resposta = self.aguardar_resposta(timeout=20)
                    
                    if resposta:
                        print("Comunicação inicial estabelecida com sucesso!")
                        return True
                    else:
                        print(f"Tentativa falhou - sem resposta JSON válida")
                else:
                    print(f"Tentativa falhou - erro no envio")
                    
            except Exception as e:
                print(f"Erro: {e}")
            
            time.sleep(2)
    
    def aguardar_resposta(self, timeout=120):
        """Aguarda uma resposta do ESP32"""
        if not self.ser or not self.ser.is_open:
            print("Conexão serial não disponível")
            return None
            
        original_timeout = self.ser.timeout
        
        if timeout:
            self.ser.timeout = timeout
        
        try:
            tempo_inicio = time.time()
            
            while True:
                if timeout and (time.time() - tempo_inicio) > timeout:
                    print(f"Timeout após {timeout} segundos")
                    return None
                
                try:
                    raw = self.ser.readline()
                    
                    if raw:
                        linha = raw.decode('utf-8', errors='ignore').strip()
                        
                        if linha:
                            print(f"Dados recebidos: {linha}")
                            
                            if linha.startswith("{") and linha.endswith("}"):
                                try:
                                    resposta = json.loads(linha)
                                    print(f"JSON válido recebido: {resposta}")
                                    return resposta
                                except json.JSONDecodeError as je:
                                    print(f"JSON inválido: {linha} - Erro: {je}")
                            else:
                                print(f"Dados não-JSON ignorados: {linha}")
                    else:
                        time.sleep(0.05)
                        
                except serial.SerialException as e:
                    print(f"Erro de comunicação serial: {e}")
                    return None
                except UnicodeDecodeError:
                    continue
                except Exception as e:
                    print(f"Erro ao ler dados: {e}")
                    return None
                    
        except Exception as e:
            print(f"Erro inesperado ao aguardar resposta: {e}")
            return None
        finally:
            if timeout:
                self.ser.timeout = original_timeout
    
    def fechar_conexao(self):
        """Fecha a conexão serial"""
        if self.ser and self.ser.is_open:
            self.ser.close()
            print("Conexão serial fechada")

    def enviar_comando(self, comando, resposta):
        """Envia um comando JSON para o ESP32"""
        try:
            comando_json = json.dumps({"comando": comando, "resposta": resposta, "qtde": len(resposta), "dificuldade": self.dificuldade_depth}) + '\n'
            
            self.ser.reset_output_buffer()
            bytes_enviados = self.ser.write(comando_json.encode('utf-8'))
            self.ser.flush()
            
            print(f"Enviado: {comando} -> {str(resposta)[:100]}... ({bytes_enviados} bytes)")
            return True
            
        except serial.SerialException as e:
            print(f"Erro de comunicação serial ao enviar: {e}")
            return False
        except Exception as e:
            print(f"Erro inesperado ao enviar comando: {e}")
            return False
    
    ###########
    # JOGADOR #
    ###########    
    
    def aguardar_jogada_usuario(self, timeout=10000):
        """Aguarda o usuário fazer uma jogada"""
        print("\nAguardando jogada do usuário...")
        resposta = self.aguardar_resposta(timeout=timeout)

        if resposta and resposta.get("comando") == "jogada":
            posicao = resposta.get("posicao")
            print(f"Jogada do usuário recebida: posição {posicao}")
            return posicao
        else:
            print("Jogada do usuário não recebida")
            return None
    
    def movimentos_possiveis(self, origem):
        """Filtra todos os movimentos legais que saem de uma casa específica"""
        try:
            origem_square = chess.parse_square(origem)
            return [move for move in self.board.legal_moves if move.from_square == origem_square]
        except:
            return []

    def melhor_jogada_para_casa(self, engine, movimentos):
        """Usa o Stockfish para encontrar a melhor jogada entre as disponíveis"""
        if not movimentos:
            return None
        melhores = []
        scores = []
        for move in movimentos:
            self.board.push(move)
            try:
                score = engine.analyse(self.board, chess.engine.Limit(depth=8))["score"].white().score(mate_score=10000)
                scores.append(score if score is not None else 0)
                melhores.append(move)
            except:
                scores.append(0)
                melhores.append(move)
            self.board.pop()
        
        if scores:
            melhor_score = max(scores)
            idx = scores.index(melhor_score)
            return melhores[idx]
        return None
    
    def enviar_melhores_movimentos(self, movimentos):
        """Envia os melhores movimentos possíveis"""
        print(f"\nEnviando melhores movimentos: {movimentos}")
        if not self.enviar_comando("melhores_movimentos", movimentos):
            return False
        return True
    
    ##############
    # COMPUTADOR #
    ##############
    def enviar_movimento_computador(self, origem, destino):
        """Envia o movimento do computador"""
        movimentos = [origem, destino]
        print(f"\nEnviando movimento do computador: {origem} -> {destino}")
        if not self.enviar_comando("melhores_movimentos", movimentos):
            return False
        return True
    
    ######################
    # VERIFICAR VENCEDOR #
    ######################
    def verificar_vencedor(self, resultado):
        """Envia resultado da partida"""
        print(f"\nEnviando resultado: {resultado}")
        if not self.enviar_comando("vencedor", resultado):
            return False
        return True
    
    ##########################
    # TRADUÇÃO DE MOVIMENTOS #
    ##########################
    def traduzir_movimento_jogador(self, posicao):
        """Traduz 0 para a1, 1 para b1, ..., 63 para h8"""
        posicao = str(posicao)
        dicionario = {
            '0': "a1", '1': "b1", '2': "c1", '3': "d1", '4': "e1", '5': "f1", '6': "g1", '7': "h1",
            '8': "a2", '9': "b2", '10': "c2", '11': "d2", '12': "e2", '13': "f2", '14': "g2", '15': "h2",
            '16': "a3", '17': "b3", '18': "c3", '19': "d3", '20': "e3", '21': "f3", '22': "g3", '23': "h3",
            '24': "a4", '25': "b4", '26': "c4", '27': "d4", '28': "e4", '29': "f4", '30': "g4", '31': "h4",
            '32': "a5", '33': "b5", '34': "c5", '35': "d5", '36': "e5", '37': "f5", '38': "g5", '39': "h5",
            '40': "a6", '41': "b6", '42': "c6", '43': "d6", '44': "e6", '45': "f6", '46': "g6", '47': "h6",
            '48': "a7", '49': "b7", '50': "c7", '51': "d7", '52': "e7", '53': "f7", '54': "g7", '55': "h7",
            '56': "a8", '57': "b8", '58': "c8", '59': "d8", '60': "e8", '61': "f8", '62': "g8", '63': "h8"
        }
        return dicionario.get(posicao, "invalid")

    def traduzir_movimento_computador(self, movimento):
        """Traduz a1 para 0, b1 para 1, ..., h8 para 63"""
        dicionario = {
            "a1": 0, "b1": 1, "c1": 2, "d1": 3, "e1": 4, "f1": 5, "g1": 6, "h1": 7,
            "a2": 8, "b2": 9, "c2": 10, "d2": 11, "e2": 12, "f2": 13, "g2": 14, "h2": 15,
            "a3": 16, "b3": 17, "c3": 18, "d3": 19, "e3": 20, "f3": 21, "g3": 22, "h3": 23,
            "a4": 24, "b4": 25, "c4": 26, "d4": 27, "e4": 28, "f4": 29, "g4": 30, "h4": 31,
            "a5": 32, "b5": 33, "c5": 34, "d5": 35, "e5": 36, "f5": 37, "g5": 38, "h5": 39,
            "a6": 40, "b6": 41, "c6": 42, "d6": 43, "e6": 44, "f6": 45, "g6": 46, "h6": 47,
            "a7": 48, "b7": 49, "c7": 50, "d7": 51, "e7": 52, "f7": 53, "g7": 54, "h7": 55,
            "a8": 56, "b8": 57, "c8": 58, "d8": 59, "e8": 60, "f8": 61, "g8": 62, "h8": 63
        }
        return dicionario.get(movimento, -1)

    ###################
    # INICIAR PARTIDA #
    ###################
    def definir_dificuldade(self):
        """Menu para definir a dificuldade do jogo"""
        print("\nConfigurar dificuldade")
        print("1. Fácil")
        print("2. Médio")
        print("3. Difícil")
        
        while True:
            try:
                opcao = input(f"\nDificuldade atual: {self.dificuldade}\nEscolha nova dificuldade (1-3): ").strip()
                
                if opcao == "1":
                    self.dificuldade = "Fácil"
                    self.dificuldade_depth = 3
                    break
                elif opcao == "2":
                    self.dificuldade = "Médio"
                    self.dificuldade_depth = 6
                    break
                elif opcao == "3":
                    self.dificuldade = "Difícil"
                    self.dificuldade_depth = 10
                    break
                else:
                    print("Opção inválida! Digite um número de 1 a 3.")
            except Exception as e:
                print(f"Erro: {e}")
        
        print(f"Dificuldade definida como: {self.dificuldade}")
    
    def iniciar_partida(self):
        """Inicia uma nova partida com chess engine"""
        print(f"\nIniciando nova partida")
        print(f"Dificuldade: {self.dificuldade} (Depth: {self.dificuldade_depth})")
        
        self.board = chess.Board()
        
        if not self.enviar_comando("iniciar_partida", "iniciar"):
            return False
        
        resposta = self.aguardar_resposta(timeout=10000)
        if not resposta or resposta.get("comando") != "ok":
            print("ESP32 não confirmou inicialização")
            return False
        
        print("ESP32 confirmou inicialização!")
        
        try:
            with chess.engine.SimpleEngine.popen_uci(self.STOCKFISH_PATH) as engine:
                print(f"Motor Stockfish iniciado - Dificuldade: {self.dificuldade}")
                
                turno = 1
                while not self.board.is_game_over():
                    print("\nTabuleiro atual:")
                    print(self.board)
                    
                    tabuleiro = [0 for _ in range(64)]
                    for index, piece in enumerate(self.board.piece_map()):
                        tabuleiro[piece] = 1
                        
                    self.enviar_comando("tabuleiro", tabuleiro)
                    
                    resposta = self.aguardar_resposta(timeout=10000)
                    if not resposta or resposta.get("comando") != "ok":
                        print("ESP32 não confirmou recebimento do tabuleiro")
                        return False
                    
                    print("ESP32 confirmou recebimento do tabuleiro!")
                    
                    print(f"\nTurno {turno}")
                    print(f"Tabuleiro atual (FEN): {self.board.fen()}")
                    print(f"Vez do {'BRANCO' if self.board.turn else 'PRETO'}")
                    
                    print("\nJogada do usuário")
                    while True:
                        try:
                            posicao_origem = self.aguardar_jogada_usuario(1000000)
                            if posicao_origem < 0 or posicao_origem > 63:
                                print("Timeout ou erro na escolha da origem")
                                continue
                            
                            origem_chess = self.traduzir_movimento_jogador(posicao_origem)
                            movimentos_possiveis = self.movimentos_possiveis(origem_chess)
                            if not movimentos_possiveis:
                                print(f"Nenhum movimento legal possível de {origem_chess}")
                                self.enviar_comando("confirmacao", "nao")
                                continue
                            
                            self.enviar_comando("confirmacao", "sim")
                            break

                        except Exception as e:
                            print(f"Erro ao processar jogada do usuário: {e}")
                            self.enviar_comando("confirmacao", "nao")
                            continue
                    
                    print(f"Analisando melhores jogadas de {origem_chess} (posição {posicao_origem})...")
                    melhor_movimento = self.melhor_jogada_para_casa(engine, movimentos_possiveis)
                    
                    destinos_possiveis = []
                    if melhor_movimento:
                        melhor_destino = self.traduzir_movimento_computador(melhor_movimento.uci()[2:4])
                        destinos_possiveis.append(melhor_destino)
                        print(f"Melhor jogada sugerida: {origem_chess} -> {melhor_movimento.uci()[2:4]}")
                    
                    for movimento in movimentos_possiveis:
                        destino = self.traduzir_movimento_computador(movimento.uci()[2:4])
                        if destino not in destinos_possiveis:
                            destinos_possiveis.append(destino)
                    
                    print(f"Destinos possíveis: {destinos_possiveis}")
                    
                    if not self.enviar_melhores_movimentos(destinos_possiveis):
                        break
                    
                    print("\nAguardando usuário escolher destino...")
                    posicao_destino = self.aguardar_jogada_usuario()
                    if posicao_destino < 0 or posicao_destino > 63:
                        print("Timeout ou erro na escolha do destino")
                        continue

                    destino_chess = self.traduzir_movimento_jogador(posicao_destino)
                    jogada_uci = origem_chess + destino_chess
                    print(f"Jogada do usuário: {jogada_uci}")
                    
                    try:
                        movimento = chess.Move.from_uci(jogada_uci)
                        if movimento not in self.board.legal_moves:
                            # Tentativa de promoção automática para dama
                            movimento_promocao = chess.Move.from_uci(jogada_uci + 'q')
                            if movimento_promocao in self.board.legal_moves:
                                movimento = movimento_promocao
                            else:
                                print(f"Jogada {jogada_uci} é inválida!")
                                continue
                        self.board.push(movimento)
                        print(f"Jogada {movimento.uci()} executada com sucesso!")
                    except Exception as e:
                        print(f"Erro ao processar jogada {jogada_uci}: {e}")
                        continue
                    
                    if self.board.is_game_over():
                        break
                    
                    print(f"\nJogada do computador ({self.dificuldade})")
                    
                    resultado_engine = engine.play(self.board, chess.engine.Limit(depth=self.dificuldade_depth))
                    movimento_computador = resultado_engine.move
                    
                    origem_comp = self.traduzir_movimento_computador(movimento_computador.uci()[:2])
                    destino_comp = self.traduzir_movimento_computador(movimento_computador.uci()[2:4])
                    
                    print(f"Stockfish escolheu: {movimento_computador.uci()} ({origem_comp} -> {destino_comp})")
                    
                    if not self.enviar_movimento_computador(origem_comp, destino_comp):
                        break
                    
                    print("Aguardando execução física da jogada do computador...")
                    confirmacao = self.aguardar_jogada_usuario(10000000)
                    if confirmacao is None:
                        print("Timeout aguardando confirmação da jogada do computador")
                        break

                    # Força promoção para dama, se for uma promoção
                    if movimento_computador.promotion and movimento_computador.promotion != chess.QUEEN:
                        movimento_forcado = chess.Move(
                            movimento_computador.from_square,
                            movimento_computador.to_square,
                            promotion=chess.QUEEN
                        )
                        if movimento_forcado in self.board.legal_moves:
                            print(f"Substituindo promoção sugerida por: {movimento_forcado.uci()} (promoção para dama)")
                            movimento_computador = movimento_forcado
                    
                    self.board.push(movimento_computador)
                    print(f"Jogada do computador {movimento_computador.uci()} executada!")
                    
                    turno += 1
                
                print(f"\nFim de jogo")
                resultado = self.board.result()
                print(f"Resultado: {resultado}")
                print(f"Motivo: {self.board.outcome().termination if self.board.outcome() else 'Desconhecido'}")
                
                if resultado == "1-0":
                    print("Vitória das BRANCAS!")
                    resultado_msg = "1-0"
                    self.dificuldade_depth = self.dificuldade_depth % 12 + 3
                elif resultado == "0-1":
                    print("Vitória das PRETAS!")
                    resultado_msg = "0-1"
                else:
                    print("EMPATE!")
                    resultado_msg = "1/2-1/2"
                
                self.verificar_vencedor(resultado_msg)
                print(f"Partida finalizada")
                
                return True
                
        except Exception as e:
            print(f"Erro ao iniciar motor Stockfish: {e}")
            print("Certifique-se de que o Stockfish está instalado e no PATH do sistema")
            return False

def main():    
    xadrez = XadrezESP32()
    
    print("\nBem-vindo ao jogo de Xadrez com ESP32!")
    try:
        while True:
            xadrez.iniciar_partida()
                
            # print("\nMenu principal")
            # print("1. Definir Dificuldade")
            # print("2. Jogar Partida de Xadrez")
            # print("3. Teste de Comunicação")
            # print("0. Sair")
            
            # opcao = input("\nEscolha uma opção: ").strip()
            
            # if opcao == "1":
            #     xadrez.definir_dificuldade()
            # elif opcao == "2":
            #     xadrez.iniciar_partida()
            # elif opcao == "3":
            #     print("Testando comunicação...")
            #     sucesso = False
            #     for tentativa in range(1, 4):
            #         print(f"Tentativa {tentativa}/3...")
            #         if xadrez.enviar_comando("teste", "ping"):
            #             resposta = xadrez.aguardar_resposta(timeout=10)
            #             if resposta:
            #                 print("Comunicação OK")
            #                 sucesso = True
            #                 break
            #             else:
            #                 print(f"Tentativa {tentativa} falhou - sem resposta")
            #         else:
            #             print(f"Tentativa {tentativa} falhou - erro no envio")
                    
            #         if tentativa < 3:
            #             time.sleep(2)
                
            #     if not sucesso:
            #         print("Falha na comunicação após 3 tentativas")
                    
            # elif opcao == "0":
            #     print("Encerrando programa...")
            #     break
            # else:
            #     print("Opção inválida!")
        
    except KeyboardInterrupt:
        print("\nPrograma interrompido pelo usuário (Ctrl+C)")
    except Exception as e:
        print(f"\nErro inesperado: {e}")
    finally:
        xadrez.fechar_conexao()

if __name__ == "__main__":
    main()