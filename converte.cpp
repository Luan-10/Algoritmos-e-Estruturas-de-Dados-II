#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINHA 1024
#define MAX_PRODUTOS 10000

typedef struct {
    int id_produto;
    char marca[50];
    char categoria[30];
    float preco;
} Produto;

typedef struct {
    int id_evento;
    int id_usuario;
    int id_produto;
    char tipo_evento[30];
} Acesso;

int main(void) {
    FILE *entrada = fopen("dados.txt", "r");
    FILE *arq_produtos = NULL;
    FILE *arq_acessos = NULL;

    char linha[MAX_LINHA];
    char *campos[9];
    char *campo;

    Produto produtos[MAX_PRODUTOS];
    Acesso acesso;

    int total_produtos = 0;
    int total_acessos = 0;
    int id, encontrado, i;

    if (entrada == NULL) {
        perror("Erro ao abrir dados.txt");
        return 1;
    }

    arq_produtos = fopen("produtos.bin", "wb");
    arq_acessos = fopen("acessos.bin", "wb");

    if (arq_produtos == NULL || arq_acessos == NULL) {
        perror("Erro ao criar arquivos");
        goto erro;
    }

    /* Ignora o cabecalho */
    if (fgets(linha, sizeof(linha), entrada) == NULL)
        goto erro;

    while (fgets(linha, sizeof(linha), entrada) != NULL) {
        linha[strcspn(linha, "\r\n")] = '\0';

        if (linha[0] == '\0')
            continue;

        campo = strtok(linha, ",");

        for (i = 0; i < 9 && campo != NULL; i++) {
            campos[i] = campo;
            campo = strtok(NULL, ",");
        }

        if (i != 9 || campo != NULL)
            continue;

        id = atoi(campos[2]);

        if (id <= 0)
            continue;

        /* Registra o acesso */
        acesso.id_evento = total_acessos + 1;
        acesso.id_usuario = atoi(campos[7]);
        acesso.id_produto = id;

        snprintf(acesso.tipo_evento,
                 sizeof(acesso.tipo_evento), "%s", campos[1]);

        if (fwrite(&acesso, sizeof(Acesso), 1, arq_acessos) != 1) {
            perror("Erro ao gravar acessos.bin");
            goto erro;
        }

        total_acessos++;

        /* Verifica se o produto ja existe */
        encontrado = 0;

        for (i = 0; i < total_produtos; i++) {
            if (produtos[i].id_produto == id) {
                encontrado = 1;
                break;
            }
        }

        /* Cadastra o produto uma unica vez */
        if (!encontrado) {
            if (total_produtos >= MAX_PRODUTOS) {
                printf("Limite de produtos excedido.\n");
                goto erro;
            }

            Produto *p = &produtos[total_produtos];

            p->id_produto = id;
            snprintf(p->marca, sizeof(p->marca), "%s", campos[5]);
            snprintf(p->categoria, sizeof(p->categoria), "%s", campos[4]);
            p->preco = (float)atof(campos[6]);

            total_produtos++;
        }
    }

    if (ferror(entrada)) {
        perror("Erro de leitura");
        goto erro;
    }

    /* Grava os produtos */
    if (fwrite(produtos, sizeof(Produto), total_produtos, arq_produtos)
        != (size_t)total_produtos) {
        perror("Erro ao gravar produtos.bin");
        goto erro;
    }

    fclose(entrada);
    fclose(arq_produtos);
    fclose(arq_acessos);

    printf("Conversao concluida!\n");
    printf("Produtos unicos: %d\n", total_produtos);
    printf("Eventos gravados: %d\n", total_acessos);
    printf("Arquivos criados:\n");
    printf("- produtos.bin\n");
    printf("- acessos.bin\n");

    return 0;

erro:
    if (entrada != NULL)
        fclose(entrada);
    if (arq_produtos != NULL)
        fclose(arq_produtos);
    if (arq_acessos != NULL)
        fclose(arq_acessos);

    return 1;
}
