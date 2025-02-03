#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "pico/sync.h" // Inclusão para irq_set_enabled

#include "ws2818b.pio.h"

// Definições
#define LED_COUNT 25
#define LED_PIN 7
#define LED_PIN2 13

#define BUTTON_A 5
#define BUTTON_B 6

// Tipo para representar um LED RGB (GRB)
typedef struct {
    uint8_t G, R, B;
} npLED_t;

// Array de LEDs
npLED_t leds[LED_COUNT];

// Variáveis para a máquina de estados do PIO
PIO np_pio;
uint sm;

//contador
volatile int animacao_atual = 0; 

// Variáveis para debouncing dos botões
volatile uint32_t last_time_A = 0; 
volatile uint32_t last_time_B = 0; 

// Tempo de debounce em microssegundos (200ms)
const uint32_t debounce_delay_us = 200000;

// Protótipos das funções de interrupção
void button_a_irq_handler(uint gpio, uint32_t events);
void button_b_irq_handler(uint gpio, uint32_t events);

// Função para inicializar a matriz de LEDs WS2812
void npInit(uint pin) {
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;
  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0) {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true); 
  }

  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
  
  for (uint i = 0; i < LED_COUNT; ++i) {
    leds[i].R = 0;
    leds[i].G = 0;
    leds[i].B = 0;
  }
}

// Função para definir a cor de um LED individual
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
  leds[index].R = r;
  leds[index].G = g;
  leds[index].B = b;
}

// Função para limpar a matriz de LEDs
void npClear() {
  for (uint i = 0; i < LED_COUNT; ++i)
    npSetLED(i, 0, 0, 0);
}

// Função para escrever os dados dos LEDs para a matriz
void npWrite() {
  for (uint i = 0; i < LED_COUNT; ++i) {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100); 
}

// Função para calcular o índice de um LED na matriz a partir de suas coordenadas (x, y)
int getIndex(int x, int y) {
    if (y % 2 == 0) {
        return 24-(y * 5 + x); // Linha par
    } else {
        return 24-(y * 5 + (4 - x)); // Linha ímpar
    }
}

// Funções de animação 
void animacao1() {
  int matriz[5][5][3] = {
      {{0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}},
      {{255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
      {{255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
      {{255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
      {{0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}}
  };
  // Desenhando Sprite contido na matriz.
  for (int linha = 0; linha < 5; linha++) {
      for (int coluna = 0; coluna < 5; coluna++) {
          int posicao = getIndex(linha, coluna);
          npSetLED(posicao, matriz[coluna][linha][0], matriz[coluna][linha][1], matriz[coluna][linha][2]);
      }
  }
}

void animacao2() {
  int matriz2[5][5][3] = {
    {{0, 0, 0}, {0, 0, 0}, {255, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {255, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {255, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}}
  };


  for (int linha = 0; linha < 5; linha++) {
      for (int coluna = 0; coluna < 5; coluna++) {
          int posicao = getIndex(linha, coluna);
          npSetLED(posicao, matriz2[coluna][linha][0], matriz2[coluna][linha][1], matriz2[coluna][linha][2]);
      }
  }
}

void animacao3() {
  int matriz3[5][5][3] = {
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}}
};
  for (int linha = 0; linha < 5; linha++) {
      for (int coluna = 0; coluna < 5; coluna++) {
          int posicao = getIndex(linha, coluna);
          npSetLED(posicao, matriz3[coluna][linha][0], matriz3[coluna][linha][1], matriz3[coluna][linha][2]);
      }
  }
}

void animacao4() {
  int matriz4[5][5][3] = {
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
    {{0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}}
};
   for (int linha = 0; linha < 5; linha++) {
      for (int coluna = 0; coluna < 5; coluna++) {
          int posicao = getIndex(linha, coluna);
          npSetLED(posicao, matriz4[coluna][linha][0], matriz4[coluna][linha][1], matriz4[coluna][linha][2]);
      }
  }
}

void animacao5() {
  int matriz5[5][5][3] = {
    {{255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}}
};
  for (int linha = 0; linha < 5; linha++) {
      for (int coluna = 0; coluna < 5; coluna++) {
          int posicao = getIndex(linha, coluna);
          npSetLED(posicao, matriz5[coluna][linha][0], matriz5[coluna][linha][1], matriz5[coluna][linha][2]);
      }
  }

}

void animacao6() {
  int matriz6[5][5][3] = {
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}}
};
   for (int linha = 0; linha < 5; linha++) {
      for (int coluna = 0; coluna < 5; coluna++) {
          int posicao = getIndex(linha, coluna);
          npSetLED(posicao, matriz6[coluna][linha][0], matriz6[coluna][linha][1], matriz6[coluna][linha][2]);
      }
  }

}

void animacao7(){
  int matriz7[5][5][3] = {
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}}
};
  for (int linha = 0; linha < 5; linha++) {
      for (int coluna = 0; coluna < 5; coluna++) {
          int posicao = getIndex(linha, coluna);
          npSetLED(posicao, matriz7[coluna][linha][0], matriz7[coluna][linha][1], matriz7[coluna][linha][2]);
      }
  }
}

void animacao8(){
  int matriz8[5][5][3] = {
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {255, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {255, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {255, 0, 0}, {0, 0, 0}, {0, 0, 0}}
};
  for (int linha = 0; linha < 5; linha++) {
      for (int coluna = 0; coluna < 5; coluna++) {
          int posicao = getIndex(linha, coluna);
          npSetLED(posicao, matriz8[coluna][linha][0], matriz8[coluna][linha][1], matriz8[coluna][linha][2]);
      }
  }
}

void animacao9(){
  int matriz9[5][5][3] = {
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}}
};
  for (int linha = 0; linha < 5; linha++) {
      for (int coluna = 0; coluna < 5; coluna++) {
          int posicao = getIndex(linha, coluna);
          npSetLED(posicao, matriz9[coluna][linha][0], matriz9[coluna][linha][1], matriz9[coluna][linha][2]);
      }
  }  
}

void animacao10(){
  int matriz10[5][5][3] = {
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}}
};
  for (int linha = 0; linha < 5; linha++) {
      for (int coluna = 0; coluna < 5; coluna++) {
          int posicao = getIndex(linha, coluna);
          npSetLED(posicao, matriz10[coluna][linha][0], matriz10[coluna][linha][1], matriz10[coluna][linha][2]);
      }
  }  
}

// Loop principal do programa
void loop() {
      npClear();

    switch (animacao_atual) {
        case 0: animacao1(); break;
        case 1: animacao2(); break;
        case 2: animacao3(); break;
        case 3: animacao4(); break;
        case 4: animacao5(); break;
        case 5: animacao6(); break;
        case 6: animacao7(); break;
        case 7: animacao8(); break;
        case 8: animacao9(); break;
        case 9: animacao10(); break;
    }

    npWrite();
    sleep_ms(100);

    blinkLED();
}

// Função para piscar o LED RGB
void blinkLED() {
    static uint32_t last_time = 0;
    const uint32_t interval = 100;

    if (to_ms_since_boot(get_absolute_time()) - last_time >= interval) {
        last_time = to_ms_since_boot(get_absolute_time());
        gpio_put(LED_PIN2, !gpio_get(LED_PIN2));
    }
}

// Função de interrupção com debouncing
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Se o evento for do botão A
    if (gpio == BUTTON_A) { // Usando BUTTON_A diretamente
        // Verifica se passou tempo suficiente para o debounce
        if (current_time - last_time_A > debounce_delay_us) {
            last_time_A = current_time; // Atualiza o tempo do último evento
            animacao_atual++; // Incrementa a animação atual
            if (animacao_atual > 9) {
                animacao_atual = 0; // Volta para a primeira animação se chegar ao limite
            }
            printf("Botão A pressionado! Animação: %d\n", animacao_atual);
        }
    }

    // Se o evento for do botão B
    if (gpio == BUTTON_B) { // Usando BUTTON_B diretamente
        // Verifica se passou tempo suficiente para o debounce
        if (current_time - last_time_B > debounce_delay_us) {
            last_time_B = current_time; // Atualiza o tempo do último evento
            animacao_atual--; // Decrementa a animação atual
            if (animacao_atual < 0) {
                animacao_atual = 9; // Volta para a última animação se chegar ao limite
            }
            printf("Botão B pressionado! Animação: %d\n", animacao_atual);
        }
    }
}

int main() {
    stdio_init_all();
    gpio_init(LED_PIN2);
    gpio_set_dir(LED_PIN2, GPIO_OUT);

    npInit(LED_PIN);

    // Configura os botões com interrupções
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); 

    // Habilita as interrupções globalmente
    irq_set_enabled(0, true);

    while (true) {
        loop();
    }
}