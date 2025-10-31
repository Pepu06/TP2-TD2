#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 500

#define GRID_OFFSET_X 220
#define GRID_OFFSET_Y 59
#define GRID_WIDTH 650
#define GRID_HEIGHT 425

#define GRID_ROWS 5
#define GRID_COLS 9
#define CELL_WIDTH (GRID_WIDTH / GRID_COLS)
#define CELL_HEIGHT (GRID_HEIGHT / GRID_ROWS)

#define PEASHOOTER_FRAME_WIDTH 177
#define PEASHOOTER_FRAME_HEIGHT 166
#define PEASHOOTER_TOTAL_FRAMES 31
#define PEASHOOTER_ANIMATION_SPEED 4
#define PEASHOOTER_SHOOT_FRAME 18

#define ZOMBIE_FRAME_WIDTH 164
#define ZOMBIE_FRAME_HEIGHT 203
#define ZOMBIE_TOTAL_FRAMES 90
#define ZOMBIE_ANIMATION_SPEED 2
#define ZOMBIE_DISTANCE_PER_CYCLE 40.0f

#define MAX_ARVEJAS 100
#define PEA_SPEED 5
#define ZOMBIE_SPAWN_RATE 300

// ========= ESTRUCTURAS DE DATOS =========
typedef struct
{
    int row, col;
} Cursor;

typedef struct
{
    SDL_Rect rect;
    int activo;
    int cooldown;
    int current_frame;
    int frame_timer;
    int debe_disparar;
} Planta;

typedef struct
{
    SDL_Rect rect;
    int activo;
} Arveja;

typedef struct
{
    SDL_Rect rect;
    int activo;
    int vida;
    int row;
    int current_frame;
    int frame_timer;
    float pos_x;
} Zombie;

// ========= NUEVAS ESTRUCTURAS =========
#define STATUS_VACIO 0
#define STATUS_PLANTA 1

typedef struct RowSegment
{
    int status;
    int start_col;
    int length;
    Planta *planta_data;
    struct RowSegment *next;
} RowSegment;

typedef struct ZombieNode
{
    Zombie zombie_data;
    struct ZombieNode *next;
} ZombieNode;

typedef struct GardenRow
{
    RowSegment *first_segment;
    ZombieNode *first_zombie;
} GardenRow;

typedef struct GameBoard
{
    GardenRow rows[GRID_ROWS];
    Arveja arvejas[MAX_ARVEJAS]; // array adicional para manejar las arvejas
    int zombie_spawn_timer;      // variable para saber cada cuanto crear un zombie
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *tex_background = NULL;
SDL_Texture *tex_peashooter_sheet = NULL;
SDL_Texture *tex_zombie_sheet = NULL;
SDL_Texture *tex_pea = NULL;

Cursor cursor = {0, 0};
GameBoard *game_board = NULL;

// ========= FUNCIONES =========

GameBoard *gameBoardNew()
{
    GameBoard *board = (GameBoard *)malloc(sizeof(GameBoard));
    if (!board)
        return NULL;

    board->zombie_spawn_timer = ZOMBIE_SPAWN_RATE;

    for (int i = 0; i < GRID_ROWS; i++)
    {
        RowSegment *first = (RowSegment *)malloc(sizeof(RowSegment));
        if (!first)
        {
            free(board);
            return NULL;
        }
        first->status = STATUS_VACIO;
        first->start_col = 0;
        first->length = GRID_COLS;
        first->planta_data = NULL;
        first->next = NULL;

        board->rows[i].first_segment = first;
        board->rows[i].first_zombie = NULL;
    }
    for (int i = 0; i < MAX_ARVEJAS; i++)
    {
        board->arvejas[i].activo = 0;
    }
    return board;
}

void gameBoardDelete(GameBoard *board)
{
    // TODO: Liberar toda la memoria dinámica.
    // TODO: Recorrer cada GardenRow.
    for (int i = 0; i < GRID_ROWS; i++)
    {
        RowSegment *segmento = &board->rows[i].first_segment;
        while (segmento)
        {
            // TODO: Liberar todos los RowSegment (y los planta_data si existen).
            RowSegment *proximo = segmento->next;
            free(segmento);
            if (segmento->planta_data)
            {
                free(segmento->planta_data);
            }
            segmento = proximo;
        }
        // TODO: Liberar todos los ZombieNode.
        ZombieNode *zombie = &board->rows[i].first_zombie;
        while (zombie)
        {
            ZombieNode *proximo = zombie->next;
            free(zombie);
            zombie = proximo;
        }
    }
    // TODO: Finalmente, liberar el GameBoard.
    free(board);
    printf("Función gameBoardDelete no implementada.\n");
}

int gameBoardAddPlant(GameBoard *board, int row, int col)
{
    // TODO: Encontrar la GardenRow correcta.
    RowSegment *segmento = board->rows[row].first_segment;

    // TODO: Recorrer la lista de RowSegment hasta encontrar el segmento VACIO que contenga a `col`.
    while (segmento)
    {
        RowSegment *proximo = segmento->next;
        if (segmento->start_col <= col && (segmento->start_col + segmento->length) > col && segmento->status == STATUS_VACIO)
        {
            // TODO: Si se encuentra y tiene espacio, realizar la lógica de DIVISIÓN de segmento.

            // TODO: Crear la nueva `Planta` con memoria dinámica y asignarla al `planta_data` del nuevo segmento.
            Planta *p = (Planta *)malloc(sizeof(Planta));
            p->rect.x = GRID_OFFSET_X + (cursor.col * CELL_WIDTH);
            p->rect.y = GRID_OFFSET_Y + (cursor.row * CELL_HEIGHT);
            p->rect.w = CELL_WIDTH;
            p->rect.h = CELL_HEIGHT;
            p->activo = 1;
            p->cooldown = rand() % 100;
            p->current_frame = 0;
            p->frame_timer = 0;
            p->debe_disparar = 0;

            int longitud = segmento->length;

            // Si col es la ultima columna del segmento
            if (segmento->start_col + segmento->length - 1 == col)
            {
                RowSegment *planta_segmento = (RowSegment *)malloc(sizeof(RowSegment));
                planta_segmento->status = STATUS_PLANTA;
                planta_segmento->start_col = col;
                planta_segmento->length = 1;
                planta_segmento->planta_data = p;

                // enlazamos los nuevos segmentos
                segmento->next = planta_segmento;
                planta_segmento->next = proximo;
            }
            // si col esta en el principio del segmento
            else if (col == segmento->start_col)
            {
                segmento->status = STATUS_PLANTA;
                segmento->length = 1;
                segmento->planta_data = p;

                RowSegment *nuevo_vacio = (RowSegment *)malloc(sizeof(RowSegment));
                nuevo_vacio->status = STATUS_VACIO;
                nuevo_vacio->start_col = col + 1;
                nuevo_vacio->length = longitud - 1;
                nuevo_vacio->planta_data = NULL;

                // enlazamos los nuevos segmentos
                segmento->next = nuevo_vacio;
                nuevo_vacio->next = proximo;
            }
            // Si col esta en el medio
            else
            {
                RowSegment *planta_segmento = (RowSegment *)malloc(sizeof(RowSegment));
                planta_segmento->status = STATUS_PLANTA;
                planta_segmento->start_col = col;
                planta_segmento->length = 1;
                planta_segmento->planta_data = p;

                segmento->length = col - segmento->start_col;

                RowSegment *nuevo_vacio = (RowSegment *)malloc(sizeof(RowSegment));
                nuevo_vacio->status = STATUS_VACIO;
                nuevo_vacio->start_col = col + 1;
                nuevo_vacio->length = longitud - segmento->length - 1;
                nuevo_vacio->planta_data = NULL;

                // enlazamos los nuevos segmentos
                segmento->next = planta_segmento;
                planta_segmento->next = nuevo_vacio;
                nuevo_vacio->next = proximo;
            }
            return 0;
        }
        segmento = proximo;
    }
    printf("Función gameBoardAddPlant no implementada.\n");
    return 0;
}

void gameBoardRemovePlant(GameBoard *board, int row, int col)
{
    // TODO: Similar a AddPlant, encontrar el segmento que contiene `col`.
    RowSegment *segmento = board->rows[row].first_segment;
    RowSegment *prev = NULL;

    // TODO: Recorrer los segmentos hasta encontrar el que contiene la planta a eliminar.
    while (segmento)
    {
        RowSegment *proximo = segmento->next;

        // TODO: Si es un segmento de tipo PLANTA que contiene `col`, eliminarlo.
        if (segmento->start_col <= col && col < segmento->start_col + segmento->length && segmento->status == STATUS_PLANTA)
        {
            // Se libera la planta y se asigna el status vaico
            free(segmento->planta_data);
            segmento->planta_data = NULL;
            segmento->status = STATUS_VACIO;

            // Ifs para chequear si debe fusionarse con el segmento anterior, con el siguiente o ambos
            if (prev && prev->status == STATUS_VACIO)
            {
                // Fusionar con el segmento anterior vacío
                prev->length =  prev->length + segmento->length;
                prev->next = segmento->next;
                free(segmento);
                segmento = prev;
            }

            if (segmento->next && segmento->next->status == STATUS_VACIO)
            {
                // Fusionar con el segmento siguiente vacío
                RowSegment *sig = segmento->next;
                segmento->length = segmento->length + sig->length;
                segmento->next = sig->next;
                free(sig);
            }

            return;
        }

        prev = segmento;
        segmento = proximo;
    }

    // TODO: Implementar la lógica de FUSIÓN con los segmentos vecinos si también son VACIO.
    printf("Función gameBoardRemovePlant no implementada.\n");
}

void gameBoardAddZombie(GameBoard *board, int row)
{
    // TODO: Crear un nuevo ZombieNode con memoria dinámica.
    // TODO: Inicializar sus datos (posición, vida, animación, etc.).
    // TODO: Agregarlo a la lista enlazada simple de la GardenRow correspondiente.
    printf("Función gameBoardAddZombie no implementada.\n");
}

void gameBoardUpdate(GameBoard *board)
{
    if (!board)
        return;
    // TODO: Re-implementar la lógica de `actualizarEstado` usando las nuevas estructuras.
    // TODO: Recorrer las listas de zombies de cada fila para moverlos y animarlos.
    // TODO: Recorrer las listas de segmentos de cada fila para gestionar los cooldowns y animaciones de las plantas.
    // TODO: Actualizar la lógica de disparo, colisiones y spawn de zombies.
}

void gameBoardDraw(GameBoard *board)
{
    if (!board)
        return;
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, tex_background, NULL, NULL);

    // TODO: Re-implementar la lógica de `dibujar` usando las nuevas estructuras.
    // TODO: Recorrer las listas de segmentos para dibujar las plantas.
    // TODO: Recorrer las listas de zombies para dibujarlos.
    // TODO: Dibujar las arvejas y el cursor.

    SDL_RenderPresent(renderer);
}

SDL_Texture *cargarTextura(const char *path)
{
    SDL_Texture *newTexture = IMG_LoadTexture(renderer, path);
    if (newTexture == NULL)
        printf("No se pudo cargar la textura %s! SDL_image Error: %s\n", path, IMG_GetError());
    return newTexture;
}
int inicializar()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 0;
    window = SDL_CreateWindow("Plantas vs Zombies - Base para TP", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
        return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
        return 0;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
        return 0;
    tex_background = cargarTextura("res/Frontyard.png");
    tex_peashooter_sheet = cargarTextura("res/peashooter_sprite_sheet.png");
    tex_zombie_sheet = cargarTextura("res/zombie_sprite_sheet.png");
    tex_pea = cargarTextura("res/pea.png");
    return 1;
}
void cerrar()
{
    SDL_DestroyTexture(tex_background);
    SDL_DestroyTexture(tex_peashooter_sheet);
    SDL_DestroyTexture(tex_zombie_sheet);
    SDL_DestroyTexture(tex_pea);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char *args[])
{
    srand(time(NULL));
    if (!inicializar())
        return 1;

    game_board = gameBoardNew();

    SDL_Event e;
    int game_over = 0;

    while (!game_over)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
                game_over = 1;
            if (e.type == SDL_MOUSEMOTION)
            {
                int mouse_x = e.motion.x;
                int mouse_y = e.motion.y;
                if (mouse_x >= GRID_OFFSET_X && mouse_x < GRID_OFFSET_X + GRID_WIDTH &&
                    mouse_y >= GRID_OFFSET_Y && mouse_y < GRID_OFFSET_Y + GRID_HEIGHT)
                {
                    cursor.col = (mouse_x - GRID_OFFSET_X) / CELL_WIDTH;
                    cursor.row = (mouse_y - GRID_OFFSET_Y) / CELL_HEIGHT;
                }
            }
            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                gameBoardAddPlant(game_board, cursor.row, cursor.col);
            }
        }

        gameBoardUpdate(game_board);
        gameBoardDraw(game_board);

        // TODO: Agregar la lógica para ver si un zombie llegó a la casa y terminó el juego

        SDL_Delay(16);
    }

    gameBoardDelete(game_board);
    cerrar();
    return 0;
}

// Duplica un string. Debe contar la cantidad de caracteres totales de src y solicitar la memoria equivalente. Luego, debe copiar todos los caracteres a esta nueva ´area de memoria. Adem´as, como valor de retorno se debe retornar el puntero al nuevo string.
char *strDuplicate(char *src)
{
    int cont = 0;
    for (int i = 0; src[i] != '\0'; i++)
    {
        cont++;
    }
    char *duplica = (char *)malloc(sizeof(char) * cont + 1);
    for (int i = 0; src[i] != '\0'; i++)
    {
        duplica[i] = src[i];
    }
    duplica[cont] = '\0';

    return duplica;
}

// Compara dos strings lexicograficamente. Devuelve 0 si son iguales, 1 si el primer string es menor al segundo, -1 en otro caso.
int strCompare(char *s1, char *s2)
{
    int largo1 = 0;
    for (int i = 0; s1[i] != '\0'; i++)
    {
        largo1++;
    }
    int largo2 = 0;
    for (int i = 0; s2[i] != '\0'; i++)
    {
        largo2++;
    }
    if (largo1 < largo2)
    {
        return 1;
    }
    else if (largo1 == largo2)
    {
        for (int i = 0; i < largo1; i++)
        {
            if (s1[i] != s2[i])
            {
                if (s1[i] < s2[i])
                {
                    return 1;
                }
                return -1;
            }
        }
        return 0;
    }
    else
    {
        return -1;
    }
}

// La funci´on toma dos strings src1 y src2, y retorna un nuevo string que contiene una copia de todos los caracteres de src1 seguidos de los caracteres de src2. Adem´as, la memoria de src1 y src2 debe ser liberada.
char *strConcatenate(char *src1, char *src2)
{
    int largo1 = 0;
    for (int i = 0; src1[i] != '\0'; i++)
    {
        largo1++;
    }
    int largo2 = 0;
    for (int i = 0; src2[i] != '\0'; i++)
    {
        largo2++;
    }
    char *ret = (char *)malloc(sizeof(char) * (largo1 + largo2) + 1);

    int i = 0;
    for (; i < largo1; i++)
    {
        ret[i] = src1[i];
    }
    for (int j = 0; j < largo2; j++)
    {
        ret[i + j] = src2[j];
    }
    ret[largo1 + largo2] = '\0';
    free(src1);
    free(src2);
    return ret;
}
