#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "inc/pio_matrix.pio.h"
#include "inc/ssd1306.h"

#define TEMPO_DEBOUNCE 200000
#define NUM_LEDS 25
#define PINO_SAIDA 7
#define BOTAO_A 5
#define BOTAO_B 6
#define BOTAO_STICK 22
#define LED_VERMELHO 13
#define LED_VERDE 11
#define LED_AZUL 12
#define BUZZER 21
#define PORTA_I2C i2c1
#define PINO_SDA 14
#define PINO_SCL 15
#define ENDERECO_DISPLAY 0x3C

uint32_t ultimo_tempo = 0;
bool led_verde_ligado = false;
uint32_t valor_led;
double r = 0.0, g = 0.0, b = 0.0;
PIO pio = pio0;
uint offset;
uint sm;
ssd1306_t display;
bool cor_fundo = true;

double leds_apagados[NUM_LEDS] = {0};
double numero_0[NUM_LEDS] = {0.0,0.35,0.35,0.35,0.0,0.35,0.0,0.0,0.0,0.35,0.35,0.0,0.0,0.0,0.35,0.35,0.0,0.0,0.0,0.35,0.0,0.35,0.35,0.35,0.0};
double numero_1[NUM_LEDS] = {0.0,0.0,0.2,0.0,0.0,0.0,0.0,0.2,0.2,0.0,0.2,0.0,0.2,0.0,0.0,0.0,0.0,0.2,0.0,0.0,0.0,0.2,0.2,0.2,0.0};
double numero_2[NUM_LEDS] = {0.2,0.2,0.2,0.2,0.2,0.2,0.0,0.0,0.0,0.0,0.2,0.2,0.2,0.2,0.2,0.0,0.0,0.0,0.0,0.2,0.2,0.2,0.2,0.2,0.2};
double numero_3[NUM_LEDS] = {0.0,0.35,0.35,0.35,0.35,0.35,0.0,0.0,0.0,0.0,0.0,0.0,0.35,0.35,0.35,0.35,0.0,0.0,0.0,0.0,0.0,0.35,0.35,0.35,0.35};
double numero_4[NUM_LEDS] = {0.0,0.35,0.0,0.0,0.35,0.35,0.0,0.0,0.35,0.0,0.0,0.35,0.35,0.35,0.35,0.35,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.35};
double numero_5[NUM_LEDS] = {0.35,0.35,0.35,0.35,0.35,0.0,0.0,0.0,0.0,0.35,0.35,0.35,0.35,0.35,0.35,0.35,0.0,0.0,0.0,0.0,0.35,0.35,0.35,0.35,0.35};
double numero_6[NUM_LEDS] = {0.35,0.35,0.35,0.35,0.0,0.0,0.0,0.0,0.0,0.35,0.35,0.35,0.35,0.35,0.0,0.35,0.0,0.0,0.0,0.35,0.35,0.35,0.35,0.35,0.35};
double numero_7[NUM_LEDS] = {0.35,0.35,0.35,0.35,0.35,0.35,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.35,0.35,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.35};
double numero_8[NUM_LEDS] = {0.0,0.35,0.35,0.35,0.0,0.35,0.0,0.0,0.0,0.35,0.35,0.35,0.35,0.35,0.35,0.35,0.0,0.0,0.0,0.35,0.0,0.35,0.35,0.35,0.0};
double numero_9[NUM_LEDS] = {0.35,0.35,0.35,0.35,0.35,0.35,0.0,0.0,0.0,0.35,0.35,0.35,0.35,0.35,0.35,0.35,0.0,0.0,0.0,0.0,0.0,0.35,0.35,0.35,0.35};

uint32_t gerar_cor_rgb(double r, double g, double b) {
    unsigned char R = r * 255;
    unsigned char G = g * 255;
    unsigned char B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

void ligar_matriz(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b) {
    for (int16_t i = 0; i < NUM_LEDS; i++) {
        valor_led = gerar_cor_rgb(desenho[24 - i], 0.0, 0.0);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

void inicializar_pio_matriz() {
    offset = pio_add_program(pio, &pio_matrix_program);
    sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, PINO_SAIDA);
}

void inicializar_gpio() {
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);
    gpio_init(BOTAO_STICK);
    gpio_set_dir(BOTAO_STICK, GPIO_IN);
    gpio_pull_up(BOTAO_STICK);
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
}

void inicializar_i2c() {
    i2c_init(PORTA_I2C, 400 * 1000);
    gpio_set_function(PINO_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PINO_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PINO_SDA);
    gpio_pull_up(PINO_SCL);
    ssd1306_init(&display, 128, 64, false, ENDERECO_DISPLAY, PORTA_I2C);
    ssd1306_config(&display);
    ssd1306_send_data(&display);
    ssd1306_fill(&display, true);
    ssd1306_send_data(&display);
}

void desenhar_caractere(char caractere) {
    ssd1306_fill(&display, cor_fundo);
    ssd1306_rect(&display, 3, 3, 122, 58, !cor_fundo, cor_fundo);
    ssd1306_draw_string(&display, "CARACTERE DIGITADO:", 10, 10);
    ssd1306_draw_string(&display, &caractere, 63, 30);
    ssd1306_draw_string(&display, "PRESSIONE UM BOTAO", 10, 48);
    ssd1306_send_data(&display);
}

void tratar_botao(bool estado_led, bool botao_verde) {
    if (botao_verde) {
        gpio_put(LED_AZUL, false);
        gpio_put(LED_VERDE, !gpio_get(LED_VERDE));
        if (estado_led) {
            ssd1306_fill(&display, cor_fundo);
            ssd1306_rect(&display, 3, 3, 122, 58, !cor_fundo, cor_fundo);
            ssd1306_draw_string(&display, "BOTAO A PRESSIONADO", 10, 10);
            ssd1306_draw_string(&display, "LED VERDE LIGADO", 10, 30);
            ssd1306_send_data(&display);
            printf("Botao A pressionado: LED Verde ligado\n");
        } else {
            ssd1306_fill(&display, cor_fundo);
            ssd1306_rect(&display, 3, 3, 122, 58, !cor_fundo, cor_fundo);
            ssd1306_draw_string(&display, "BOTAO A PRESSIONADO", 10, 10);
            ssd1306_draw_string(&display, "LED VERDE DESLIGADO", 10, 30);
            ssd1306_send_data(&display);
            printf("Botao A pressionado: LED Verde desligado\n");
        }
    } else {
        gpio_put(LED_VERDE, false);
        gpio_put(LED_AZUL, !gpio_get(LED_AZUL));
        if (estado_led) {
            ssd1306_fill(&display, cor_fundo);
            ssd1306_rect(&display, 3, 3, 122, 58, !cor_fundo, cor_fundo);
            ssd1306_draw_string(&display, "BOTAO B PRESSIONADO", 10, 10);
            ssd1306_draw_string(&display, "LED AZUL LIGADO", 10, 30);
            ssd1306_send_data(&display);
            printf("Botao B pressionado: LED Azul ligado\n");
        } else {
            ssd1306_fill(&display, cor_fundo);
            ssd1306_rect(&display, 3, 3, 122, 58, !cor_fundo, cor_fundo);
            ssd1306_draw_string(&display, "BOTAO B PRESSIONADO", 10, 10);
            ssd1306_draw_string(&display, "LED AZUL DESLIGADO", 10, 30);
            ssd1306_send_data(&display);
            printf("Botao B pressionado: LED Azul desligado\n");
        }
    }
}

void callback_botoes(uint gpio, uint32_t eventos) {
    uint32_t tempo_atual = to_us_since_boot(get_absolute_time());
    if (tempo_atual - ultimo_tempo > TEMPO_DEBOUNCE) {
        if (gpio == BOTAO_A) {
            tratar_botao(!gpio_get(LED_VERDE), true);
        } else if (gpio == BOTAO_B) {
            tratar_botao(!gpio_get(LED_AZUL), false);
        } else if (gpio == BOTAO_STICK) {
            reset_usb_boot(0, 0);
        }
        ultimo_tempo = tempo_atual;
    }
}

int main() {
    stdio_init_all();
    inicializar_gpio();
    inicializar_pio_matriz();
    inicializar_i2c();

    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, callback_botoes);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, callback_botoes);
    gpio_set_irq_enabled_with_callback(BOTAO_STICK, GPIO_IRQ_EDGE_FALL, true, callback_botoes);

    while (true) {
        if (stdio_usb_connected()) {
            char caractere;
            if (scanf("%c", &caractere) == 1) {
                printf("Caractere recebido: %c\n", caractere);
                switch (caractere) {
                    case '0': ligar_matriz(numero_0, valor_led, pio, sm, r, g, b); desenhar_caractere(caractere); break;
                    case '1': ligar_matriz(numero_1, valor_led, pio, sm, r, g, b); desenhar_caractere(caractere); break;
                    case '2': ligar_matriz(numero_2, valor_led, pio, sm, r, g, b); desenhar_caractere(caractere); break;
                    case '3': ligar_matriz(numero_3, valor_led, pio, sm, r, g, b); desenhar_caractere(caractere); break;
                    case '4': ligar_matriz(numero_4, valor_led, pio, sm, r, g, b); desenhar_caractere(caractere); break;
                    case '5': ligar_matriz(numero_5, valor_led, pio, sm, r, g, b); desenhar_caractere(caractere); break;
                    case '6': ligar_matriz(numero_6, valor_led, pio, sm, r, g, b); desenhar_caractere(caractere); break;
                    case '7': ligar_matriz(numero_7, valor_led, pio, sm, r, g, b); desenhar_caractere(caractere); break;
                    case '8': ligar_matriz(numero_8, valor_led, pio, sm, r, g, b); desenhar_caractere(caractere); break;
                    case '9': ligar_matriz(numero_9, valor_led, pio, sm, r, g, b); desenhar_caractere(caractere); break;
                    default: desenhar_caractere(caractere); ligar_matriz(leds_apagados, valor_led, pio, sm, r, g, b);
                }
            }
        }
        sleep_ms(10);
    }
}