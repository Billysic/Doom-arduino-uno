# 🕹️ Doom no Arduino Uno

Este projeto traz um motor gráfico de *raycasting* em 3D, fortemente inspirado nos clássicos **Doom** e **Wolfenstein 3D**, otimizado para rodar nas extremas limitações de hardware de um **Arduino Uno** (ATmega328P).

Devido à restrição de memória (apenas 2 KB de RAM e 32 KB de Flash) e processamento (16 MHz), o jogo utiliza matemática de baixa precisão para renderizar um ambiente 3D em um display OLED monocromático, incluindo cenários, inimigos, movimentação e efeitos sonoros básicos.

---

## 🛠️ Componentes Necessários

Para montar o circuito físico, você precisará dos seguintes itens:

* **1x** Placa Arduino Uno (ou Nano)
* **1x** Display OLED 0.96" I2C (128x64 pixels - 4 pinos)
* **4x** Botões de Pressão (*Push Buttons*)
* **1x** Buzzer Passivo (Para os efeitos sonoros)
* **1x** Protoboard (Matriz de contatos)
* **Vários** Fios Jumpers (Macho-Macho)

> **Nota:** Não é necessário o uso de resistores externos para os botões. O código utiliza os resistores internos do Arduino via `INPUT_PULLUP`.

---

## 🔌 Esquema de Ligação (Montagem)

Certifique-se de realizar as conexões com o Arduino desconectado do cabo USB/Energia.

### 📺 1. Display OLED (I2C)
O display exige os pinos analógicos I2C dedicados do Arduino Uno.

| Pino do Display | Pino do Arduino | Descrição |
| :--- | :--- | :--- |
| **GND** | GND | Aterramento |
| **VCC** | 5V | Alimentação |
| **SCL** | A5 | Clock do I2C |
| **SDA** | A4 | Dados do I2C |

### 🔊 2. Áudio (Buzzer)
O áudio é gerado nativamente e depende do Timer1.

| Pino do Buzzer | Pino do Arduino / Protoboard |
| :--- | :--- |
| **Positivo (+)** | D9 |
| **Negativo (-)** | Trilha GND da Protoboard |

### 🎮 3. Controles (Botões)
Conecte **um dos terminais** de todos os botões à trilha **GND** da protoboard. O terminal oposto de cada botão vai para os respectivos pinos digitais do Arduino:

| Botão (Ação) | Pino do Arduino |
| :--- | :--- |
| **Esquerda** | D2 |
| **Direita** | D3 |
| **Avançar** | D4 |
| **Atirar / Ação** | D5 |

---

## 💻 Instalação e Execução

1. Faça o download da **Arduino IDE** (caso ainda não possua).
2. Clone este repositório ou baixe o código-fonte em formato ZIP e extraia em seu computador.
3. Abra o arquivo principal `.ino` na Arduino IDE.
4. Vá em **Ferramentas > Placa** e selecione **Arduino Uno**.
5. Vá em **Ferramentas > Porta** e selecione a porta COM correspondente à sua placa.
6. Clique no botão **Carregar** (Upload).

> **Atenção:** Este projeto utiliza rotinas de renderização otimizadas desenhadas diretamente para o display. Não instale ou importe bibliotecas gráficas pesadas padrão (como a `Adafruit_SSD1306` completa) caso elas não façam parte dos arquivos locais do projeto, pois isso causará estouro de memória.

---

## 💡 Agradecimentos e Créditos

Este projeto é baseado no incrível trabalho da comunidade maker de otimização de motores de *raycasting* para microcontroladores AVR de 8 bits, com forte inspiração no projeto base `doom-nano` de *daveruiz*.