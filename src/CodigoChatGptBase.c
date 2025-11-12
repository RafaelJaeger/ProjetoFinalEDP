/* rede_social.c
   Rede de Amizades (grafo não orientado)
   Representação: Lista de adjacência (com matriz de adjacência atualizada)
   Limite máximo de vértices: MAX_VERTICES (20 por padrão)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VERTICES 20
#define NAME_LEN 50

/* ----- Estruturas ----- */

typedef struct AdjNode {
    int v;                  // índice do vértice adjacente
    struct AdjNode *next;
} AdjNode;

typedef struct {
    char *name;             // nome do usuário (alocado dinamicamente)
    AdjNode *head;          // cabeça da lista de adjacência
} Vertex;

typedef struct {
    Vertex vertices[MAX_VERTICES];
    int n;                  // número atual de vértices
    int adjMatrix[MAX_VERTICES][MAX_VERTICES]; // matriz de adjacência espelho
} Graph;

/* ----- Funções utilitárias ----- */

void init_graph(Graph *g) {
    g->n = 0;
    for (int i = 0; i < MAX_VERTICES; ++i) {
        g->vertices[i].name = NULL;
        g->vertices[i].head = NULL;
        for (int j = 0; j < MAX_VERTICES; ++j) g->adjMatrix[i][j] = 0;
    }
}

char *strdup_local(const char *s) {
    if (!s) return NULL;
    size_t l = strlen(s) + 1;
    char *p = malloc(l);
    if (p) memcpy(p, s, l);
    return p;
}

/* Cria um novo nó de adjacência */
AdjNode *create_adj_node(int v) {
    AdjNode *node = (AdjNode*) malloc(sizeof(AdjNode));
    if (!node) {
        fprintf(stderr, "Erro: sem memória para nó.\n");
        exit(EXIT_FAILURE);
    }
    node->v = v;
    node->next = NULL;
    return node;
}

/* Encontra índice do vértice pelo nome; retorna -1 se não encontrado */
int find_vertex_index(Graph *g, const char *name) {
    for (int i = 0; i < g->n; ++i) {
        if (g->vertices[i].name && strcmp(g->vertices[i].name, name) == 0)
            return i;
    }
    return -1;
}

/* ----- Operações no grafo ----- */

/* Adiciona vértice com nome (retorna 0 sucesso, -1 se cheio, -2 se já existe) */
int add_vertex(Graph *g, const char *name) {
    if (g->n >= MAX_VERTICES) return -1;
    if (find_vertex_index(g, name) != -1) return -2;
    g->vertices[g->n].name = strdup_local(name);
    g->vertices[g->n].head = NULL;
    // adjMatrix já inicializada; linha/coluna correspondentes já zero
    g->n++;
    return 0;
}

/* Adiciona aresta não orientada entre índices u e v (retorna 0 sucesso, -1 erro) */
int add_edge_by_index(Graph *g, int u, int v) {
    if (u < 0 || u >= g->n || v < 0 || v >= g->n) return -1;
    if (u == v) return -1; // sem loop
    if (g->adjMatrix[u][v]) return -1; // já existe

    // inserir no início da lista (u -> v)
    AdjNode *n1 = create_adj_node(v);
    n1->next = g->vertices[u].head;
    g->vertices[u].head = n1;

    // (v -> u)
    AdjNode *n2 = create_adj_node(u);
    n2->next = g->vertices[v].head;
    g->vertices[v].head = n2;

    g->adjMatrix[u][v] = g->adjMatrix[v][u] = 1;
    return 0;
}

/* Inserir aresta por nomes */
int add_edge(Graph *g, const char *name1, const char *name2) {
    int u = find_vertex_index(g, name1);
    int v = find_vertex_index(g, name2);
    if (u == -1 || v == -1) return -1;
    return add_edge_by_index(g, u, v);
}

/* Remove ocorrência target da lista do vertice idx (libera memória de nodes removidos) */
void remove_occurrences_from_list(Vertex *vert, int target) {
    AdjNode *curr = vert->head;
    AdjNode *prev = NULL;
    while (curr) {
        if (curr->v == target) {
            AdjNode *tmp = curr;
            if (prev) prev->next = curr->next;
            else vert->head = curr->next;
            curr = curr->next;
            free(tmp);
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
}

/* Remove aresta por índices (não atualiza índices dos vértices) */
int remove_edge_by_index(Graph *g, int u, int v) {
    if (u < 0 || v < 0 || u >= g->n || v >= g->n) return -1;
    if (!g->adjMatrix[u][v]) return -1; // não existe

    // remover v da lista de u
    AdjNode *curr = g->vertices[u].head;
    AdjNode *prev = NULL;
    while (curr) {
        if (curr->v == v) {
            if (prev) prev->next = curr->next;
            else g->vertices[u].head = curr->next;
            free(curr);
            break;
        }
        prev = curr; curr = curr->next;
    }

    // remover u da lista de v
    curr = g->vertices[v].head; prev = NULL;
    while (curr) {
        if (curr->v == u) {
            if (prev) prev->next = curr->next;
            else g->vertices[v].head = curr->next;
            free(curr);
            break;
        }
        prev = curr; curr = curr->next;
    }

    g->adjMatrix[u][v] = g->adjMatrix[v][u] = 0;
    return 0;
}

/* Remove aresta por nomes */
int remove_edge(Graph *g, const char *name1, const char *name2) {
    int u = find_vertex_index(g, name1);
    int v = find_vertex_index(g, name2);
    if (u == -1 || v == -1) return -1;
    return remove_edge_by_index(g, u, v);
}

/* Libera toda a lista de adjacência de um vértice */
void free_adj_list(AdjNode *head) {
    AdjNode *curr = head;
    while (curr) {
        AdjNode *tmp = curr;
        curr = curr->next;
        free(tmp);
    }
}

/* Remove vértice no índice target (compacta o vetor de vértices e ajusta índices) */
int remove_vertex_by_index(Graph *g, int target) {
    if (target < 0 || target >= g->n) return -1;

    // 1) Remover todas as ocorrências de target nas listas dos outros vértices
    for (int i = 0; i < g->n; ++i) {
        if (i == target) continue;
        remove_occurrences_from_list(&g->vertices[i], target);
    }

    // 2) Liberar a lista do próprio vértice e o nome
    free_adj_list(g->vertices[target].head);
    free(g->vertices[target].name);
    g->vertices[target].head = NULL;
    g->vertices[target].name = NULL;

    // 3) Shift (compactar) vertices à esquerda
    for (int i = target; i < g->n - 1; ++i) {
        g->vertices[i] = g->vertices[i + 1];
    }
    // Limpar última posição agora duplicada
    g->vertices[g->n - 1].head = NULL;
    g->vertices[g->n - 1].name = NULL;

    // 4) Atualizar matriz de adjacência: shift linhas/colunas
    for (int i = target; i < g->n - 1; ++i) {
        for (int j = 0; j < g->n; ++j) {
            g->adjMatrix[i][j] = g->adjMatrix[i + 1][j];
        }
    }
    for (int j = target; j < g->n - 1; ++j) {
        for (int i = 0; i < g->n - 1; ++i) {
            g->adjMatrix[i][j] = g->adjMatrix[i][j + 1];
        }
    }
    // limpar última linha/coluna
    for (int i = 0; i < g->n; ++i) {
        g->adjMatrix[g->n - 1][i] = 0;
        g->adjMatrix[i][g->n - 1] = 0;
    }

    // 5) Ajustar índices nos nós das listas (decrementar índices maiores que target)
    for (int i = 0; i < g->n - 1; ++i) {
        AdjNode *curr = g->vertices[i].head;
        while (curr) {
            if (curr->v > target) curr->v--;
            curr = curr->next;
        }
    }

    g->n--;
    return 0;
}

/* Remove vértice por nome */
int remove_vertex(Graph *g, const char *name) {
    int idx = find_vertex_index(g, name);
    if (idx == -1) return -1;
    return remove_vertex_by_index(g, idx);
}

/* ----- Exibição ----- */

/* Exibe lista de adjacência */
void display_adj_list(Graph *g) {
    printf("Lista de Adjacência:\n");
    for (int i = 0; i < g->n; ++i) {
        printf(" %d: %s -> ", i, g->vertices[i].name);
        AdjNode *curr = g->vertices[i].head;
        if (!curr) printf("NULL");
        while (curr) {
            printf("%s", g->vertices[curr->v].name);
            if (curr->next) printf(" -> ");
            curr = curr->next;
        }
        printf("\n");
    }
}

/* Exibe matriz de adjacência */
void display_adj_matrix(Graph *g) {
    printf("\nMatriz de Adjacência (0/1):\n    ");
    for (int j = 0; j < g->n; ++j) {
        printf("%3d", j);
    }
    printf("\n   +");
    for (int j = 0; j < g->n; ++j) printf("---");
    printf("\n");
    for (int i = 0; i < g->n; ++i) {
        printf("%2d |", i);
        for (int j = 0; j < g->n; ++j) {
            printf("%3d", g->adjMatrix[i][j]);
        }
        printf("   %s\n", g->vertices[i].name);
    }
}

/* Gera e exibe matriz de incidência (n x m) */
void display_incidence_matrix(Graph *g) {
    // Primeiro, coletar todas as arestas (u < v)
    int m = 0;
    int edges[MAX_VERTICES * MAX_VERTICES][2]; // lista de pares (u,v)
    for (int u = 0; u < g->n; ++u) {
        for (int v = u + 1; v < g->n; ++v) {
            if (g->adjMatrix[u][v]) {
                edges[m][0] = u;
                edges[m][1] = v;
                m++;
            }
        }
    }
    printf("\nMatriz de Incidência (%d vértices x %d arestas):\n    ", g->n, m);
    for (int e = 0; e < m; ++e) printf("%3d", e);
    printf("\n   +");
    for (int e = 0; e < m; ++e) printf("---");
    printf("\n");
    for (int i = 0; i < g->n; ++i) {
        printf("%2d |", i);
        for (int e = 0; e < m; ++e) {
            int a = edges[e][0], b = edges[e][1];
            if (i == a || i == b) printf("%3d", 1);
            else printf("%3d", 0);
        }
        printf("   %s\n", g->vertices[i].name);
    }
    if (m == 0) printf("(Sem arestas)\n");
}

/* Visualização ASCII simples */
void ascii_visual(Graph *g) {
    printf("\nVisualização ASCII (lista):\n");
    for (int i = 0; i < g->n; ++i) {
        printf("[%d] %s", i, g->vertices[i].name);
        AdjNode *curr = g->vertices[i].head;
        if (!curr) { printf(" -- (sem amigos)\n"); continue; }
        printf(" -- ");
        int first = 1;
        while (curr) {
            if (!first) printf(", ");
            printf("%s", g->vertices[curr->v].name);
            first = 0;
            curr = curr->next;
        }
        printf("\n");
    }
}

/* Gera arquivo .dot para Graphviz */
void generate_dot(Graph *g, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Erro ao criar .dot");
        return;
    }
    fprintf(f, "graph RedeAmizades {\n");
    // imprimir nós com rótulo como nome
    for (int i = 0; i < g->n; ++i) {
        fprintf(f, "  v%d [label=\"%s\"];\n", i, g->vertices[i].name);
    }
    // arestas (u < v)
    for (int u = 0; u < g->n; ++u) {
        for (int v = u + 1; v < g->n; ++v) {
            if (g->adjMatrix[u][v]) {
                fprintf(f, "  v%d -- v%d;\n", u, v);
            }
        }
    }
    fprintf(f, "}\n");
    fclose(f);
    printf("\nArquivo '%s' gerado. Visualize com: dot -Tpng %s -o grafo.png\n", filename, filename);
}

/* ----- Percursos ----- */

/* Fila simples para BFS */
typedef struct {
    int *data;
    int head, tail, cap;
} Queue;

Queue *queue_create(int cap) {
    Queue *q = malloc(sizeof(Queue));
    q->data = malloc(sizeof(int) * cap);
    q->head = q->tail = 0;
    q->cap = cap;
    return q;
}
void queue_free(Queue *q) { free(q->data); free(q); }
int queue_empty(Queue *q) { return q->head == q->tail; }
void queue_push(Queue *q, int x) { q->data[q->tail++] = x; }
int queue_pop(Queue *q) { return q->data[q->head++]; }

/* BFS: imprime ordem de visita e retorna número de visitados */
int bfs(Graph *g, int start, int *visited_order, int max_out) {
    if (start < 0 || start >= g->n) return 0;
    int *visited = calloc(g->n, sizeof(int));
    Queue *q = queue_create(g->n + 5);
    visited[start] = 1;
    queue_push(q, start);
    int count = 0;
    while (!queue_empty(q)) {
        int u = queue_pop(q);
        if (count < max_out) visited_order[count] = u;
        count++;
        AdjNode *curr = g->vertices[u].head;
        while (curr) {
            int v = curr->v;
            if (!visited[v]) {
                visited[v] = 1;
                queue_push(q, v);
            }
            curr = curr->next;
        }
    }
    free(visited);
    queue_free(q);
    return count;
}

/* DFS recursiva (registro de ordem) */
void dfs_util(Graph *g, int u, int *visited, int *order, int *pos) {
    visited[u] = 1;
    order[(*pos)++] = u;
    AdjNode *curr = g->vertices[u].head;
    while (curr) {
        if (!visited[curr->v]) dfs_util(g, curr->v, visited, order, pos);
        curr = curr->next;
    }
}
int dfs(Graph *g, int start, int *order, int max_out) {
    if (start < 0 || start >= g->n) return 0;
    int *visited = calloc(g->n, sizeof(int));
    int pos = 0;
    dfs_util(g, start, visited, order, &pos);
    free(visited);
    return pos;
}

/* ----- Grafo de exemplo (pré-definido) ----- */
void insert_sample_graph(Graph *g) {
    // nomes simples
    const char *names[] = {"Alice", "Bob", "Carol", "Dave", "Eve", "Frank"};
    int cnt = sizeof(names)/sizeof(names[0]);
    for (int i = 0; i < cnt; ++i) add_vertex(g, names[i]);
    add_edge(g, "Alice", "Bob");
    add_edge(g, "Alice", "Carol");
    add_edge(g, "Bob", "Dave");
    add_edge(g, "Carol", "Eve");
    add_edge(g, "Eve", "Frank");
    add_edge(g, "Bob", "Carol");
    add_edge(g, "Dave", "Frank");
    printf("Grafo de exemplo inserido (%d vértices).\n", g->n);
}

/* ----- Limpeza final ----- */
void free_graph(Graph *g) {
    for (int i = 0; i < g->n; ++i) {
        free_adj_list(g->vertices[i].head);
        free(g->vertices[i].name);
        g->vertices[i].head = NULL;
        g->vertices[i].name = NULL;
    }
    g->n = 0;
}

/* ----- Menu e interação (entrada segura de strings) ----- */

void read_line(char *buffer, int size) {
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }
    // remover \n
    size_t l = strlen(buffer);
    if (l > 0 && buffer[l-1] == '\n') buffer[l-1] = '\0';
}

void menu() {
    printf("\n===== Rede de Amizades (Grafo) =====\n");
    printf("1 - Inserir vértice (pessoa)\n");
    printf("2 - Inserir aresta (amizade)\n");
    printf("3 - Remover vértice\n");
    printf("4 - Remover aresta\n");
    printf("5 - Exibir grafo (lista, matriz, incidência)\n");
    printf("6 - BFS (lista amigos / conexões)\n");
    printf("7 - DFS (ordem de visita)\n");
    printf("8 - Inserir grafo de exemplo\n");
    printf("9 - Gerar arquivo grafo.dot (Graphviz)\n");
    printf("10 - Visualização ASCII\n");
    printf("0 - Sair\n");
    printf("Escolha: ");
}

int main() {
    Graph g;
    init_graph(&g);

    char input[128];
    int option = -1;
    while (1) {
        menu();
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        option = atoi(input);
        if (option == 0) break;

        if (option == 1) {
            char name[NAME_LEN];
            printf("Nome da nova pessoa: ");
            read_line(name, NAME_LEN);
            if (strlen(name) == 0) { printf("Nome vazio. Cancelado.\n"); continue; }
            int r = add_vertex(&g, name);
            if (r == 0) printf("Pessoa '%s' adicionada (indice %d).\n", name, g.n - 1);
            else if (r == -1) printf("Erro: limite de vértices alcançado (%d).\n", MAX_VERTICES);
            else if (r == -2) printf("Erro: já existe pessoa com esse nome.\n");
        }
        else if (option == 2) {
            char a[NAME_LEN], b[NAME_LEN];
            printf("Nome da pessoa 1: ");
            read_line(a, NAME_LEN);
            printf("Nome da pessoa 2: ");
            read_line(b, NAME_LEN);
            if (strcmp(a,b) == 0) { printf("Não é possível criar amizade consigo mesmo.\n"); continue; }
            int r = add_edge(&g, a, b);
            if (r == 0) printf("Amizade entre '%s' e '%s' adicionada.\n", a, b);
            else printf("Erro ao adicionar aresta (verifique nomes ou já existe).\n");
        }
        else if (option == 3) {
            char name[NAME_LEN];
            printf("Nome da pessoa a remover: ");
            read_line(name, NAME_LEN);
            int r = remove_vertex(&g, name);
            if (r == 0) printf("Pessoa '%s' removida.\n", name);
            else printf("Erro: pessoa não encontrada.\n");
        }
        else if (option == 4) {
            char a[NAME_LEN], b[NAME_LEN];
            printf("Nome da pessoa 1: ");
            read_line(a, NAME_LEN);
            printf("Nome da pessoa 2: ");
            read_line(b, NAME_LEN);
            int r = remove_edge(&g, a, b);
            if (r == 0) printf("Amizade entre '%s' e '%s' removida.\n", a, b);
            else printf("Erro ao remover aresta (verifique nomes/existência).\n");
        }
        else if (option == 5) {
            display_adj_list(&g);
            display_adj_matrix(&g);
            display_incidence_matrix(&g);
        }
        else if (option == 6) {
            char name[NAME_LEN];
            printf("Nome da pessoa para BFS: ");
            read_line(name, NAME_LEN);
            int idx = find_vertex_index(&g, name);
            if (idx == -1) { printf("Pessoa nao encontrada.\n"); continue; }
            int order[MAX_VERTICES];
            int visited_count = bfs(&g, idx, order, MAX_VERTICES);
            printf("Ordem de visita BFS (a partir de %s):\n", name);
            if (visited_count == 0) { printf("(nenhum)\n"); continue; }
            for (int i = 0; i < visited_count && i < MAX_VERTICES; ++i) {
                int id = order[i];
                printf(" %d: %s\n", id, g.vertices[id].name);
            }
            printf("Total visitados: %d\n", visited_count);
        }
        else if (option == 7) {
            char name[NAME_LEN];
            printf("Nome da pessoa para DFS: ");
            read_line(name, NAME_LEN);
            int idx = find_vertex_index(&g, name);
            if (idx == -1) { printf("Pessoa nao encontrada.\n"); continue; }
            int order[MAX_VERTICES];
            int visited_count = dfs(&g, idx, order, MAX_VERTICES);
            printf("Ordem de visita DFS (a partir de %s):\n", name);
            for (int i = 0; i < visited_count; ++i) {
                printf(" %d: %s\n", order[i], g.vertices[order[i]].name);
            }
            printf("Total visitados: %d\n", visited_count);
        }
        else if (option == 8) {
            insert_sample_graph(&g);
        }
        else if (option == 9) {
            generate_dot(&g, "grafo.dot");
        }
        else if (option == 10) {
            ascii_visual(&g);
        }
        else {
            printf("Opção inválida.\n");
        }
    }

    free_graph(&g);
    printf("Encerrando.\n");
    return 0;
}
