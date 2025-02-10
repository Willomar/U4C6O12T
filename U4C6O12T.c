#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "inc/pio_matrix.pio"
#include "inc/ssd1306.h"

// Definições de constantes e pinos
#define TEMPO_DEBOUNCE 200000  // Tempo de debounce dos botões
#define NUM_LEDS 25            // Número de LEDs na matriz
#define PINO_SAIDA 7           // Pino de saída para controle dos LEDs
#define BOTAO_A 5              // Pino do botão A
#define BOTAO_B 6              // Pino do botão B
#define BOTAO_STICK 22         // Pino do botão Stick (botão central)
#define LED_VERMELHO 13        // Pino do LED vermelho
#define LED_VERDE 11           // Pino do LED verde
#define LED_AZUL 12            // Pino do LED azul
#define BUZZER 21              // Pino do buzzer
#define PORTA_I2C i2c1        // Porta I2C para comunicação com o display
#define PINO_SDA 14            // Pino SDA para I2C
#define PINO_SCL 15            // Pino SCL para I2C
#define ENDERECO_DISPLAY 0x3C  // Endereço I2C do display SSD1306

// Variáveis globais para controle
uint32_t ultimo_tempo = 0;
bool led_verde_ligado = false;
uint32_t valor_led;
double r = 0.0, g = 0.0, b = 0.0;
PIO pio = pio0;
uint offset;
uint sm;
ssd1306_t display;
bool cor_fundo = true;  // Variável para definir a cor de fundo do display

// Definindo os padrões de LEDs para números de 0 a 9 (com base em um display 5x5)
double leds_apagados[NUM_LEDS] = {0};
double numero_0[NUM_LEDS] = {0.0,0.35,0.35,0.35,0.0,0.35,0.0,0.0,0.0,0.35,0.35,0.0,0.0,0.0,0.35,0.35,0.0,0.0,0.0,0.35,0.0,0.35,0.35,0.35,0.0};
// [Outros números omitidos para brevidade...]

// Função para gerar a cor RGB
uint32_t gerar_cor_rgb(double r, double g, double b) {
    unsigned char R = r * 255;
    unsigned char G = g * 255;
    unsigned char B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);  // Retorna a cor RGB codificada
}

// Função para ligar os LEDs na matriz com uma cor especificada
void ligar_matriz(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b) {
    for (int16_t i = 0; i < NUM_LEDS; i++) {
        valor_led = gerar_cor_rgb(desenho[24 - i], 0.0, 0.0);  // Atualiza a cor para o LED correspondente
        pio_sm_put_blocking(pio, sm, valor_led);  // Envia o valor de cor para o PIO
    }
}

// Função para inicializar o PIO da matriz de LEDs
void inicializar_pio_matriz() {
    offset = pio_add_program(pio, &pio_matrix_program);  // Adiciona o programa do PIO
    sm = pio_claim_unused_sm(pio, true);  // Reclama um state machine disponível
    pio_matrix_program_init(pio, sm, offset, PINO_SAIDA);  // Inicializa o programa do PIO
}

// Função para configurar os pinos GPIO (botões e LEDs)
void inicializar_gpio() {
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);  // Ativa o pull-up para evitar leituras erradas
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);  // Ativa o pull-up
    gpio_init(BOTAO_STICK);
    gpio_set_dir(BOTAO_STICK, GPIO_IN);
    gpio_pull_up(BOTAO_STICK);  // Ativa o pull-up
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);  // Configura o LED vermelho como saída
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);  // Configura o LED verde como saída
    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);  // Configura o LED azul como saída
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);  // Configura o buzzer como saída
}

// Função para configurar o barramento I2C e inicializar o display SSD1306
void inicializar_i2c() {
    i2c_init(PORTA_I2C, 400 * 1000);  // Inicializa o I2C com 400kHz de velocidade
    gpio_set_function(PINO_SDA, GPIO_FUNC_I2C);  // Configura o pino SDA para I2C
    gpio_set_function(PINO_SCL, GPIO_FUNC_I2C);  // Configura o pino SCL para I2C
    gpio_pull_up(PINO_SDA);  // Habilita o pull-up no SDA
    gpio_pull_up(PINO_SCL);  // Habilita o pull-up no SCL
    ssd1306_init(&display, 128, 64, false, ENDERECO_DISPLAY, PORTA_I2C);  // Inicializa o display 128x64
    ssd1306_config(&display);  // Configura o display
    ssd1306_send_data(&display);  // Envia os dados de configuração para o display
    ssd1306_fill(&display, true);  // Preenche a tela com a cor de fundo
    ssd1306_send_data(&display);  // Atualiza o display
}

// Função para desenhar o caractere no display
void desenhar_caractere(char caractere) {
    ssd1306_fill(&display, cor_fundo);  // Limpa o display com a cor de fundo
    ssd1306_rect(&display, 3, 3, 122, 58, !cor_fundo, cor_fundo);  // Desenha um retângulo na tela
    ssd1306_draw_string(&display, "CARACTERE DIGITADO:", 10, 10);  // Exibe o título
    ssd1306_draw_string(&display, &caractere, 63, 30);  // Exibe o caractere digitado
    ssd1306_draw_string(&display, "PRESSIONE UM BOTAO", 10, 48);  // Exibe a mensagem para pressionar um botão
    ssd1306_send_data(&display);  // Atualiza o display com as informações desenhadas
}

// Função para tratar o estado dos botões e acionar LEDs correspondentes
void tratar_botao(bool estado_led, bool botao_verde) {
    if (botao_verde) {
        gpio_put(LED_AZUL, false);  // Desliga o LED azul
        gpio_put(LED_VERDE, !gpio_get(LED_VERDE));  // Alterna o estado do LED verde
        if (estado_led) {
            ssd1306_fill(&display, cor_fundo);  // Limpa o display
            ssd1306_rect(&display, 3, 3, 122, 58, !cor_fundo, cor_fundo);  // Desenha o retângulo
            ssd1306_draw_string(&display, "BOTAO A PRESSIONADO", 10, 10);  // Mensagem no display
            ssd1306_draw_string(&display, "LED VERDE LIGADO", 10, 30);  // Mensagem sobre o LED verde
            ssd1306_send_data(&display);  // Atualiza o display
            printf("Botao A pressionado: LED Verde ligado\n");
        } else {
            ssd1306_fill(&display, cor_fundo);  // Limpa o display
            ssd1306_rect(&display, 3, 3, 122, 58, !cor_fundo, cor_fundo);  // Desenha o retângulo
            ssd1306_draw_string(&display, "BOTAO A PRESSIONADO", 10, 10);  // Mensagem no display
            ssd1306_draw_string(&display, "LED VERDE DESLIGADO", 10, 30);  // Mensagem sobre o LED verde
            ssd1306_send_data(&display);  // Atualiza o display
            printf("Botao A pressionado: LED Verde desligado\n");
        }
    } else {
        gpio_put(LED_VERDE, false);  // Desliga o LED verde
        gpio_put(LED_AZUL, !gpio_get(LED_AZUL));  // Alterna o estado do LED azul
        if (estado_led) {
            ssd1306_fill(&display, cor_fundo);  // Limpa o display
            ssd1306_rect(&display, 3, 3, 122, 58, !cor_fundo, cor_fundo);  // Desenha o retângulo
            ssd1306_draw_string(&display, "BOTAO B PRESSIONADO", 10, 10);  // Mensagem no display
            ssd1306_draw_string(&display, "LED AZUL LIGADO", 10, 30);  // Mensagem sobre o LED azul
            ssd1306_send_data(&display);  // Atualiza o display
            printf("Botao B pressionado: LED Azul ligado\n");
        } else {
            ssd1306_fill(&display, cor_fundo);  // Limpa o display
            ssd1306_rect(&display, 3, 3, 122, 58, !cor_fundo, cor_fundo);  // Desenha o retângulo
            ssd1306_draw_string(&display, "BOTAO B PRESSIONADO", 10, 10);  // Mensagem no display
            ssd1306_draw_string(&display, "LED AZUL DESLIGADO", 10, 30);  // Mensagem sobre o LED azul
            ssd1306_send_data(&display);  // Atualiza o display
            printf("Botao B pressionado: LED Azul desligado\n");
        }
    }
}

// Função de callback para tratar eventos de botões
void callback_botoes(uint gpio, uint32_t eventos) {
    uint32_t tempo_atual = to_us_since_boot(get_absolute_time());  // Captura o tempo atual
    if (tempo_atual -
