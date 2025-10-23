#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char operacao[10]; // Arrays com tamanho 2 para armazenar os caracteres e o terminador '\0'.

int gerar_soma_subtracao_aleatorio() {
  
    srand(time(NULL)); // Inicializa o gerador de números aleatórios.
    int numero1_op = rand() % 6; // Gera um número aleatório entre 0 a 5.
    int numero2_op = rand() % 5; // Gera um número aleatório entre 0 a 4.
    int num_operacao = (rand() % 2) + 1; // Gera o número 1 ou 2, 1 - para soma, 2 - para subtração.
    int num_resultado = 0;
  
    if (num_operacao == 1) {
        operacao[2]= '+'; // Define a operação como soma
        num_resultado = numero1_op + numero2_op;
    } else {
        operacao[2] = '-'; // Define a operação como subtração
        if (numero1_op > numero2_op) {
            num_resultado = numero1_op - numero2_op;
        } else {
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
    operacao[8] = num_resultado + '0';
    operacao[9] = '\0';
    return num_resultado;
  }

int main() {
    srand(time(NULL)); // Inicializa o gerador de números aleatórios.
    
    gerar_soma_subtracao_aleatorio(); // Gera uma operação aleatória.
    
    // Criação do array de texto para armazenar as variáveis
    char *text[] = {operacao};
    
    // Exibe a operação armazenada no array text
    printf("%s", text[0]);
    
    return 0;
}
