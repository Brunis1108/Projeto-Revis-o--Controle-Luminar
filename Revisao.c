// Bibliotecas necessárias
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "ws2812.pio.h"

/* ========== DEFINIÇÕES DE PINOS ========== */

// Display OLED (I2C)
#define I2C_PORT i2c1          // Porta I2C utilizada
#define PIN_I2C_SDA 14         // Pino SDA (GPIO 14)
#define PIN_I2C_SCL 15         // Pino SCL (GPIO 15)
#define endereco 0x3C          // Endereço I2C do display

// LEDs RGB individuais
#define led_red 13             // LED Vermelho (GPIO 13)
#define led_blue 12            // LED Azul (GPIO 12)
#define led_green 11           // LED Verde (GPIO 11)

// Botões
#define buttonA 5              // Botão A (GPIO 5)
#define buttonB 6              // Botão B (GPIO 6)

// Joystick
#define analogicox 26          // Eixo X (GPIO 26 - ADC0)
#define analogicoy 27          // Eixo Y (GPIO 27 - ADC1)
#define botao_joystick 22      // Botão do joystick (GPIO 22)

// Configurações PWM
const uint16_t period = 4096;  // Período do PWM (12 bits)
const float divider_pwm = 16.0; // Divisor de frequência

// Matriz de LEDs WS2812
#define MATRIX_LED 7           // Pino da matriz (GPIO 7)
#define NUM_PIXELS 25          // 5x5 = 25 LEDs

// Buzzer
#define BUZZER_PIN 10          // Pino do buzzer (GPIO 10)
#define REST 0                 // Define repouso para o buzzer
// Notas musicais
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659

/* ========== VARIÁVEIS GLOBAIS ========== */

// Display OLED
ssd1306_t ssd;                 // Objeto do display

// Estados do sistema
bool led_on = true;            // Estado dos LEDs RGB
bool buttonA_state = false;    // Estado do botão A
bool piscar_borda = false;     // Estado da borda do display

// Cores para os LEDs (formato GRB)
uint cores[3] = {0x00550000, 0x00005500, 0x55000000}; // Vermelho, Verde, Azul
int cor_led;                   // Cor atual selecionada

// Temporização LEDs RGB
absolute_time_t last_led_time;
int led_state=0;               // Estado atual do ciclo RGB

// Matriz de LEDs WS2812
PIO pio = pio0;                // Controlador PIO
uint sm = 0;                   // Máquina de estados
uint32_t led_buffer[NUM_PIXELS]; // Buffer para os LEDs
uint32_t matriz[5][5];         // Matriz 5x5 para mapeamento

// Navegação
int linhaEcoluna[2] = {0,0};   // Posição atual na matriz
int index=0;                   // Índice para seleção de cores

// Controle de música
bool music1=false;             // Flags para tocar melodias
bool music2=false;
bool music3=false;

/* ========== PROTÓTIPOS DE FUNÇÕES ========== */
void led_init();                      // Inicializa LEDs RGB
void button_init(int button);         // Inicializa botões
void display_init();                  // Inicializa display OLED
void iniciar_adc();                   // Inicializa ADC do joystick
void debounce(uint gpio, uint32_t events); // Tratamento de botões
void matrix_init();                   // Inicializa matriz de LEDs
void atualizar_leds();                // Atualiza matriz WS2812
void preencher_matriz(int linhaAtiva, int colunaAtiva, int cor_led); // Preenche matriz
void converter_Matriz_em_linha();     // Converte matriz para buffer linear
void buzzer_init();                   // Inicializa buzzer
void buzzer_play_note(int freq, int duration_ms); // Toca uma nota
void tocar_musica(int *melodia, int *duracoes, int tamanho); // Toca melodia

/* ========== FUNÇÃO PRINCIPAL ========== */
int main()
{
    // Inicializações
    stdio_init_all();           // SDK do Pico
    led_init();                 // LEDs RGB
    button_init(buttonA);       // Botão A
    button_init(buttonB);       // Botão B
    iniciar_adc();              // ADC para joystick
    display_init();             // Display OLED
    matrix_init();              // Matriz WS2812
    buzzer_init();              // Buzzer
    
    last_led_time = get_absolute_time(); // Inicia temporização
    
    bool cor = true;            // Controle de cor da borda
    sleep_ms(1000);             // Delay para estabilização

    // Calibração do joystick (posição central)
    adc_select_input(1);        // Seleciona eixo X
    uint16_t valor_centralX = adc_read(); // Lê valor central X
    
    adc_select_input(0);        // Seleciona eixo Y
    uint16_t valor_centralY = adc_read(); // Lê valor central Y

    // Padrão para desenho no display (8x8 pixels)
    uint8_t quadrado[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    cor_led = cores[0];         // Cor inicial (vermelho)

    // Definições das músicas
    int musica1[] = {NOTE_G4, NOTE_E4, NOTE_G4, NOTE_E4, NOTE_G4, NOTE_E4, NOTE_G4, NOTE_E4};
    int duracao_musica1[] = {150, 150, 150, 150, 150, 150, 150, 150};

    int musica2[] = {NOTE_E5, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_A4};
    int duracao_musica2[] = {125, 125, 125, 125, 125, 125, 125, 125};

    int musica3[] = {NOTE_A4, NOTE_D5, NOTE_B4, NOTE_A4, NOTE_D5, NOTE_E5};
    int duracao_musica3[] = {250, 250, 250, 250, 250, 250};

    // Calcula tamanho dos arrays de música
    int musica1_size = sizeof(musica1) / sizeof(int);
    int musica2_size = sizeof(musica2) / sizeof(int);
    int musica3_size = sizeof(musica3) / sizeof(int);

    // Configura interrupções para os botões
    gpio_set_irq_enabled_with_callback(buttonA, GPIO_IRQ_EDGE_FALL, true, &debounce);
    gpio_set_irq_enabled_with_callback(buttonB, GPIO_IRQ_EDGE_FALL, true, &debounce);

    // Loop principal
    while (true) {
        // Controle dos LEDs RGB (ciclo de cores)
        if (led_on==1){
            if (absolute_time_diff_us(last_led_time, get_absolute_time()) > 300000) { // 300ms
                // Desliga todos os LEDs
                gpio_put(led_red, false);
                gpio_put(led_blue, false);
                gpio_put(led_green, false);

                // Aciona o LED conforme o estado atual
                if (led_state == 0) {
                    gpio_put(led_blue, true);
                } else if (led_state == 1) {
                    gpio_put(led_red, true);
                } else if (led_state == 2) {
                    gpio_put(led_green, true);
                }

                // Avança para o próximo estado (0→1→2→0...)
                led_state = (led_state + 1) % 3;

                // Atualiza temporização
                last_led_time = get_absolute_time();
            }     
        }
        
        // Leitura do joystick
        adc_select_input(1);    // Seleciona eixo X
        uint16_t adcx_value = adc_read();
    
        adc_select_input(0);    // Seleciona eixo Y
        uint16_t adcy_value = period - adc_read(); // Inverte o valor para correção

        // Verifica movimento no joystick
        bool eixo_x = !buttonA_state && (adcx_value > valor_centralX + 250 || adcx_value < valor_centralX - 250);
        bool eixo_y = !buttonA_state && (adcy_value > valor_centralY + 250 || adcy_value < valor_centralY - 250);
        
        // Mapeia valores do joystick para posições
        // Display OLED
        int posicaoX = ((adcx_value * 125) / period) - 4;
        int posicaoY = ((adcy_value * 64) / period) - 4;

        // Matriz LED (mapeamento invertido para orientação física)
        int ledPosicaox =4- trunc(((adcy_value * 5) / period));
        int ledPosicaoy = trunc(((adcx_value * 5) / period));
        
        // Atualiza matriz LED
        preencher_matriz(ledPosicaox, ledPosicaoy, cor_led);
        atualizar_leds();

        // Limita posição do cursor no display
        if (posicaoX <= 0) posicaoX = 0 + 4;
        if (posicaoX >= 127 - 13) posicaoX = 127 - 13;
        if (posicaoY <= 0) posicaoY = 0 + 4;
        if (posicaoY >= 63 - 13) posicaoY = 63 - 13;

        // Atualiza display OLED
        cor = !cor; // Alterna cor da borda
        ssd1306_fill(&ssd, piscar_borda ? !cor : false); // Preenche fundo
        
        // Desenha borda
        ssd1306_rect(&ssd, 3, 3, 122, 58, piscar_borda ? cor : true, piscar_borda ? !cor : false);
        
        // Desenha cursor (quadrado)
        ssd1306_draw(&ssd, quadrado, posicaoX, posicaoY);
        ssd1306_send_data(&ssd); // Envia dados para o display
        
        // Toca músicas conforme solicitação
        if (music1){
            printf("Tocando musica 1\n");
            tocar_musica(musica1, duracao_musica1, musica1_size);
            music1=false;
        }
        if (music2){
            printf("Tocando musica 2\n");
            tocar_musica(musica2, duracao_musica2, musica2_size);
            music2=false;
        }
        if (music3){
            printf("Tocando musica 3\n");
            tocar_musica(musica3, duracao_musica3, musica3_size);
            music3=false;
        }
        
        sleep_ms(50); // Delay para estabilização
    }
}

/* ========== IMPLEMENTAÇÃO DAS FUNÇÕES ========== */

// Inicializa LEDs RGB
void led_init()
{
    gpio_init(led_red);
    gpio_init(led_blue);
    gpio_init(led_green);
    gpio_set_dir(led_red, GPIO_OUT);
    gpio_set_dir(led_blue, GPIO_OUT);
    gpio_set_dir(led_green, GPIO_OUT);
    gpio_put(led_red, false);
    gpio_put(led_blue, false);
    gpio_put(led_green, false);
}

// Inicializa botões com pull-up
void button_init(int button)
{
    gpio_init(button);
    gpio_set_dir(button, GPIO_IN);
    gpio_pull_up(button);
}

// Inicializa display OLED (I2C)
void display_init()
{
    i2c_init(I2C_PORT, 400 * 1000); // I2C a 400kHz
    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_SDA);
    gpio_pull_up(PIN_I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
}

// Inicializa ADC para leitura do joystick
void iniciar_adc()
{
    adc_init();
    adc_gpio_init(analogicox); // Configura como entrada analógica
    adc_gpio_init(analogicoy);
}

// Tratamento de interrupção dos botões (debounce)
void debounce(uint gpio, uint32_t events)
{
    static uint32_t last_time = 0;
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Debounce - ignora pulsos menores que 200ms
    if (current_time - last_time > 200000)
    {
        last_time = current_time;
        
        // Botão A - controla LEDs RGB
        if (gpio == buttonA){
            led_on = !led_on; // Alterna estado

            // Desliga todos os LEDs
            gpio_put(led_red, false);
            gpio_put(led_blue, false);
            gpio_put(led_green, false);     

            printf("Botão A pressionado\n");
        }
        
        // Botão B - controla cor da matriz e toca música
        if (gpio == buttonB){
            printf("Botão B pressionado\n");
            index++;
            
            // Ciclo entre as cores (0→1→2→0...)
            if (index ==0){
                printf("Alterando para a cor vermelho\n");
                cor_led = cores[0];
                music1=true; // Toca música 1
            }else if (index == 1){
                printf("Alterando para a cor azul\n");
                cor_led = cores[1];
                music2=true; // Toca música 2
            }else if (index == 2){
                cor_led = cores[2];
                printf("Alterando para a cor verde\n");
                music3=true; // Toca música 3
                index=-1; // Reinicia ciclo
            }
        }
    }
}

// Inicializa matriz de LEDs WS2812 via PIO
void matrix_init()
{
    // Configura PIO para controlar os LEDs
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, MATRIX_LED, 800000, false);
    pio_sm_set_enabled(pio, sm, true);
    sleep_ms(100); // Delay para estabilização
}

// Envia dados para a matriz de LEDs
void atualizar_leds(){
    for (int i = 0; i < 25; i++){
        pio_sm_put_blocking(pio, sm, led_buffer[i]);
    }   
}

/*
 * Mapeamento físico da matriz:
 * 24,23,22,21,20
 * 15,16,17,18,19
 * 14,13,12,11,10
 * 05,06,07,08,09
 * 04,03,02,01,00
 */

// Preenche matriz com a posição ativa
void preencher_matriz(int linhaAtiva, int colunaAtiva, int cor_led) {
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            // Ativa apenas o LED na posição atual
            matriz[y][x] = (y == linhaAtiva && x == colunaAtiva) ? cor_led : 0x00000000;
        }
    }
    converter_Matriz_em_linha(); // Converte para o formato linear
}

// Converte matriz 5x5 para buffer linear (com mapeamento serpentina)
void converter_Matriz_em_linha() {
    int countLeds = 0;
    for(int linha = 0; linha < 5 ; linha++){
        if(linha % 2 == 0) {
            // Linhas pares - ordem inversa
            for(int coluna = 4; coluna >= 0; coluna--){
                led_buffer[countLeds++] = matriz[linha][coluna];
            }
        } else {
            // Linhas ímpares - ordem direta
            for(int coluna = 0; coluna < 5; coluna++){
                led_buffer[countLeds++] = matriz[linha][coluna];
            }    
        }
    }
}

// Inicializa buzzer
void buzzer_init() {
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
}

// Toca uma nota musical
void buzzer_play_note(int freq, int duration_ms) {
    if (freq == REST) {
        gpio_put(BUZZER_PIN, 0);
        sleep_ms(duration_ms);
        return;
    }

    // Calcula período e número de ciclos
    uint32_t period_us = 1000000 / freq;
    uint32_t cycles = (freq * duration_ms) / 1000;

    // Gera onda quadrada
    for (uint32_t i = 0; i < cycles; i++) {
        gpio_put(BUZZER_PIN, 1);
        sleep_us(period_us / 2);
        gpio_put(BUZZER_PIN, 0);
        sleep_us(period_us / 2);
    }
}

// Toca uma melodia completa
void tocar_musica(int *melodia, int *duracoes, int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        buzzer_play_note(melodia[i], duracoes[i]);
        sleep_ms(20); // Pequena pausa entre notas
    }
}