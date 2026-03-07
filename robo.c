#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Etapa 2 — Declarações
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

// Log circular de ações
typedef struct
{
    Acao *v;
    int   cap, ini, sz;
} Log;

Log iniciarLog(int capacidade)
{
    Log log;
    log.v   = (Acao *)malloc(capacidade * sizeof(Acao));
    log.cap = capacidade;
    log.ini = 0;
    log.sz  = 0;
    return log;
}

void registrarAcao(Log *log, Acao acao)
{
    int pos = (log->ini + log->sz) % log->cap;
    if (log->sz < log->cap)
    {
        log->v[pos] = acao;
        log->sz++;
    }
    else
    {
        // Buffer cheio: sobrescreve o mais antigo
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
    log->v  = NULL;
    log->sz = 0;
}

// Etapa 1 — Definir dificuldade com base nas linhas do mapa
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

// Funções auxiliares
int dentroDoMapa(int r, int c, int linhasMapa, int colunasMapa)
{
    return r >= 0 && r < linhasMapa && c >= 0 && c < colunasMapa;
}

int estaLivre(char mapa[][100], int r, int c, int linhasMapa, int colunasMapa)
{
    return dentroDoMapa(r, c, linhasMapa, colunasMapa) && mapa[r][c] != '#';
}

// Etapa 3, 4 e 5 — Lógica do agente reflexo
Acao decidirAcao(char mapa[][100], int linhasMapa, int colunasMapa, Ponto robo)
{
    // Etapa 5: Limpar sujeira atual (prioridade máxima)
    if (mapa[robo.r][robo.c] == '*')
    {
        printf("Regra 1: limpar sujeira atual\n");
        return LIMPAR;
    }

    // Etapa 3: Procurar sujeira nos vizinhos (N, S, L, O)
    int dr[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, 1, -1};

    for (int i = 0; i < 4; i++)
    {
        int nr = robo.r + dr[i];
        int nc = robo.c + dc[i];
        if (dentroDoMapa(nr, nc, linhasMapa, colunasMapa) && mapa[nr][nc] == '*')
        {
            printf("Regra 2: sujeira vizinha encontrada\n");
            if (i == 0) return MOVER_N;
            if (i == 1) return MOVER_S;
            if (i == 2) return MOVER_L;
            if (i == 3) return MOVER_O;
        }
    }

    // Etapa 3 + 4: Zig-zag por colunas
    Acao direcaoPrincipal = (robo.c % 2 == 0) ? MOVER_S : MOVER_N;
    Acao direcaoOposta    = (robo.c % 2 == 0) ? MOVER_N : MOVER_S;
    printf("Regra 3 (zig-zag): coluna %s\n", robo.c % 2 == 0 ? "par" : "impar");

    // Tentar direção principal vertical
    int nrPrincipal = (direcaoPrincipal == MOVER_S) ? robo.r + 1 : robo.r - 1;
    if (estaLivre(mapa, nrPrincipal, robo.c, linhasMapa, colunasMapa))
        return direcaoPrincipal;

    // Etapa 4: Bloqueado — tentar leste
    if (estaLivre(mapa, robo.r, robo.c + 1, linhasMapa, colunasMapa))
        return MOVER_L;

    // Etapa 4: Leste bloqueado — tentar direção oposta vertical
    int nrOposto = (direcaoOposta == MOVER_S) ? robo.r + 1 : robo.r - 1;
    if (estaLivre(mapa, nrOposto, robo.c, linhasMapa, colunasMapa))
        return direcaoOposta;

    // Fallback: tentar leste, oeste ou ficar
    printf("Fallback: tentando leste ou oeste\n");
    if (estaLivre(mapa, robo.r, robo.c + 1, linhasMapa, colunasMapa))
        return MOVER_L;
    if (estaLivre(mapa, robo.r, robo.c - 1, linhasMapa, colunasMapa))
        return MOVER_O;

    return FICAR;
}

void imprimirMapa(char mapa[][100], int linhasMapa, int colunasMapa, Ponto robo)
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

int main()
{
    // Etapa 1 — Leitura do mapa
    int linhasMapa, colunasMapa, passosTotaisMax;
    fscanf(stdin, "%d %d %d", &linhasMapa, &colunasMapa, &passosTotaisMax);

    const char *dificuldadeMapa = definirDificuldade(linhasMapa);
    printf("Dificuldade: %s\n", dificuldadeMapa);

    // Etapa 2 — Construção do robô
    char mapa[100][100];
    Ponto robo;
    int sujeiraTotal = 0;

    for (int i = 0; i < linhasMapa; i++)
    {
        fscanf(stdin, "%s", mapa[i]);
        printf("Linha %d: %s\n", i, mapa[i]);
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

    printf("Sujeira encontrada: %d\n", sujeiraTotal);
    printf("Posicao do robo: %d %d\n", robo.r, robo.c);

    // Etapa 6 — Contadores
    int passosTotais = 0;
    int pontosLimpos = 0;
    int bloqueios    = 0;

    Log logAcoes = iniciarLog(10);

    clock_t inicio = clock();

    // Loop principal
    while (passosTotais < passosTotaisMax && sujeiraTotal > 0)
    {
        Acao acao = decidirAcao(mapa, linhasMapa, colunasMapa, robo);
        registrarAcao(&logAcoes, acao);

        if (acao == LIMPAR)
        {
            mapa[robo.r][robo.c] = '.';
            sujeiraTotal--;
            pontosLimpos++;
        }
        else if (acao != FICAR)
        {
            int nr = robo.r;
            int nc = robo.c;

            if (acao == MOVER_N) nr--;
            if (acao == MOVER_S) nr++;
            if (acao == MOVER_L) nc++;
            if (acao == MOVER_O) nc--;

            // Etapa 4: incrementar bloqueios apenas quando o robô bate de fato
            if (!dentroDoMapa(nr, nc, linhasMapa, colunasMapa) || mapa[nr][nc] == '#')
            {
                bloqueios++;
            }
            else
            {
                robo.r = nr;
                robo.c = nc;
            }
        }

        // Etapa 6: incrementar passos ao fim de cada ciclo
        passosTotais++;
        printf("Robo em (%d,%d)\n", robo.r, robo.c);
        imprimirMapa(mapa, linhasMapa, colunasMapa, robo);
    }

    clock_t fim = clock();
    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;

    imprimirLog(&logAcoes);
    liberarLog(&logAcoes);

    printf("RESULTADO:\n");
    printf("dificuldade: %s\n", dificuldadeMapa);
    printf("passosTotais: %d\n", passosTotais);
    printf("pontosLimpos: %d\n", pontosLimpos);
    printf("bloqueios: %d\n", bloqueios);
    printf("sujeiraTotal: %d\n", sujeiraTotal);
    printf("tempo: %f seg\n", tempo);

    return 0;
}
