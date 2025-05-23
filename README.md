# Projeto - Controle Luminar, Display OLED, LEDs WS2812 e Buzzer

Este projeto foi desenvolvido para o Raspberry Pi Pico usando a Pico-SDK em C, e integra múltiplos dispositivos como joystick analógico, botões, matriz de LEDs WS2812, display OLED SSD1306 e buzzer.  
O sistema possibilita desenhar e navegar usando o joystick, alterar cores, tocar músicas e controlar LEDs RGB.

* **Funcionalidades**
    * Controle de cursor no display OLED via joystick analógico.
    * Borda animada no display que pode piscar.
    * Matriz de LEDs WS2812 (5x5) controlada por joystick, exibindo a posição atual.
    * Controle de cores da matriz e ativação de músicas usando botões físicos.
    * Ciclo automático de cores nos LEDs RGB.
    * Buzzer para tocar melodias curtas.

* **Hardware Utilizado**
    * Raspberry Pi Pico
    * Display OLED SSD1306 (via I2C)
    * LEDs RGB comuns
    * Matriz de LEDs WS2812 (5x5)
    * Joystick analógico (com botão)
    * Botões físicos
    * Buzzer passivo

* **Pinagem**
    * **Componente** - **GPIO**
    * Display SDA - 14
    * Display SCL - 15
    * LED Vermelho - 13
    * LED Azul - 12
    * LED Verde - 11
    * Botão A - 5
    * Botão B - 6
    * Joystick X - 26 (ADC0)
    * Joystick Y - 27 (ADC1)
    * Botão Joystick - 22
    * Matriz WS2812 - 7
    * Buzzer - 10

* **Estrutura do Projeto**
    * **Display OLED:**
      * Comunicação via I2C.
      * Mostra um cursor controlado pelo joystick.

    * **Matriz de LEDs WS2812:**
      * Controlada conforme a posição do joystick.
      * Permite mudar as cores dos LEDs via botões.
    
    * **LEDs RGB:**
      * Ciclo automático de cores para indicar estados do sistema.
    
    * **Buzzer:**
      * Toca melodias simples conforme ações no sistema.

    * **Joystick Analógico:**
      * Movimenta o cursor no display OLED.
      * Controla a posição dos LEDs acesos na matriz WS2812.
    
    * **Botões Físicos:**
      * Alteram cores da matriz.
      * Disparam músicas no buzzer.

* **Organização do Código**
  * Inicializações de periféricos:
    * led_init()
    * button_init()
    * display_init()
    * iniciar_adc()
    * matrix_init()
    * buzzer_init()

  * Controle de fluxo:
    * Temporização de LEDs RGB.
    * Atualização da matriz WS2812
