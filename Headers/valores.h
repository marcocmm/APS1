/* 
 * File:   dados.h
 * Author: romulo
 *
 * Created on 2 de Outubro de 2015, 18:51
 */

#include "banco.h"

typedef struct valores Valores;

struct valores {
    Tabela* tabela;
    int** integer;
    char** boolean;
    char** chars;
    char** varchar;
};

void inserirRegistro(Valores* dados);
int calcTamanhoInserir();
int procurarEspaco(char* nomeArq, int tamanho);