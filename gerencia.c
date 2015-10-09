/* 
 * File:   gerencia.c
 * Author: romulo
 *
 * Created on 2 de Outubro de 2015, 19:42
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Headers/gerencia.h"

int persistirBanco(Banco* banco, char* nomeArquivoBanco) {
    FILE* file;
    file = fopen(nomeArquivoBanco, "wb");
    if (file == NULL) {
        return 0;
    }
    fwrite(banco, sizeof (Banco), 1, file);
    fclose(file);
    return 1;
}

Banco* carregarBanco(char* nomeArquivoBanco) {
    FILE* file;
    file = fopen(nomeArquivoBanco, "rb");
    if (file == NULL) {
        return NULL;
    }
    Banco* banco;
    fread(banco, sizeof (Banco), 1, file);
    fclose(file);
    return banco;
}

void normalizarArquivo(char* nomeArquivo) {
    FILE* file;
    FILE* temp;
    TokenReader* tokenReader;
    char* linha;
    char* token;

    file = fopen(nomeArquivo, "r");
    temp = fopen("Arquivos/temp", "w");
    linha = (char*) calloc(1000, sizeof (char));
    tokenReader = newTokenReader(linha);

    while (fgets(linha, 1000, file) != NULL) {
        setTokenString(tokenReader, linha);
        while (hasMoreTokens(tokenReader)) {
            token = nextToken(tokenReader);
            if (strcasecmp(token, "\n")) {
                fputs(token, temp);
                fputs(" ", temp);
                if (!strcasecmp(token, ";")) {
                    fputs("\n", temp);
                }
            }
        }
    }
    fclose(file);
    fclose(temp);
}

void interpretarCreateTable(Banco* banco, char* nomeArquivo) {
    FILE* file;
    TokenReader* tokenReader;
    Tabela* tabela;
    Campo* campo;
    char* linha;
    char* token;
    char* nomeBloco;
    char* nomeCampo;
    Tipo tipoCampo;
    int bytesCampo;

    //chamar de fora?
    normalizarArquivo(nomeArquivo);
    file = fopen("Arquivos/temp", "r");
    linha = (char*) calloc(1000, sizeof (char));
    nomeBloco = (char*) calloc(100, sizeof (char));

    //Cada linha corresponde à um CREATE TABLE
    tokenReader = newTokenReader(linha);
    while (fgets(linha, 1000, file) != NULL) {
        setTokenString(tokenReader, linha);

        token = nextToken(tokenReader); //CREATE
        if (!strcasecmp(token, "CREATE")) {
            token = nextToken(tokenReader); //TABLE
            token = nextToken(tokenReader);
            tabela = criarTabela(token);
            adicionarTabela(banco, tabela);
            token = nextToken(tokenReader); //abre parêntesis
        }

        while (hasMoreTokens(tokenReader)) {
            nomeCampo = nextToken(tokenReader); //nome
            if (!strcasecmp(nomeCampo, ";")) {
                break;
            }

            token = nextToken(tokenReader); //tipo

            if (!strcasecmp(token, "INTEGER")) {
                tipoCampo = INTEGER;
                bytesCampo = 4;
            } else if (!strcasecmp(token, "BOOLEAN")) {
                tipoCampo = BOOLEAN;
                bytesCampo = 1;
            } else {
                if (!strcasecmp(token, "CHAR")) {
                    tipoCampo = CHAR;
                } else if (!strcasecmp(token, "VARCHAR")) {
                    tipoCampo = VARCHAR;
                }
                token = nextToken(tokenReader); //abre parêntesis
                token = nextToken(tokenReader); //bytes
                bytesCampo = atoi(token);
                token = nextToken(tokenReader); //fecha parêntesis
            }
            token = nextToken(tokenReader); //vírgula
            campo = criarCampo(nomeCampo, tipoCampo, bytesCampo);
            adicionarCampo(tabela, campo);
        }

        //criar bloco para cada tabela
        sprintf(nomeCampo, "_%d", tabela->numeroBlocos);
        strcpy(nomeBloco, "Arquivos/");
        strcat(nomeBloco, tabela->nome);
        strcat(nomeBloco, nomeCampo);
        strcat(nomeBloco, ".dat");
        gerarBloco(nomeBloco);
        adicionarBloco(tabela, nomeBloco);
    }
    fclose(file);
}

void interpretarInsertInto(Banco* banco, char* nomeArquivo) {
    FILE* file;
    TokenReader* tokenReader;
    Tabela* tabela;
    Campo** campos;
    int contadorCampos;
    Tupla* tupla;
    Associacao* associacao;
    char* linha;
    char* token;

    //chamar de fora?
    normalizarArquivo(nomeArquivo);
    file = fopen("Arquivos/temp", "r");
    linha = (char*) calloc(1000, sizeof (char));

    //Para cada INSERT
    tokenReader = newTokenReader(linha);
    while (fgets(linha, 1000, file) != NULL) {
        setTokenString(tokenReader, linha);
        token = nextToken(tokenReader); //INSERT

        if (!strcasecmp(token, "INSERT")) {
            token = nextToken(tokenReader); //INTO
            token = nextToken(tokenReader);
            tabela = getTabela(banco, token);
            if (!tabela) {
                return;
            }
            token = nextToken(tokenReader); //abre parêntesis
        }

        tupla = criarTupla(tabela);
        //não é preciso pedir memória para cada posição, uma vez que ela apenas apontará para uma região já alocada
        campos = (Campo**) calloc(tabela->numeroCampos, sizeof (Campo*));
        contadorCampos = 0;
        while (hasMoreTokens(tokenReader)) {
            //campos
            while (1) {
                token = nextToken(tokenReader); //campo
                if (!strcasecmp(token, "VALUES")) {
                    break;
                }
                campos[contadorCampos] = getCampo(tabela, token);
                contadorCampos++;
                token = nextToken(tokenReader); //comma
            }

            token = nextToken(tokenReader); //abre parêntesis
            contadorCampos = 0;
            //Para cada valor
            while (1) {
                token = nextToken(tokenReader); //campo
                if (!strcmp(token, ";")) {
                    break;
                }

                if (token[0] == '\'') {

                }

                associacao = findAssociacao(tabela, tupla, campos[contadorCampos]);
                associarValor(associacao, (char*) token);
                contadorCampos++;

                token = nextToken(tokenReader); //comma
            }
            adicionarTupla(tabela, tupla);
            free(campos);
        }
    }
}

void gerarBloco(char* nomeArquivo) {
    FILE* file = fopen(nomeArquivo, "w");
    int i, n = 0;
    for (i = 0; i < 500; i++) {
        fwrite(&n, sizeof (int), 1, file);
    }

    short int deslocamento = 2048;
    fseek(file, 4, SEEK_SET);
    fwrite(&deslocamento, sizeof (short int), 1, file);

    fclose(file);
}
