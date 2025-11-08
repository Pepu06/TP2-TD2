#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

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
        RowSegment *segmento = board->rows[i].first_segment;
        while (segmento)
        {
            // TODO: Liberar todos los RowSegment (y los planta_data si existen).
            RowSegment *proximo = segmento->next;
            if (segmento->planta_data)
            {
                free(segmento->planta_data);
            }
            free(segmento);
            segmento = proximo;
        }
        // TODO: Liberar todos los ZombieNode.
        ZombieNode *zombie = board->rows[i].first_zombie;
        while (zombie)
        {
            ZombieNode *proximo = zombie->next;
            free(zombie);
            zombie = proximo;
        }
    }
    // TODO: Finalmente, liberar el GameBoard.
    free(board);
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
            p->rect.x = GRID_OFFSET_X + (col * CELL_WIDTH);
            p->rect.y = GRID_OFFSET_Y + (row * CELL_HEIGHT);
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
        if (segmento->start_col <= col && col < (segmento->start_col + segmento->length) && segmento->status == STATUS_PLANTA)
        {
            // Se libera la planta y se asigna el status vaico
            free(segmento->planta_data);
            segmento->planta_data = NULL;
            segmento->status = STATUS_VACIO;

            // Ifs para chequear si debe fusionarse con el segmento anterior, con el siguiente o ambos
            if (prev && prev->status == STATUS_VACIO)
            {
                // Fusionar con el segmento anterior vacío
                prev->length = prev->length + segmento->length;
                prev->next = segmento->next;
                free(segmento);
                segmento = prev;
            }

            if (proximo && proximo->status == STATUS_VACIO)
            {
                // Fusionar con el segmento siguiente vacío
                segmento->length = segmento->length + proximo->length;
                segmento->next = proximo->next;
                free(proximo);
                proximo = NULL;
            }

            return;
        }

        prev = segmento;
        segmento = proximo;
    }

    // TODO: Implementar la lógica de FUSIÓN con los segmentos vecinos si también son VACIO.
}

void gameBoardAddZombie(GameBoard *board, int row)
{
    // TODO: Crear un nuevo ZombieNode con memoria dinámica.
    ZombieNode *nuevoZombieNode = (ZombieNode *)malloc(sizeof(ZombieNode));

    // TODO: Inicializar sus datos (posición, vida, animación, etc.).
    // creo un nuevo zombie
    Zombie *z = (Zombie *)malloc(sizeof(Zombie));
    z->row = row;
    z->pos_x = SCREEN_WIDTH;
    z->rect.x = (int)z->pos_x;
    z->rect.y = GRID_OFFSET_Y + (z->row * CELL_HEIGHT);
    z->rect.w = CELL_WIDTH;
    z->rect.h = CELL_HEIGHT;
    z->vida = 100;
    z->activo = 1;
    z->current_frame = 0;
    z->frame_timer = 0;

    // lo asigno al zombienode
    nuevoZombieNode->zombie_data = *z;

    // TODO: Agregarlo a la lista enlazada simple de la GardenRow correspondiente.
    // hago que el next del nuevo zombienode sea el primero de la lista y luego pongo al nuevo primero
    nuevoZombieNode->next = board->rows[row].first_zombie;
    board->rows[row].first_zombie = nuevoZombieNode;
}

// funcion para disparar una arveja desde la planta en la posicion row,col modificada pero casi igual a la del juego base
void dispararArveja(int row, int col, GameBoard *board)
{
    for (int i = 0; i < MAX_ARVEJAS; i++)
    {
        if (!board->arvejas[i].activo)
        {
            board->arvejas[i].rect.x = GRID_OFFSET_X + (col * CELL_WIDTH) + (CELL_WIDTH / 2);
            board->arvejas[i].rect.y = GRID_OFFSET_Y + (row * CELL_HEIGHT) + (CELL_HEIGHT / 4);
            board->arvejas[i].rect.w = 20;
            board->arvejas[i].rect.h = 20;
            board->arvejas[i].activo = 1;
            break;
        }
    }
}

void gameBoardUpdate(GameBoard *board)
{
    if (!board)
        return;
    // TODO: Re-implementar la lógica de `actualizarEstado` usando las nuevas estructuras.
    // TODO: Recorrer las listas de zombies de cada fila para moverlos y animarlos.
    for (int i = 0; i < GRID_ROWS; i++)
    {
        ZombieNode *zombieNode = board->rows[i].first_zombie;
        // recorro todos los zombies de la fila y actualizo el estado de cada uno como en juego base
        while (zombieNode)
        {
            if (zombieNode->zombie_data.activo)
            {
                Zombie *z = &zombieNode->zombie_data;
                float distance_per_tick = ZOMBIE_DISTANCE_PER_CYCLE / (float)(ZOMBIE_TOTAL_FRAMES * ZOMBIE_ANIMATION_SPEED);
                z->pos_x -= distance_per_tick;
                z->rect.x = (int)z->pos_x;
                z->frame_timer++;
                if (z->frame_timer >= ZOMBIE_ANIMATION_SPEED)
                {
                    z->frame_timer = 0;
                    z->current_frame = (z->current_frame + 1) % ZOMBIE_TOTAL_FRAMES;
                }
            }

            zombieNode = zombieNode->next;
        }
    }
    // TODO: Recorrer las listas de segmentos de cada fila para gestionar los cooldowns y animaciones de las plantas.
    for (int i = 0; i < GRID_ROWS; i++)
    {
        RowSegment *segmento = board->rows[i].first_segment;
        // recorro todos los segmentos
        while (segmento)
        {
            // chequeo si el segmento tiene una planta activa y actualizo su estado como en juego base
            if (segmento->status == STATUS_PLANTA)
            {
                if (segmento->planta_data->activo)
                {
                    Planta *p = segmento->planta_data;
                    if (p->cooldown <= 0)
                    {
                        p->debe_disparar = 1;
                    }
                    else
                    {
                        p->cooldown--;
                    }
                    p->frame_timer++;
                    if (p->frame_timer >= PEASHOOTER_ANIMATION_SPEED)
                    {
                        p->frame_timer = 0;
                        p->current_frame = (p->current_frame + 1) % PEASHOOTER_TOTAL_FRAMES;
                        if (p->debe_disparar && p->current_frame == PEASHOOTER_SHOOT_FRAME)
                        {
                            // disparar arveja
                            dispararArveja(i, segmento->start_col, board);
                            p->cooldown = 120;
                            p->debe_disparar = 0;
                        }
                    }
                    segmento->planta_data = p;
                }
            }
            segmento = segmento->next;
        }
    }
    // TODO: Actualizar la lógica de disparo, colisiones y spawn de zombies.

    // logica de disparo como en juego base
    for (int i = 0; i < MAX_ARVEJAS; i++)
    {
        if (board->arvejas[i].activo)
        {
            board->arvejas[i].rect.x += PEA_SPEED;
            if (board->arvejas[i].rect.x > SCREEN_WIDTH)
                board->arvejas[i].activo = 0;
        }
    }

    // logica de colision como en juego base
    for (int i = 0; i < GRID_ROWS; i++)
    {
        ZombieNode *zombieNode = board->rows[i].first_zombie;

        while (zombieNode)
        {
            if (!zombieNode->zombie_data.activo)
            {
                zombieNode = zombieNode->next;
                continue;
            }
            for (int j = 0; j < MAX_ARVEJAS; j++)
            {
                if (!board->arvejas[j].activo)
                    continue;
                int arveja_row = (board->arvejas[j].rect.y - GRID_OFFSET_Y) / CELL_HEIGHT;
                if (zombieNode->zombie_data.row == arveja_row)
                {
                    if (SDL_HasIntersection(&board->arvejas[j].rect, &zombieNode->zombie_data.rect))
                    {
                        board->arvejas[j].activo = 0;
                        zombieNode->zombie_data.vida -= 25;
                        if (zombieNode->zombie_data.vida <= 0)
                            zombieNode->zombie_data.activo = 0;
                    }
                }
            }
            zombieNode = zombieNode->next;
        }
    }

    // logica de spawn
    board->zombie_spawn_timer--;
    if (board->zombie_spawn_timer <= 0)
    {
        gameBoardAddZombie(board, rand() % GRID_ROWS);
        board->zombie_spawn_timer = ZOMBIE_SPAWN_RATE;
    }
}

void gameBoardDraw(GameBoard *board)
{
    if (!board)
        return;
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, tex_background, NULL, NULL);

    // TODO: Re-implementar la lógica de `dibujar` usando las nuevas estructuras.
    // TODO: Recorrer las listas de segmentos para dibujar las plantas.
    for (int i = 0; i < GRID_ROWS; i++)
    {
        RowSegment *segmento = board->rows[i].first_segment;
        // recorro todos los segmentos
        while (segmento)
        {
            // chequeo si el segmento tiene una planta activa y la dibujo
            if (segmento->status == STATUS_PLANTA && segmento->planta_data)
            {
                Planta *p = segmento->planta_data;
                int frame = p->current_frame % PEASHOOTER_TOTAL_FRAMES;

                SDL_Rect src_rect = {frame * PEASHOOTER_FRAME_WIDTH, 0,
                                     PEASHOOTER_FRAME_WIDTH, PEASHOOTER_FRAME_HEIGHT};

                SDL_RenderCopy(renderer, tex_peashooter_sheet, &src_rect, &p->rect);
            }
            segmento = segmento->next;
        }
    }
    // TODO: Recorrer las listas de zombies para dibujarlos.
    for (int i = 0; i < GRID_ROWS; i++)
    {
        ZombieNode *zombie = board->rows[i].first_zombie;
        // recorro todos los zombies de la fila y los dibujo
        while (zombie)
        {
            if (zombie->zombie_data.activo)
            {
                Zombie *z = &zombie->zombie_data;
                SDL_Rect src_rect = {z->current_frame * ZOMBIE_FRAME_WIDTH, 0, ZOMBIE_FRAME_WIDTH, ZOMBIE_FRAME_HEIGHT};
                SDL_RenderCopy(renderer, tex_zombie_sheet, &src_rect, &z->rect);
            }
            zombie = zombie->next;
        }
    }
    // TODO: Dibujar las arvejas y el cursor.
    for (int i = 0; i < MAX_ARVEJAS; i++)
    {
        if (board->arvejas[i].activo)
        {
            SDL_RenderCopy(renderer, tex_pea, NULL, &board->arvejas[i].rect);
        }
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 200);
    SDL_Rect cursor_rect = {GRID_OFFSET_X + cursor.col * CELL_WIDTH, GRID_OFFSET_Y + cursor.row * CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT};
    SDL_RenderDrawRect(renderer, &cursor_rect);
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

// Duplica un string. Debe contar la cantidad de caracteres totales de src y solicitar la memoria equivalente. Luego, debe copiar todos los caracteres a esta nueva ´area de memoria. Adem´as, como valor de retorno se debe retornar el puntero al nuevo string.
char *strDuplicate(char *src)
{
    // calculo el largo del string
    int cont = 0;
    for (int i = 0; src[i] != '\0'; i++)
    {
        cont++;
    }

    // reservo memoria para el nuevo string
    char *duplica = (char *)malloc(sizeof(char) * cont + 1);

    // copio el string en el nuevo espacio de memoria
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
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') // se fija letra por letra si hay alguna menor lexicograficamente antes de que se terminen los strings para devolver el valor correspondiente
    {
        if (s1[i] < s2[i])
        {
            return 1;
        }
        else if (s1[i] > s2[i])
        {
            return -1;
        }
        i++;
    }
    // si algun string termina antes de que haya diferencias se evaluan los posibles casos
    if (s1[i] == '\0' && s2[i] == '\0') // si son iguales para devolver 0
    {
        return 0;
    }
    else if (s1[i] == '\0') // si termino primero s1 para devolver 1
    {
        return 1;
    }
    else // si termino primero s2 para devolver -1
    {
        return -1;
    }
}

// La funci´on toma dos strings src1 y src2, y retorna un nuevo string que contiene una copia de todos los caracteres de src1 seguidos de los caracteres de src2. Adem´as, la memoria de src1 y src2 debe ser liberada.
char *strConcatenate(char *src1, char *src2)
{
    // calculo el largo de ambos strings
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

    // reservo memoria para el nuevo string
    char *ret = (char *)malloc(sizeof(char) * ((largo1 + largo2) + 1));


    // copio ambos strings en el nuevo
    int i = 0;
    for (; i < largo1; i++)
    {
        ret[i] = src1[i];
    }
    for (int j = 0; j < largo2; j++)
    {
        ret[largo1 + j] = src2[j];
    }
    ret[largo1 + largo2] = '\0';

    // libero la memoria de los strings originales
    free(src1);
    free(src2);
    return ret;
}

int casos_test(GameBoard *board)
{
    printf("Duplicate vacío: '%s'\n", strDuplicate(""));
    printf("Duplicate A: '%s'\n", strDuplicate("A"));
    printf("Duplicate largo: '%s'\n", strDuplicate("ZXCVBNMASDFGHJKLQWERTYUIOPzxcvbnmasdfghjklqwertyuiop123456789"));
    printf("Duplicate solo numeros: '%s'\n", strDuplicate("291206"));

    printf("Compare iguales: %d\n", strCompare("AAAA", "AAAA"));
    printf("Compare iguales pero de distinta longitud: %d\n", strCompare("AAAAA", "AAAA"));
    printf("Compare iguales pero de distinta longitud: %d\n", strCompare("AAAA", "AAAAA"));
    printf("Compare vacíos: %d\n", strCompare("", ""));
    printf("Compare vacíos con no vacios: %d\n", strCompare("", "A"));
    printf("Compare vacíos con no vacios: %d\n", strCompare("A", ""));
    printf("Compare dos de un solo caracter: %d\n", strCompare("A", "B"));
    printf("Compare dos de un solo caracter: %d\n", strCompare("B", "A"));
    printf("Compare iguales hasta un caracter: %d\n", strCompare("Pedro", "Pedra"));
    printf("Compare iguales hasta un caracter: %d\n", strCompare("Pedra", "Pedro"));
    printf("Compare strings diferentes donde el mas largo va antes: %d\n", strCompare("Bautista", "Loisi"));
    printf("Compare strings diferentes donde el mas largo va antes: %d\n", strCompare("Loisi", "Bautista"));
    printf("Compare strings diferentes donde el mas corto va antes: %d\n", strCompare("Gonz", "Pedritooooo"));
    printf("Compare strings diferentes donde el mas corto va antes: %d\n", strCompare("Pedritoooo", "Gonz"));

    printf("Concat vacio y un string de 3 caracteres: '%s'\n", strConcatenate(strDuplicate(""), strDuplicate("ANA")));
    printf("Concat 3 caracteres y un string vacio: '%s'\n", strConcatenate(strDuplicate("ANA"), strDuplicate("")));
    printf("Concat dos vacíos: '%s'\n", strConcatenate(strDuplicate(""), strDuplicate("")));
    printf("Concat dos strings de 1 caracter: '%s'\n", strConcatenate(strDuplicate("P"), strDuplicate("A")));
    printf("Concat dos strings de 5 caracteres: '%s'\n", strConcatenate(strDuplicate("Pedro"), strDuplicate("Bauti")));
    printf("Concat dos strings de 5 y 9 caracteres: '%s'\n", strConcatenate(strDuplicate("Pedro"), strDuplicate("Bauticapo")));
    printf("Concat dos strings de 9 y 5 caracteres: '%s'\n", strConcatenate(strDuplicate("Pedrobobo"), strDuplicate("Bauti")));

    gameBoardAddPlant(board, 1, 0); // planta al principio
    gameBoardAddPlant(board, 1, 8); // al final
    gameBoardAddPlant(board, 1, 4); // al medio
    for (int i = 0; i <= 9; i++)
    {
        gameBoardAddPlant(board, 2, i); // lleno de plantas una row
    }
    gameBoardAddPlant(board, 3, 4); // pongo planta
    gameBoardAddPlant(board, 3, 4); // pongo planta en lugar ocupado

    gameBoardAddPlant(board, 4, 3);    // pongo planta
    gameBoardAddPlant(board, 4, 4);    // pongo planta
    gameBoardAddPlant(board, 4, 5);    // pongo planta
    gameBoardRemovePlant(board, 4, 4); // borro el 4
    gameBoardRemovePlant(board, 4, 3); // borro el 3
    for (int i = 0; i <= 9; i++)
    {
        gameBoardAddPlant(board, 0, i); // lleno de plantas una row
    }
    gameBoardRemovePlant(board, 0, 0); // borro el 0

    for (int i = 0; i < 3; i++)
    {
        gameBoardAddZombie(board, 0); // agrego 3 zombies a una fila
    }
    gameBoardAddZombie(board, 0); // Agrego un zombie adicional a la fila 0

    for (int i = 0; i < 10000; i++)
    {
        gameBoardAddZombie(board, 3); // agrego 3 zombies a una fila
    }
    return 0;
}

int main(int argc, char *args[])
{
    srand(time(NULL));
    if (!inicializar())
        return 1;

    game_board = gameBoardNew();
    casos_test(game_board);

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
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    gameBoardAddPlant(game_board, cursor.row, cursor.col);
                }
                else if (e.button.button == SDL_BUTTON_RIGHT)
                {
                    gameBoardRemovePlant(game_board, cursor.row, cursor.col);
                }
            }
        }

        gameBoardUpdate(game_board);
        gameBoardDraw(game_board);

        // TODO: Agregar la lógica para ver si un zombie llegó a la casa y terminó el juego

        for (int i = 0; i < GRID_ROWS; i++)
        {
            ZombieNode *zombieNode = game_board->rows[i].first_zombie;
            while (zombieNode)
            {
                if (zombieNode->zombie_data.activo && zombieNode->zombie_data.rect.x < GRID_OFFSET_X - zombieNode->zombie_data.rect.w)
                {
                    printf("GAME OVER - Un zombie llego a tu casa!\n");
                    game_over = 1;
                    break;
                }
                zombieNode = zombieNode->next;
            }
            if (game_over)
                break;
        }
        SDL_Delay(16);
    }

    gameBoardDelete(game_board);
    cerrar();

    return 0;
}


// Indique el porcentaje aproximado de lineas de codigo del trabajo practico que fueron realizadas con asistencia de una IA.
        // El 10% de las lineas de codigo fueron realizadas con ayuda de la IA, principalmente en las partes relacionadas con SDL. Y al principio del tp nos ayudo a entender la logica del juego base para poder reimplementar las funciones con las nuevas estructuras de datos.
// ¿Como verificaron que las sugerencias de la IA eran correctas?
        // Como no tenemos idea de como funciona esta libreria, intentabamos correr el codigo para verificar si habia errores. En caso de que los hubiera, volviamos a pedirle a la IA que nos ayudara a corregirlos.
// ¿Se enfrentaron a alguna dificultad al utilizar las herramientas de IA? ¿Como las resolvieron?
        // No hubo muchas dificultades, ya que no utilizamos casi la IA para la resolucion de los ejercicios, sino cosas especificas de SDL y logica del juego.
// ¿Consideran que el uso de la IA les ha permitido desarrollar habilidades de programacion en C? ¿Por que?
        // Si, ya que al utilizar la IA para entender ciertas partes del codigo, pudimos entender mejor como funcionaba la logica del juego y como implementarla en C.