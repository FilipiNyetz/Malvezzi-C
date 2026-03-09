#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct
{
    int r, c;
} Ponto;

typedef enum
{
    LIMPAR,
    MOVER_N,
    MOVER_S,
    MOVER_L,
    MOVER_O,
    FICAR
} Acao;

typedef struct
{
    Acao *v;
    int cap, ini, sz;
} Log;

Log iniciarLog(int capacidade)
{
    Log log;
    log.v = (Acao *)malloc(capacidade * sizeof(Acao));
    log.cap = capacidade;
    log.ini = 0;
    log.sz = 0;
    return log;
}

void exinirLog(Log *log, Acao acao)
{
    int pos = (log->ini + log->sz) % log->cap;
    if (log->sz < log->cap)
    {
        log->v[pos] = acao;
        log->sz++;
    }
    else
    {
        log->v[log->ini] = acao;
        log->ini = (log->ini + 1) % log->cap;
    }
}

void imprimirLog(const Log *log)
{
    const char *nomes[] = {"LIMPAR", "MOVER_N", "MOVER_S", "MOVER_L", "MOVER_O", "FICAR"};
    printf("Log das ultimas %d acoes:\n", log->sz);
    for (int i = 0; i < log->sz; i++)
    {
        int idx = (log->ini + i) % log->cap;
        printf("  [%d] %s\n", i, nomes[log->v[idx]]);
    }
}

void liberarLog(Log *log)
{
    free(log->v);
    log->v = NULL;
    log->sz = 0;
}

const char *definirDificuldade(int linhasMapa)
{
    switch (linhasMapa)
    {
    case 5:
        return "facil";
    case 7:
        return "medio";
    case 10:
        return "dificil";
    default:
        return "desconhecida";
    }
}

int dentro(int r, int c, int linhasMapa, int colunasMapa)
{
    return r >= 0 && r < linhasMapa && c >= 0 && c < colunasMapa;
}

int eh_sujo(char mapa[][100], int r, int c)
{
    return mapa[r][c] == '*';
}

int eh_bloqueio(char mapa[][100], int r, int c, int linhasMapa, int colunasMapa)
{
    return !dentro(r, c, linhasMapa, colunasMapa) || mapa[r][c] == '#';
}

// Apenas parede '#' conta como bloqueio
int eh_parede(char mapa[][100], int r, int c, int linhasMapa, int colunasMapa)
{
    return dentro(r, c, linhasMapa, colunasMapa) && mapa[r][c] == '#';
}

const char *nome_acao(Acao a)
{
    switch (a)
    {
    case LIMPAR:
        return "LIMPAR";
    case MOVER_N:
        return "MOVER_N (norte)";
    case MOVER_S:
        return "MOVER_S (sul)";
    case MOVER_L:
        return "MOVER_L (leste)";
    case MOVER_O:
        return "MOVER_O (oeste)";
    case FICAR:
        return "FICAR";
    default:
        return "?";
    }
}

Acao decide_reflex(char mapa[][100], int linhasMapa, int colunasMapa,
                   Ponto robo, int *bloqueios)
{
    int r = robo.r, c = robo.c;

    // Regra 1: célula atual suja
    if (eh_sujo(mapa, r, c))
    {
        printf("  Motivo : Regra 1: celula atual suja em (%d,%d)\n", r, c);
        return LIMPAR;
    }

    // Regra 2: vizinho sujo (N, S, L, O)
    if (dentro(r - 1, c, linhasMapa, colunasMapa) && eh_sujo(mapa, r - 1, c))
    {
        printf("  Motivo : Regra 2: vizinho sujo ao norte (%d,%d)\n", r - 1, c);
        return MOVER_N;
    }
    if (dentro(r + 1, c, linhasMapa, colunasMapa) && eh_sujo(mapa, r + 1, c))
    {
        printf("  Motivo : Regra 2: vizinho sujo ao sul (%d,%d)\n", r + 1, c);
        return MOVER_S;
    }
    if (dentro(r, c + 1, linhasMapa, colunasMapa) && eh_sujo(mapa, r, c + 1))
    {
        printf("  Motivo : Regra 2: vizinho sujo ao leste (%d,%d)\n", r, c + 1);
        return MOVER_L;
    }
    if (dentro(r, c - 1, linhasMapa, colunasMapa) && eh_sujo(mapa, r, c - 1))
    {
        printf("  Motivo : Regra 2: vizinho sujo ao oeste (%d,%d)\n", r, c - 1);
        return MOVER_O;
    }

    // Regra 3: zig-zag por colunas
    if (c % 2 == 0)
    {
        // Coluna par: desce, senão leste (→), senão sobe
        if (!eh_bloqueio(mapa, r + 1, c, linhasMapa, colunasMapa))
        {
            printf("  Motivo : Regra 3 (zig-zag): coluna par -> sul\n");
            return MOVER_S;
        }
        if (eh_parede(mapa, r + 1, c, linhasMapa, colunasMapa))
            (*bloqueios)++;
        printf("  Motivo : Regra 3: sul bloqueado, tentando leste\n");
        if (!eh_bloqueio(mapa, r, c + 1, linhasMapa, colunasMapa))
        {
            printf("  Motivo : Regra 3 (zig-zag): coluna par bloqueada -> leste\n");
            return MOVER_L;
        }
        if (eh_parede(mapa, r, c + 1, linhasMapa, colunasMapa))
            (*bloqueios)++;
        printf("  Motivo : Regra 3: leste bloqueado, tentando norte\n");
        if (!eh_bloqueio(mapa, r - 1, c, linhasMapa, colunasMapa))
        {
            printf("  Motivo : Regra 3 (zig-zag): coluna par bloqueada -> norte\n");
            return MOVER_N;
        }
        if (eh_parede(mapa, r - 1, c, linhasMapa, colunasMapa))
            (*bloqueios)++;
    }
    else
    {
        // Coluna ímpar: sobe, caso não de para subir leste, senão desce
        if (!eh_bloqueio(mapa, r - 1, c, linhasMapa, colunasMapa))
        {
            printf("  Motivo : Regra 3 (zig-zag): coluna impar -> norte\n");
            return MOVER_N;
        }
        if (eh_parede(mapa, r - 1, c, linhasMapa, colunasMapa))
            (*bloqueios)++;
        printf("  Motivo : Regra 3: norte bloqueado, tentando leste\n");
        if (!eh_bloqueio(mapa, r, c + 1, linhasMapa, colunasMapa))
        {
            printf("  Motivo : Regra 3 (zig-zag): coluna impar bloqueada -> leste\n");
            return MOVER_L;
        }
        if (eh_parede(mapa, r, c + 1, linhasMapa, colunasMapa))
            (*bloqueios)++;
        printf("  Motivo : Regra 3: leste bloqueado, tentando sul\n");
        if (!eh_bloqueio(mapa, r + 1, c, linhasMapa, colunasMapa))
        {
            printf("  Motivo : Regra 3 (zig-zag): coluna impar bloqueada -> sul\n");
            return MOVER_S;
        }
        if (eh_parede(mapa, r + 1, c, linhasMapa, colunasMapa))
            (*bloqueios)++;
    }

    if (!eh_bloqueio(mapa, r - 1, c, linhasMapa, colunasMapa))
    {
        printf("  Motivo : Fallback: norte\n");
        return MOVER_N;
    }
    if (eh_parede(mapa, r - 1, c, linhasMapa, colunasMapa))
        (*bloqueios)++;
    if (!eh_bloqueio(mapa, r + 1, c, linhasMapa, colunasMapa))
    {
        printf("  Motivo : Fallback: sul\n");
        return MOVER_S;
    }
    if (eh_parede(mapa, r + 1, c, linhasMapa, colunasMapa))
        (*bloqueios)++;
    printf("  Motivo : Fallback: nenhuma direcao disponivel\n");
    return FICAR;
}

int aplicar_acao(char mapa[][100], int linhasMapa, int colunasMapa,
                 Ponto *robo, Acao acao,
                 int *sujeiraTotal, int *pontosLimpos)
{
    if (acao == LIMPAR)
    {
        mapa[robo->r][robo->c] = '.';
        (*sujeiraTotal)--;
        (*pontosLimpos)++;
        return 1;
    }
    if (acao == FICAR)
        return 1;

    int nr = robo->r, nc = robo->c;
    if (acao == MOVER_N)
        nr--;
    if (acao == MOVER_S)
        nr++;
    if (acao == MOVER_L)
        nc++;
    if (acao == MOVER_O)
        nc--;

    if (eh_bloqueio(mapa, nr, nc, linhasMapa, colunasMapa))
        return 0;
    robo->r = nr;
    robo->c = nc;
    return 1;
}

void imprimir_mapa(char mapa[][100], int linhasMapa, int colunasMapa, Ponto robo)
{
    for (int i = 0; i < linhasMapa; i++)
    {
        for (int j = 0; j < colunasMapa; j++)
        {
            if (i == robo.r && j == robo.c)
                printf("R ");
            else
                printf("%c ", mapa[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void espera_enter(void)
{
    printf("  [Aperte enter para continuar]");
    fflush(stdout);
    while (getchar() != '\n')
        ;
}

int main(void)
{
    int linhasMapa, colunasMapa, passosTotaisMax;
    fscanf(stdin, "%d %d %d", &linhasMapa, &colunasMapa, &passosTotaisMax);

    const char *dificuldadeMapa = definirDificuldade(linhasMapa);

    char mapa[100][100];
    Ponto robo = {0, 0};
    int sujeiraTotal = 0;

    for (int i = 0; i < linhasMapa; i++)
    {
        fscanf(stdin, "%s", mapa[i]);
        for (int j = 0; j < colunasMapa; j++)
        {
            if (mapa[i][j] == 'S')
            {
                robo.r = i;
                robo.c = j;
                mapa[i][j] = '.';
            }
            if (mapa[i][j] == '*')
                sujeiraTotal++;
        }
    }

    freopen("/dev/tty", "r", stdin);

    printf("Dificuldade: %s | Sujeira: %d | Passos max: %d\n",
           dificuldadeMapa, sujeiraTotal, passosTotaisMax);
    printf("Deseja modo passo-a-passo? (1=sim, 0=nao): ");
    fflush(stdout);

    int passoPasso = 0;
    scanf("%d", &passoPasso);
    while (getchar() != '\n')
        ;

    if (passoPasso)
    {
        printf("\nMapa inicial:\n");
        imprimir_mapa(mapa, linhasMapa, colunasMapa, robo);
        espera_enter();
    }

    int passosTotais = 0;
    int pontosLimpos = 0;
    int bloqueios = 0;
    Log logAcoes = iniciarLog(10);

    clock_t inicio = clock();

    while (passosTotais < passosTotaisMax && sujeiraTotal > 0)
    {
        printf("--- Passo %d ---\n", passosTotais + 1);
        Acao acao = decide_reflex(mapa, linhasMapa, colunasMapa, robo, &bloqueios);

        int efetiva = aplicar_acao(mapa, linhasMapa, colunasMapa, &robo,
                                   acao, &sujeiraTotal, &pontosLimpos);
        exinirLog(&logAcoes, acao);
        passosTotais++;

        printf("  Acao   : %s\n", nome_acao(acao));
        printf("  Status : %s\n", efetiva ? "ok" : "bloqueado");
        printf("  Sujeira restante: %d | Limpezas: %d | Bloqueios: %d\n",
               sujeiraTotal, pontosLimpos, bloqueios);
        imprimir_mapa(mapa, linhasMapa, colunasMapa, robo);

        if (passoPasso)
            espera_enter();
    }

    clock_t fim = clock();
    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;

    imprimirLog(&logAcoes);
    liberarLog(&logAcoes);

    printf("\nRESULTADO:\n");
    printf("dificuldade  : %s\n", dificuldadeMapa);
    printf("passosTotais : %d\n", passosTotais);
    printf("pontosLimpos : %d\n", pontosLimpos);
    printf("bloqueios    : %d\n", bloqueios);
    printf("sujeiraTotal : %d\n", sujeiraTotal);
    printf("tempo        : %f seg\n", tempo);

    return 0;
}
