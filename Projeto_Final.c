#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2818b.pio.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"
#include <ctype.h>
#include <time.h>

// ######################################### Definições e declarações de variaveis das funções do microcontrolador ##############

#define LED_COUNT 25
#define MATRIZ_PIN 7
#define BUZZER_PIN 21
#define BUZZER_FREQUENCY 100
#define BUTTON_A 5
#define BUTTON_B 6
#define DEBOUNCE_DELAY_US 20000

const uint8_t I2C_SDA = 14;
const uint8_t I2C_SCL = 15;
const uint8_t vRx = 27;
const uint8_t vRy = 26;
const int adc_channel_vrx = 1;
const int adc_channel_vry = 0;
// ####################################### Declarações das variaveis globais ##############################

char operacao[10] = {}; // Salva a String da operação
int resultado = 0;      // Salva o resultado da operação
int volatile numero_analogico = 0;

struct repeating_timer timer_joystick; // Timer para interrupçao

char letra_escolhida;
uint8_t id_palavra_escolhida, id_letra_faltando;
uint8_t limite = 0;

volatile bool button_pressed = false;
volatile bool button_pressed_2 = false;
volatile uint32_t last_button_time = 0;
volatile uint32_t last_button_time_2 = 0;
uint8_t escolher_game = 0;

ssd1306_t disp;
// ######################################### Funções ####################################################

void npInit(uint pin);                                                              // Inicializa a matriz de LED.
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b); // Atribui uma cor RGB a um LED.
void npClear();                                                                     // Limpa a matriz de LED.
void npWrite();                                                                     // Escreve a matriz de LED.
int getIndex(int x, int y);                                                         // Retorna o índice do LED na matriz de LED.
void preencherBuffer(int matriz[5][5][3]);                                          // Preenche o buffer de pixels com a matriz de LED.
void transformar_vermelho_verde(int numero[5][5][3]);                               // Transforma o numero vermelho em verde.
void gerar_soma_subtracao_aleatorio();                                              // Gera um numero aleatorio e a operação de soma ou subtração.
void exibir_texto(ssd1306_t *disp, char *words[], size_t word_count, uint x, uint y, uint scale, uint delay_ms);
bool ler_joystick_x(struct repeating_timer *t); // Le o valor do Joystick e retorna o valor de 0 a 9.
bool ler_joystick_y(struct repeating_timer *t);
void setup_joystick();     // Configura o Joystick.
void jogo_soma_completo(); // Função que executa o jogo completo.
void pwm_init_buzzer(uint pin);
void play_tone(uint pin, uint frequency, uint duration_ms);
void jogo_palavra_completo();
int escolher_dificuldade();
void escolher_jogos();
void exibir_texto_sem_apagar(ssd1306_t *disp, char *words[], size_t word_count, uint x, uint y, uint scale, uint delay_ms);
void button_isr(uint gpio, uint32_t events);
void jogar();
void transformar_verde_vermelho(int numero[5][5][3]);
void desenhar_moldura();
void button_isr2(uint gpio, uint32_t events);
void pegar_palavra_aleatoria();
// ######################################### Definição da matriz de Led ##############################################
struct pixel_t
{
  uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;
// ########################################## Declaração dos Numeros de 1 - 9 ########################################

int numero_0[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int numero_1[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int numero_2[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int numero_3[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int numero_4[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int numero_5[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int numero_6[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int numero_7[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int numero_8[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int numero_9[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}}};

int (*numeros[10])[5][5][3] = {&numero_0, &numero_1, &numero_2, &numero_3, &numero_4, &numero_5, &numero_6, &numero_7, &numero_8, &numero_9};

// ######################################### Jogo das Palvras ################################################
char banco_de_palavras[16][8] = {
    "PU_A", "_VA", "_OLA", "GEL_", "SU_O", "_RSO", "_U_A", "Z_BRA",
    "C_LA", "_AI", "MOCH_LA", "_IO", "_ILHO", "_IVRO", "BR_NCAR", "PAP_L"};

char letra_completa[16] = {'L', 'U', 'G', 'O', 'C', 'U', 'L', 'E', 'O', 'P', 'I', 'T', 'F', 'L', 'I', 'E'};

int letra_p[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}};
int letra_u[5][5][3] = {
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int letra_l[5][5][3] = {
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int letra_c[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int letra_g[5][5][3] = {
    {{80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int letra_h[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int letra_i[5][5][3] = {
    {{0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}}};
int letra_t[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}}};
int letra_o[5][5][3] = {
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}};
int letra_e[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}}};
int letra_f[5][5][3] = {
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {80, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}};

int (*letras[11])[5][5][3] = {&letra_p, &letra_u, &letra_l, &letra_c, &letra_g, &letra_h, &letra_i, &letra_t, &letra_o, &letra_e, &letra_f};

struct LetraID
{
  char letter; // Letra associada
  int id;      // ID associado à letra
};
struct LetraID letters[11] = {
    {'P', 0},
    {'U', 1},
    {'L', 2},
    {'C', 3},
    {'G', 4},
    {'H', 5},
    {'I', 6},
    {'T', 7},
    {'O', 8},
    {'E', 9},
    {'F', 10}};
// ########################################## Limpa Buffer #######################################################
int errou[5][5][3] = {
    {{150, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {150, 0, 0}},
    {{0, 0, 0}, {150, 0, 0}, {0, 0, 0}, {150, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {150, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {150, 0, 0}, {0, 0, 0}, {150, 0, 0}, {0, 0, 0}},
    {{150, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {150, 0, 0}}};

int acertou[5][5][3] = {
    {{0, 0, 0}, {0, 0, 100}, {0, 0, 0}, {0, 0, 100}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 100, 0}, {0, 0, 0}, {100, 0, 0}, {0, 0, 0}, {0, 100, 0}},
    {{0, 100, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 100, 0}},
    {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}}};

int desligar[5][5][3] = {
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}};

int (*outras[3])[5][5][3] = {&errou, &acertou, &desligar};
// ########################################## Função Principal ####################################################
int main()
{
  stdio_init_all(); // Inicializa os tipos stdio padrão presentes ligados ao binário
  srand(time(NULL));
  setup_gpios();
  pwm_init_buzzer(BUZZER_PIN);

  disp.external_vcc = false;
  ssd1306_init(&disp, 128, 64, 0x3C, i2c1);
  ssd1306_clear(&disp);

  gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_isr);
  // Processo de inicialização completo do OLED SSD1306
  npInit(MATRIZ_PIN); // Inicializa a matriz de LED.
  npClear();
  setup_joystick(); // Configura o Joystick.


  jogar();

  return 0;
}

// ################################################## Funções referentes a matriz de Led ####################################################

int getIndex(int x, int y)
{
  // Se a linha for par (0, 2, 4), percorremos da esquerda para a direita.
  // Se a linha for ímpar (1, 3), percorremos da direita para a esquerda.
  if (y % 2 == 0)
  {
    return 24 - (y * 5 + x); // Linha par (esquerda para direita).
  }
  else
  {
    return 24 - (y * 5 + (4 - x)); // Linha ímpar (direita para esquerda).
  }
}
void preencherBuffer(int matriz[5][5][3])
{
  for (int linha = 0; linha < 5; linha++)
  {
    for (int coluna = 0; coluna < 5; coluna++)
    {
      int posicao = getIndex(linha, coluna);
      npSetLED(posicao, matriz[coluna][linha][0], matriz[coluna][linha][1], matriz[coluna][linha][2]);
    }
  }
}
void npInit(uint pin)
{

  // Cria programa PIO.
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;

  // Toma posse de uma máquina PIO.
  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0)
  {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
  }

  // Inicia programa na máquina PIO obtida.
  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

  // Limpa buffer de pixels.
  for (uint i = 0; i < LED_COUNT; ++i)
  {
    leds[i].R = 0;
    leds[i].G = 0;
    leds[i].B = 0;
  }
}
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b)
{
  leds[index].R = r;
  leds[index].G = g;
  leds[index].B = b;
}
void npClear()
{
  for (uint i = 0; i < LED_COUNT; ++i)
    npSetLED(i, 0, 0, 0);
}
void npWrite()
{

  for (uint i = 0; i < LED_COUNT; ++i)
  {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100);
}
void transformar_vermelho_verde(int numero[5][5][3])
{
  for (int i = 0; i < 5; i++)
  { // Corrigido de 10 para 5
    for (int j = 0; j < 5; j++)
    {
      if (numero[i][j][0] == 80)
      {                       // Verifica se o vermelho é 100
        numero[i][j][0] = 0;  // Zera o vermelho
        numero[i][j][1] = 80; // Define o verde como 100
      }
    }
  }
}

void transformar_verde_vermelho(int numero[5][5][3])
{
  for (int i = 0; i < 5; i++)
  { // Corrigido de 10 para 5
    for (int j = 0; j < 5; j++)
    {
      if (numero[i][j][1] == 80)
      {                       // Verifica se o vermelho é 100
        numero[i][j][0] = 80; // Zera o vermelho
        numero[i][j][1] = 0;  // Define o verde como 100
      }
    }
  }
}

// ##################################################### Funções que gera o jogo da soma ###############################################

void gerar_soma_subtracao_aleatorio()
{

  // Inicializa o gerador de números aleatórios.
  int numero1_op = rand() % 6;         // Gera um número aleatório entre 0 a 5.
  int numero2_op = rand() % 5;         // Gera um número aleatório entre 0 a 4.
  int num_operacao = (rand() % 2) + 1; // Gera o número 1 ou 2, 1 - para soma, 2 - para subtração.
  int num_resultado = 0;

  if (num_operacao == 1)
  {
    operacao[2] = '+'; // Define a operação como soma
    num_resultado = numero1_op + numero2_op;
  }
  else
  {
    operacao[2] = '-'; // Define a operação como subtração
    if (numero1_op > numero2_op)
    {
      num_resultado = numero1_op - numero2_op;
    }
    else
    {
      num_resultado = numero2_op - numero1_op;
      int troca = numero1_op;
      numero1_op = numero2_op;
      numero2_op = troca;
    }
  }
  operacao[0] = numero1_op + '0';
  operacao[1] = ' ';
  operacao[3] = ' ';
  operacao[4] = numero2_op + '0';
  operacao[5] = ' ';
  operacao[6] = '=';
  operacao[7] = ' ';
  operacao[8] = '?';
  operacao[9] = '\0';
  resultado = num_resultado;
}

void jogo_soma_completo()
{
  add_repeating_timer_ms(90, ler_joystick_x, NULL, &timer_joystick);
  limite = 9;
  gerar_soma_subtracao_aleatorio(); // Gera uma operação aleatória.
  sleep_ms(50);
  char *text[] = {operacao}; // Criação do array de texto para armazenar as variáveis
  desenhar_moldura();
  exibir_texto(&disp, &text, 1, 12, 32, 2, 400);
  int salva_numero_anterior = 1;
  uint8_t resposta = 0;
  bool liberar = false;
  button_pressed = false;
  numero_analogico = 0;
  while (!liberar)
  {
    if (numero_analogico != salva_numero_anterior)
    {
      salva_numero_anterior = numero_analogico;
      resposta = numero_analogico;
      npClear();
      preencherBuffer(*numeros[numero_analogico]);
      npWrite();
      play_tone(BUZZER_PIN, 300, 100);
    }
    if (button_pressed)
    {
      if (resposta == resultado)
      {
  liberar = true;
  // Ao confirmar a resposta correta, transformar o número de vermelho para verde
  transformar_vermelho_verde(*numeros[resposta]);
        npClear();
        preencherBuffer(*outras[1]);
        npWrite();
        play_tone(BUZZER_PIN, 450, 70);
        play_tone(BUZZER_PIN, 450, 70);
        sleep_ms(1000);
      }
      else
      {
        for (uint8_t i = 0; i <= 1; i++)
        {
          npClear();
          preencherBuffer(*outras[2]);
          npWrite();
          sleep_ms(200);
          preencherBuffer(*outras[0]);
          npWrite();
          sleep_ms(200);
          play_tone(BUZZER_PIN, 470, 200);
        }
        npClear();
        preencherBuffer(*numeros[resposta]);
        npWrite();
      }
      button_pressed = false;
    }
  }
  cancel_repeating_timer(&timer_joystick);
}

// ##################################################### Funções para ler o Joystick e os Botôes #############################################################

void setup_joystick()
{
  adc_init();
  adc_gpio_init(vRx);
}
bool ler_joystick_x(struct repeating_timer *t)
{
  adc_select_input(adc_channel_vrx);
  int vrx = adc_read();
  if (vrx > 3200)
  {
    if (numero_analogico >= limite)
    {
      numero_analogico = 0;
    }
    else
    {
      numero_analogico++;
    }
  }
  else if (vrx < 200)
  {
    if (numero_analogico <= 0)
    {
      numero_analogico = limite;
    }
    else
    {
      numero_analogico--;
    }
  }
  return true;
}

bool ler_joystick_y(struct repeating_timer *t)
{
  adc_select_input(adc_channel_vry);
  int vry = adc_read();

  // Adicionando um pequeno buffer para evitar leituras muito sensíveis
  static int last_vry = 0;
  if (abs(vry - last_vry) < 50)
    return true; // Evita mudanças pequenas e rápidas
  last_vry = vry;

  if (vry > 3200)
  {
    if (numero_analogico > 0)
    {
      numero_analogico--;
    }
    else
    {
      numero_analogico = 0;
    }
  }
  else if (vry < 200)
  {
    if (numero_analogico < limite)
    {
      numero_analogico++;
    }
    else
    {
      numero_analogico = limite;
    }
  }

  return true;
}

// ##################################################### Funções do Buzzer ############################################################

void pwm_init_buzzer(uint pin)
{
  gpio_set_function(pin, GPIO_FUNC_PWM);
  uint slice_num = pwm_gpio_to_slice_num(pin);
  pwm_config config = pwm_get_default_config();
  pwm_config_set_clkdiv(&config, 4.0f); // Ajusta divisor de clock
  pwm_init(slice_num, &config, true);
  pwm_set_gpio_level(pin, 0); // Desliga o PWM inicialmente
}

// Toca uma nota com a frequência e duração especificadas
void play_tone(uint pin, uint frequency, uint duration_ms)
{
  uint slice_num = pwm_gpio_to_slice_num(pin);
  uint32_t clock_freq = clock_get_hz(clk_sys);
  uint32_t top = clock_freq / frequency - 1;

  pwm_set_wrap(slice_num, top);
  pwm_set_gpio_level(pin, top / 2); // 50% de duty cycle

  sleep_ms(duration_ms);

  pwm_set_gpio_level(pin, 0); // Desliga o som após a duração
  sleep_ms(50);               // Pausa entre notas
}

// ##################################################### Função do jogo da Palavra ####################################################

void pegar_palavra_aleatoria()
{

  uint8_t numero_aleatorio = rand() % 16;
  letra_escolhida = letra_completa[numero_aleatorio];
  id_palavra_escolhida = numero_aleatorio;

  for (uint8_t i = 0; i < 11; i++)
  {
    if (letra_escolhida == letters[i].letter)
    {
      id_letra_faltando = letters[i].id;
    }
  }
}

void jogo_palavra_completo()
{
  add_repeating_timer_ms(90, ler_joystick_x, NULL, &timer_joystick);
  limite = 10;
  pegar_palavra_aleatoria(); // Gera uma operação aleatória.
  sleep_ms(50);
  char *text[] = {banco_de_palavras[id_palavra_escolhida]};
  desenhar_moldura();
  exibir_texto(&disp, &text, 1, 38, 32, 2, 400); // Escreve a operação no display.
  int salva_numero_anterior = 1;
  bool liberar = false;
  uint8_t resposta = 0;
  button_pressed = false;
  numero_analogico = 0;
  while (!liberar)
  {
    if (numero_analogico != salva_numero_anterior)
    {
      salva_numero_anterior = numero_analogico;
      resposta = numero_analogico;
      npClear();
      preencherBuffer(*letras[numero_analogico]);
      npWrite();
      play_tone(BUZZER_PIN, 300, 50);
      printf("%d\n", resposta);
    }

    if (button_pressed)
    {
      if (resposta == id_letra_faltando)
      {
  liberar = true;
  // Ao confirmar a resposta correta, transformar a letra de vermelho para verde
  transformar_vermelho_verde(*letras[id_letra_faltando]);
        npClear();
        preencherBuffer(*outras[1]);
        npWrite();
        play_tone(BUZZER_PIN, 450, 70);
        play_tone(BUZZER_PIN, 450, 70);
        sleep_ms(1000);
        npClear();
        preencherBuffer(*letras[numero_analogico]);
        npWrite();
      }
      else
      {
        for (uint8_t i = 0; i <= 1; i++)
        {
          npClear();
          preencherBuffer(*outras[2]);
          npWrite();
          sleep_ms(200);
          preencherBuffer(*outras[0]);
          npWrite();
          sleep_ms(200);
          play_tone(BUZZER_PIN, 470, 200);
        }
        npClear();
        preencherBuffer(*letras[numero_analogico]);
        npWrite();
      }
      button_pressed = false;
    }
  }
  cancel_repeating_timer(&timer_joystick);
}

// ######################################################## Funções do Display ##############################################//

void setup_gpios(void)
{
  i2c_init(i2c1, 400000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);
  gpio_init(BUTTON_A);
  gpio_set_dir(BUTTON_A, GPIO_IN);
  gpio_pull_up(BUTTON_A);
  gpio_init(BUTTON_B);
  gpio_set_dir(BUTTON_B, GPIO_IN);
  gpio_pull_up(BUTTON_B);
}

void exibir_texto(ssd1306_t *disp, char *words[], size_t word_count, uint x, uint y, uint scale, uint delay_ms)
{
  for (size_t i = 0; i < word_count; ++i)
  {
    ssd1306_draw_string(disp, x, y, scale, words[i]);
    ssd1306_show(disp);
    sleep_ms(delay_ms);
    ssd1306_clear(disp);
  }
}
void desenhar_moldura()
{
  ssd1306_draw_line(&disp, 0, 62, 128, 62);
  ssd1306_draw_line(&disp, 0, 0, 0, 62);
  ssd1306_draw_line(&disp, 0, 0, 128, 1);
  ssd1306_draw_line(&disp, 127, 1, 127, 62);
  ssd1306_show(&disp);
}
void exibir_texto_sem_apagar(ssd1306_t *disp, char *words[], size_t word_count, uint x, uint y, uint scale, uint delay_ms)
{
  for (size_t i = 0; i < word_count; ++i)
  {
    ssd1306_draw_string(disp, x, y, scale, words[i]);
    ssd1306_show(disp);
    sleep_ms(delay_ms);
  }
}

// ################################################### Função de Interrupção ###########################################################################

void button_isr(uint gpio, uint32_t events)
{
  uint32_t now = time_us_32();

  if (now - last_button_time > DEBOUNCE_DELAY_US)
  {
    if (gpio == BUTTON_A)
    {
      button_pressed_2 = true;
    }
    else if (gpio == BUTTON_B)
    {
      button_pressed = true;
    }
    last_button_time = now;
  }
}

// ######################################### Menu de Escolhas #############################################################//

int escolher_dificuldade()
{

  gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_isr);
  bool press_button = 0;
  limite = 2;
  uint8_t static limite_anterior = 1;
  static const char *text[] = {"> Facil"};
  static const char *text1[] = {"> Medio"};
  static const char *text2[] = {"> Dificil"};
  static const char *text3[] = {"Facil"};
  static const char *text4[] = {"Medio"};
  static const char *text5[] = {"Dificil"};
  static const char *text6[] = {"Escolha a dificuldade"};

  ssd1306_clear(&disp);

  int escolha = 0;
  add_repeating_timer_ms(90, ler_joystick_y, NULL, &timer_joystick);
  numero_analogico = 0;
  bool devorteio = true;
  while (!button_pressed && !button_pressed_2)
  {
    printf("%d\n", numero_analogico);

    if ((numero_analogico == 0 && numero_analogico != limite_anterior) || (devorteio == true))
    {
      ssd1306_clear(&disp);
      desenhar_moldura();
      exibir_texto_sem_apagar(&disp, &text6, 1, 2, 2, 1, 1);
      exibir_texto_sem_apagar(&disp, &text, 1, 16, 24, 1, 1);
      exibir_texto_sem_apagar(&disp, &text4, 1, 16, 36, 1, 1);
      exibir_texto_sem_apagar(&disp, &text5, 1, 16, 48, 1, 1);
      play_tone(BUZZER_PIN, 400, 20);
      escolha = 4;
      devorteio = false;
      limite_anterior = numero_analogico;
    }
    else if (numero_analogico == 1 && numero_analogico != limite_anterior)
    {
      ssd1306_clear(&disp);
      desenhar_moldura();
      exibir_texto_sem_apagar(&disp, &text6, 1, 2, 2, 1, 1);
      exibir_texto_sem_apagar(&disp, &text3, 1, 16, 24, 1, 1);
      exibir_texto_sem_apagar(&disp, &text1, 1, 16, 36, 1, 1);
      exibir_texto_sem_apagar(&disp, &text5, 1, 16, 48, 1, 1);
      play_tone(BUZZER_PIN, 400, 20);
      escolha = 8;
      limite_anterior = numero_analogico;
    }
    else if (numero_analogico == 2 && numero_analogico != limite_anterior)
    {
      ssd1306_clear(&disp);
      desenhar_moldura();
      exibir_texto_sem_apagar(&disp, &text6, 1, 2, 2, 1, 1);
      exibir_texto_sem_apagar(&disp, &text3, 1, 16, 24, 1, 1);
      exibir_texto_sem_apagar(&disp, &text4, 1, 16, 36, 1, 1);
      exibir_texto_sem_apagar(&disp, &text2, 1, 16, 48, 1, 1);
      play_tone(BUZZER_PIN, 400, 20);
      escolha = 16;
      limite_anterior = numero_analogico;
    }
    sleep_ms(20);
  }
  gpio_set_irq_enabled(BUTTON_A, GPIO_IRQ_EDGE_FALL, false);
  cancel_repeating_timer(&timer_joystick);
  ssd1306_clear(&disp);
  return escolha;
}

void escolher_jogos()
{
  limite = 1;
  uint8_t static limite_anterior = 0;
  static const char *text[] = {"Escolha um Jogo :"};
  static const char *text1[] = {"> Jogo da Soma"};
  static const char *text2[] = {"> Jogo da Palavra"};
  static const char *text3[] = {"  Jogo da Soma"};
  static const char *text4[] = {"  Jogo da Palavra"};

  add_repeating_timer_ms(90, ler_joystick_y, NULL, &timer_joystick);
  bool devorteio = 0;
  numero_analogico = 0;
  while (!button_pressed)
  {

    if ((numero_analogico == 0 && numero_analogico != limite_anterior) || devorteio == 0)
    {
      limite_anterior = numero_analogico;
      escolher_game = 1;
      devorteio = 1;
      ssd1306_clear(&disp);
      desenhar_moldura();
      exibir_texto_sem_apagar(&disp, &text, 1, 8, 8, 1, 1);
      exibir_texto_sem_apagar(&disp, &text1, 1, 16, 24, 1, 1);
      exibir_texto_sem_apagar(&disp, &text4, 1, 16, 36, 1, 1);
      play_tone(BUZZER_PIN, 400, 20);
    }
    else if (numero_analogico == 1 && numero_analogico != limite_anterior)
    {
      ssd1306_clear(&disp);
      desenhar_moldura();
      exibir_texto_sem_apagar(&disp, &text, 1, 8, 8, 1, 1);
      exibir_texto_sem_apagar(&disp, &text3, 1, 16, 24, 1, 1);
      exibir_texto_sem_apagar(&disp, &text2, 1, 16, 36, 1, 1);

      limite_anterior = numero_analogico;
      escolher_game = 2;
      play_tone(BUZZER_PIN, 400, 20);
    }
    sleep_ms(20);
  }
  cancel_repeating_timer(&timer_joystick);
  return 0;
}

// ################################################### Função do jogo completo ###################################################################
void jogar()
{
  npClear();
  gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_isr);

  while (true)
  {
    int nivel = 0;

    do
    {
      button_pressed_2 = false;
      npClear();
      preencherBuffer(*outras[2]);
      npWrite();

      escolher_jogos();
      sleep_ms(100);
      button_pressed = false;

      nivel = escolher_dificuldade();
      printf("%d\n",nivel);
      sleep_ms(100);

    } while (button_pressed_2);

    button_pressed = false;
    button_pressed_2 = false;
    sleep_ms(150);

    switch (escolher_game)
    {
    case 1:
      char *text[3] = {"Faca", "as", "operacoes"};
      for (uint8_t i = 0; i < 3; i++)
      {
        exibir_texto(&disp, &text[i], 1, 8, 26, 2, 1000); // Escreve a operação no display.
        ssd1306_clear(&disp);
      }
      for (uint8_t i = 0; i < nivel; i++)
      {
        jogo_soma_completo();
        npClear();
        button_pressed = false;
        npClear();
        preencherBuffer(*outras[2]);
        npWrite();
      }
      break;

    case 2:
    char *inicio[3] = {"Complete", "as", "Palavras"};
    for (uint8_t i = 0; i < 3; i++)
    {
      exibir_texto(&disp, &inicio[i], 1, 8, 26, 2, 1000); // Escreve a operação no display.
      ssd1306_clear(&disp);
    }
    for (uint8_t i = 0; i < nivel; i++)
      {
        jogo_palavra_completo();
        npClear();
        button_pressed = false;
        npClear();
        preencherBuffer(*outras[2]);
        npWrite();
      }
      break;

    default:
      printf("Nao entrou");
      break;
    }
    
    char *finalizou[] = {"Ganhou !"};
    exibir_texto(&disp, &finalizou, 1, 16, 30, 2, 1000);
    for (uint8_t i = 0; i <= 6; i++)
    {
      npClear();
      preencherBuffer(*outras[1]);
      npWrite();
      sleep_ms(200);
      preencherBuffer(*outras[2]);
      npWrite();
      sleep_ms(200);
      play_tone(BUZZER_PIN, 450, 70);
    }
    
  }
}
