/* 
 * File:   dados.h
 * Author: romulo
 *
 * Created on 2 de Outubro de 2015, 18:51
 */

#include "banco.h"

void gerarBloco(char* nomeArquivo);
int calcTamanhoInserir(Tupla *tupla, int qtdAss);
int inserirRegistros(Tabela* tabela);
int obterBloco(int tamanho, char** nomesArquivosBlocos, int numeroBlocos, char* nomeTabela);
void setMapaBits(int posicao, int* mapaBits);
void carregarRegistros(Tabela* tabela);
int remover(Tabela* tabela, Campo* campo, char operador, void* valor);