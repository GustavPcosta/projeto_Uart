#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "inc/font.h"
#include "inc/ssd1306.h"
#include "ws2812.pio.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"


#define IS_RGBW false
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define UART_TX_PIN 0 // Alterado para pino GPIO 4 para TX (para UART1)
#define UART_RX_PIN 1 // Alterado para pino GPIO 5 para RX (para UART1)

#define RGB_RED_PIN 13  //cor vermelha
#define RGB_GREEN_PIN 11//cor verde
#define RGB_BLUE_PIN 12 // cor azul

const uint button_A = 5; //button A
const uint button_B = 6;// button B

#define UART_ID uart1 // Alterado para usar UART1
#define BAUD_RATE 115200 // Define a taxa de transmissão

int last_display_state = -1;
ssd1306_t ssd; //inicializa o display
bool led_verde_ativo = false; // Variável para controlar o LED no loop principal
uint32_t tempo_led_verde = 0;
int current_number = 0;
bool estadoVerde = false; //estado do button em verdadeiro ou falso;
const bool numbers[10][NUM_PIXELS] = {
    {1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},

    // Número 1
    {0, 0, 0, 1, 0,
     0, 0, 1, 0, 0,
     1, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     1, 1, 1, 1, 1},

    //Número 2
     {1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1},

    // Número 3
    {1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1},

    // // Número 4
    {1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     0, 0, 0, 0, 1},

    // Número 5
    {1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1},

    // Número 6
    {1, 1, 1, 1, 1,  
    1, 0, 0, 0, 0, 
    1, 1, 1, 1, 1,  
    1, 0, 0, 0, 1,  
    1, 1, 1, 1, 1},

    // Número 7
    {1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    0, 0, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 1, 0, 0, 0},


    // Número 8
    {1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},

    // Número 9
    {1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1}

};

static void gpio_irq_handler(uint gpio, uint32_t events);
static inline void put_pixel(uint32_t pixel_grb);
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
void display_number(ssd1306_t *ssd, int number);

uint32_t last_press_A = 0;
uint32_t last_press_B = 0;

int main() {
    stdio_init_all();
    sleep_ms(2000);

    
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);  // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);  // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line

    ssd1306_t ssd; // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Configuração dos botões
    gpio_init(button_A);
    gpio_set_dir(button_A, GPIO_IN);
    gpio_pull_up(button_A);
    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // interrupção

    gpio_init(button_B);
    gpio_set_dir(button_B, GPIO_IN);
    gpio_pull_up(button_B);
    gpio_set_irq_enabled_with_callback(button_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    gpio_init(RGB_RED_PIN);
    gpio_set_dir(RGB_RED_PIN, GPIO_OUT);

    gpio_init(RGB_GREEN_PIN); // inicializa o led verde
    gpio_set_dir(RGB_GREEN_PIN, GPIO_OUT); // define a saída do led

    gpio_init(RGB_BLUE_PIN); // inicializa o led azul
    gpio_set_dir(RGB_BLUE_PIN, GPIO_OUT); //define a saída do led

    char buffer[100];  // Buffer para armazenar a mensagem
    int index = 0;     // Índice para armazenar caracteres no buffer

   int current_number = 0;  // Variável para armazenar o número selecionado

while (true) {
    display_number(&ssd, current_number); // Passa a referência para o display ao exibir o número
    sleep_ms(500);
    int display_state = 0;

    // Verifica se o USB está conectado
    if (stdio_usb_connected()) {
        char c;
        if (scanf("%c", &c) == 1) {  // Lê caractere da entrada padrão
            printf("Recebido: '%c'\n", c);

            switch (c) {
                case 'r':  // Se o comando for 'r', alterna o LED vermelho
                    gpio_put(RGB_RED_PIN, !gpio_get(RGB_RED_PIN));
                    printf("LED vermelho alternado!\n");
                    break;
                case 'g':  // Se o comando for 'g', alterna o LED verde
                    gpio_put(RGB_GREEN_PIN, !gpio_get(RGB_GREEN_PIN));
                    printf("LED verde alternado!\n");
                    break;
                case 'b':  // Se o comando for 'b', alterna o LED azul
                    gpio_put(RGB_BLUE_PIN, !gpio_get(RGB_BLUE_PIN));
                    printf("LED azul alternado!\n");
                    break;
                case '1':  // Se o comando for '1', define o número como 1
                    current_number = 1;
                    printf("Número selecionado: 1\n");
                    break;
                case '2':  // Se o comando for '2', define o número como 2
                    current_number = 2;
                    printf("Número selecionado: 2\n");
                    break;
                case '3':  // Se o comando for '3', define o número como 3
                    current_number = 3;
                    printf("Número selecionado: 3\n");
                    break;

                    case '4':  
                    current_number = 4;
                    printf("Número selecionado: 4\n");
                    break;

                     case '5':  
                    current_number = 5;
                    printf("Número selecionado: 5\n");
                    break;

                    case '6':  
                    current_number = 6;
                    printf("Número selecionado: 6\n");
                    break;
                    case '7':  
                    current_number = 7;
                    printf("Número selecionado: 7\n");
                    break;
                    case '8':  
                    current_number = 9;
                    printf("Número selecionado: 8\n");
                    break;
                    case '9':  
                    current_number = 9;
                    printf("Número selecionado: 9\n");
                    break;
                default:
                    printf("Comando inválido: '%c'\n", c);
            }
        }
    }

    // Verifica os botões primeiro e define o estado do display antes de atualizar
    if (gpio_get(button_A) && gpio_get(button_B)) {
        gpio_put(RGB_GREEN_PIN, 0);
        gpio_put(RGB_BLUE_PIN, 0);
        ssd1306_fill(&ssd, false);
        ssd1306_draw_string(&ssd, "led verde e azul apagados", 10, 20);
        display_state = 1;
    } else if (gpio_get(button_A) == 0) {
        gpio_put(RGB_GREEN_PIN, 1);
        gpio_put(RGB_BLUE_PIN, 0);
        ssd1306_fill(&ssd, false);
        ssd1306_draw_string(&ssd, "led verde aceso", 10, 20);
        display_state = 2;
    } else if (gpio_get(button_B) == 0) {
        gpio_put(RGB_GREEN_PIN, 0);
        gpio_put(RGB_BLUE_PIN, 1);
        ssd1306_fill(&ssd, false);
        ssd1306_draw_string(&ssd, "led azul aceso", 10, 20);
        display_state = 3;
    } else {
        // Se nenhum botão foi pressionado, apenas exibe o número na matriz de LEDs
        display_number(&ssd, current_number);
        display_state = 4;
    }

    // Atualiza o display apenas se o estado mudou
    if (display_state != last_display_state) {
        ssd1306_send_data(&ssd);
        last_display_state = display_state;
    }
    sleep_ms(500);
}

return 0;
}

void display_number(ssd1306_t *ssd, int number) {
    char str[2];  // String para armazenar o número
    snprintf(str, sizeof(str), "%d", number);  // Converte o número para string

    // Limpa a tela e exibe o número
    ssd1306_fill(ssd, false);
    ssd1306_draw_string(ssd, str, 10, 20);
    ssd1306_send_data(ssd);

    // Atualiza a matriz de LEDs com o número
    for (int i = 0; i < NUM_PIXELS; i++) {
        if (numbers[number][i]) {
            put_pixel(urgb_u32(255, 255, 255)); // Acende o LED (branco)
        } else {
            put_pixel(urgb_u32(0, 0, 0)); // Apaga o LED
        }
    }
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    // Funções relacionadas ao incremento e decremento e o debounce
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (gpio == button_A && (current_time - last_press_A) > 200) {
        last_press_A = current_time;
        current_number = (current_number + 1) % 10;
        printf("Incrementando para: %d\n", current_number);
    } else if (gpio == button_B && (current_time - last_press_B) > 200) {
        last_press_B = current_time;
        current_number = (current_number - 1 + 10) % 10;
        printf("Decrementando para: %d\n", current_number);
    }
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}
