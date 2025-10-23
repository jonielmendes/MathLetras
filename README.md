# Curso EmbarcaTech - Projeto Final

## Sobre o Projeto
Este projeto tem como objetivo auxiliar o aprendizado de matemática e língua portuguesa para crianças com Síndrome de Down. Através de um sistema embarcado interativo, os usuários participam de jogos educativos que incentivam a aprendizagem de soma, subtração e formação de palavras.

O sistema utiliza um Raspberry Pi Pico W e componentes eletrônicos como joystick, display OLED e matriz de LEDs para oferecer uma experiência lúdica e acessível.

## Objetivos
- Criar um jogo interativo para auxiliar na educação de crianças com Síndrome de Down.
- Utilizar tecnologia embarcada para tornar o ensino mais acessível.
- Oferecer feedback visual e sonoro para incentivar o aprendizado.
- Garantir uma interface intuitiva e de fácil navegação.

## Funcionalidades
- [x] Seleção de jogos e níveis de dificuldade via joystick.
- [x] Feedback visual e sonoro para respostas corretas e incorretas.
- [x] Jogos de soma/subtração e formação de palavras.
- [x] Exibição dos desafios no display OLED.
- [x] Interação e navegação intuitiva com botões físicos.

## Componentes Utilizados
- Raspberry Pi Pico W (Microcontrolador principal)
- Display OLED 128x64 (I2C) (Interface gráfica do usuário)
- Matriz de LEDs 5x5 (Exibição de alternativas e feedback visual)
- Joystick (Navegação no menu e seleção de opções)
- Botões táteis (Push Buttons) (Confirmação de ações)
- Buzzer (Feedback sonoro)

Estrutura do Software
1. Camada de Inicialização: Configuração dos periféricos e inicialização do sistema.
2. Camada de Escolha: Seleção do tipo de jogo e nível de dificuldade.
3. Camada de Jogo: Execução do jogo escolhido pelo usuário.
4. Camada de Feedback: Apresentação dos resultados e incentivo ao usuário.

Bibliotecas Utilizadas
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "inc/ssd1306.h"

Como Executar o Projeto
# Configurar o ambiente de desenvolvimento
Instalar o Raspberry Pi Pico SDK e as ferramentas de compilação.
Configurar o VSCode com a extensão do Raspberry Pi Pico.

# Compilar e carregar o código
cmake ..
make
picotool load nome_do_arquivo.uf2

Possíveis Melhorias
+ Implementação de um display maior e colorido.
+ Inclusão de um alto-falante para melhorar a experiência sonora.
+ Monitoramento do desempenho dos alunos via sistema web.
+ Painel de controle para professores adicionarem novos conteúdos.
