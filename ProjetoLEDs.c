#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "ws2812.pio.h"
#include "numeros.h"

// Definição de constantes
#define A_BUTTON 5
#define B_BUTTON 6
#define LED_RED 13
#define WS2812_PIN 7
#define JOYSTICK_BUTTON 22

// Variável global para armazenar a cor (Entre 0 e 255 para intensidade)
uint8_t red_value = 0; // Intensidade do vermelho
uint8_t green_value = 20; // Intensidade do verde
uint8_t blue_value = 0; // Intensidade do azul

uint32_t last_time;
uint8_t current_frame = 0; // Índice que representa o número atual exibido no display

// Protótipo das Funções
void setup(void);
static void gpio_irq_handler(uint gpio, uint32_t events);
static inline void put_pixel(uint32_t pixel_grb);
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
void set_led(bool *frame, uint8_t r, uint8_t g, uint8_t b);

int main()
{
    setup(); // Configuração das portas digitais

    // Inicializando a Matriz de Led (WS2812)
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);

    // Habilitando interrupção da gpio nos botões A e B.
    gpio_set_irq_enabled_with_callback(A_BUTTON, GPIO_IRQ_EDGE_FALL, 1, & gpio_irq_handler);
    gpio_set_irq_enabled(B_BUTTON, GPIO_IRQ_EDGE_FALL, true);

    set_led(numeros[0], red_value, green_value, blue_value); // Inicializa o programa colocando o número 0 no display.

    while(true) {
        // LED Vermelho liga e desliga a cada 200ms, totalizando 5 vezes por segundo (1000ms)
        gpio_put(LED_RED, 1);
        sleep_ms(100);
        gpio_put(LED_RED, 0);
        sleep_ms(100);
    }

    return 0;
}

// Inicialização e configuração das portas digitais
void setup(void)
{
    stdio_init_all();

    gpio_init(LED_RED); // Inicializando LED vermelho.
    gpio_set_dir(LED_RED, GPIO_OUT);

    gpio_init(A_BUTTON); // Inicializando Botão A.
    gpio_set_dir(A_BUTTON, GPIO_IN);
    gpio_pull_up(A_BUTTON);

    gpio_init(B_BUTTON);    // Inicializando Botão B.
    gpio_set_dir(B_BUTTON, GPIO_IN);
    gpio_pull_up(B_BUTTON);
}

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void set_led(bool *frame, uint8_t r, uint8_t g, uint8_t b)
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < WS2812_LED_COUNT; i++)
    {
        if (frame[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0);  // Desliga os LEDs com zero no buffer
        }
    }
}

// Rotina da Interrupção
static void gpio_irq_handler(uint gpio, uint32_t events){

    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time > 200000) // Delay de 200ms para debounce
    {
        last_time = current_time;

        switch (gpio) {
        case 5:
            if (current_frame != 9) {   // Se o frame atual for 9, não é possível aumentar o número do contador no led
                current_frame++;        // Incrementa o índice do contador de frames e carrega ele no display com as cores predefinidas.
                blue_value += 2;
                green_value -= 2;
                set_led(numeros[current_frame], red_value, green_value, blue_value); 
            } 
            break;

        case 6:
            if (current_frame) {        // Se o frame atual for 0, não é possível diminuir o número do contador no led
                current_frame--;        // Decrementa o índice do contador de frames e carrega ele no display com as cores predefinidas.
                blue_value -= 2;
                green_value += 2;
                set_led(numeros[current_frame], red_value, green_value, blue_value);
            } 
            break;
        
        default:
            break;
        }
    }
}