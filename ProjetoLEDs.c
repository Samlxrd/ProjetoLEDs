#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "ws2812.pio.h"

// Definição de constantes
#define A_BUTTON 5
#define B_BUTTON 6
#define LED_RED 13
#define WS2812_PIN 7
#define JOYSTICK_BUTTON 22

uint32_t last_time;

// Protótipo das Funções
void setup(void);
static void gpio_irq_handler(uint gpio, uint32_t events);

int main()
{
    setup();

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);


    // Habilitando interrupção da gpio no botão do joystick,
    // Auxiliando o desenvolvimento e testes
    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON, GPIO_IRQ_EDGE_FALL, 1, & gpio_irq_handler);

    while(true) {
        if(!gpio_get(A_BUTTON)) {
            gpio_put(LED_RED, 1);
        } else {
            gpio_put(LED_RED, 0);
        }
    }

    return 0;
}

// Inicialização e configuração das portas digitais
void setup(void) {
    stdio_init_all();

    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);

    gpio_init(A_BUTTON);
    gpio_set_dir(A_BUTTON, GPIO_IN);
    gpio_pull_up(A_BUTTON);

    gpio_init(B_BUTTON);
    gpio_set_dir(B_BUTTON, GPIO_IN);
    gpio_pull_up(B_BUTTON);

    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON);
    
}

// Rotina da Interrupção
static void gpio_irq_handler(uint gpio, uint32_t events){

    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time > 40000) // Delay de 40ms para debounce
    {
        last_time = current_time;

        switch (gpio) {

        case 22:
            printf("HABILITANDO O MODO GRAVAÇÃO");
            reset_usb_boot(0,0); // Habilita o modo de gravação do microcontrolador
            break;
        
        default:
            break;
        }
    }
}