#include "Snake.h"                                  //Header con tutte le include del caso

#define NROW 21                                     //Numero delle Righe
#define NCOL 30                                     //Numero delle Colonne
#define NPIXEL 30                                   //Numero dei pixel per ogni blocco
#define INITICK 110                                 //Numero iniziale di millisecondi per frame
#define MIDDLETICK 70                               //Numero intermedio di tick, da cui successivamente viene sottratto LSTEPTICK e non pi� HSTEPTICK
#define ENDTICK 50                                  //Numero di millisecondi minimi per ogni frame (difficolt� massima)
#define HSTEPTICK 2                                 //Numero di millisecondi che si sottraggono inizialmente da tick ad ogni mela mangiata
#define LSTEPTICK 1                                 //Numero di millisecondi che si sottraggono successivamente da tick ad ogni mela mangiata

#define HIGHSCORE_FILE "HS.txt"
#define HS_BUFF_SIZE 10



enum direzioni {                                    //enum per assegnare alle direzioni un numero
    UP,
    RIGHT,
    DOWN,
    LEFT,
} dir;

struct snake {                                      //struttura che contiene tutti i dati "globali" che sfrutto passando alle varie funzioni il puntatore ad essa, per essere pi� efficente e non dover creare una copia dei dati ad ogni chiamata 
    char **grid;                                        //Matrice di puntatori a valori, che stanno ad indicare se a data coordinata c'� il serpente(1), una mela(2), o il vuoto(0)
    SDL_Rect **rectGrid;                                //Matrice di puntatori a Rettangoli, che formano la griglia di blocchi visibili
    SDL_Surface* surf;
    nod* lastMoves;                                     //Queue contenente le ultime mosse, viene usata per aggiornare la posizione della coda
    nod* nextMoves;                                     //Queue contenente le prossime mosse, messe in input dalla tastiera
    char lastMove;                                      //Ultima direzione presa dal serpente
    char lastInput;                                     //Ultima direzione messa in input
    int head[2];                                        //Tupla contenente la posizione della testa
    int tail[2];                                        //Tupla contenente la posizione della cosa
    int len;                                            //Lunghezza del serpente
    Uint32 backC;                                      //Colore dello sfondo
    Uint32 snakeC;                                      //Colore del serpente
    Uint32 appleC;                                      //Colore della mela
    int tick;
};

SDL_Window* initWindow(void) {
    SDL_Window* window = NULL;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Video Initialization Error: %s", SDL_GetError());
        return NULL;
    }
    window = SDL_CreateWindow("Snake", NPIXEL*NCOL, NPIXEL*NROW, SDL_WINDOW_OPENGL);
    if (window == NULL) {
        printf("Window Creation Error: %s", SDL_GetError());
        return NULL;
    }
    return window;
}

void color(struct snake *snk, int r, int c, Uint32 color) {
    if(!SDL_FillSurfaceRect(snk->surf, &snk->rectGrid[r][c], color)) printf("Coloring Error on row: %d and col: %d\n",r,c);
}

void blank(struct snake* snk) {
    for (int r = 0; r < NROW; r++) {
        for (int c = 0; c < NCOL; c++) {
            color(snk, r, c, snk->backC);
        }
    }
}

void initGrid(struct snake *snk) {                                      //inizializza grid con tutti 0, e rectGrid con quadrati tutti neri
    snk->grid = (char **)calloc(NROW, sizeof(char*));
    snk->rectGrid = (SDL_Rect **)malloc(NROW*sizeof(SDL_Rect*));
    for (int r = 0; r < NROW; r++) {                               
        snk->grid[r] = (char *) calloc(NCOL, sizeof(char));
        snk->rectGrid[r] = (SDL_Rect *) malloc(NCOL*sizeof(SDL_Rect));
    }
    for (int r = 0; r < NROW; r++) {
        for (int c = 0; c < NCOL; c++) {
            snk->rectGrid[r][c] = { c * NPIXEL, r * NPIXEL, NPIXEL, NPIXEL };
        }
    }
}

void initSnake(struct snake* snk, SDL_Surface *surf) {
    snk->lastMoves = initList();                                        //inizializza la lista delle ultime mosse
    snk->nextMoves = initList();                                        //inizializza la lista delle prossime mosse
    snk->lastMove = RIGHT;                                              //inizializza la variabile dell' ultima mossa
    snk->lastInput = RIGHT;                                             //Inizializza lastInput alla direzione RIGHT
    snk->head[0] = 2;                                                   //inizializza la coordinata riga della testa
    snk->head[1] = 4;                                                   //inizializza la coordinata colonna della testa
    snk->tail[0] = 2;                                                   //inizializza la coordinata riga della coda
    snk->tail[1] = 2;                                                   //inizializza la coordinata colonna della coda
    snk->len = 3;                                                       //inizializza la lunghezza del serpente a 3
    snk->backC = SDL_MapRGB(SDL_GetPixelFormatDetails(surf->format), NULL, 50, 50, 50);
    snk->snakeC = SDL_MapRGB(SDL_GetPixelFormatDetails(surf->format), NULL, 255, 255, 255);
    snk->appleC = SDL_MapRGB(SDL_GetPixelFormatDetails(surf->format), NULL, 255, 0, 0);
    snk->surf = surf;
    for (int i = 0; i < 3; i++) {                                       //for(i=0;i<3;i++)
        snk->grid[2][2 + i] = 1;                                            //piazza i primi blocchi del serpente
        color(snk, 2, 2 + i, snk->snakeC);
        if (i) appendRight(snk->lastMoves, RIGHT);                          //inserisce nella queue delle ultime mosse le 2 direzioni giuste
    }
    blank(snk);
}

void newApple(struct snake* snk) {                                      //Genera una nuova mela in una posizione casuale
    int success = 0;
    while (success == 0) {
        int r = rand() % NROW;
        int c = rand() % NCOL;
        if (snk->grid[r][c] == 0) {
            snk->grid[r][c] = 2;
            color(snk, r, c, snk->appleC);
            success = 1;
        }
    }
}

void takeInput(struct snake* snk, SDL_Event *ev) {
    switch (ev->key.key) {
        case(SDLK_LEFT):
        case(SDLK_A):
            if (snk->lastInput == LEFT)break;
            //puts("Sinistra");
            appendRight(snk->nextMoves, LEFT);
            snk->lastInput = LEFT;
            break;
        case(SDLK_DOWN):
        case(SDLK_S):
            if (snk->lastInput == DOWN)break;
            //puts("Giu");
            appendRight(snk->nextMoves, DOWN);
            snk->lastInput = DOWN;
            break;
        case(SDLK_RIGHT):
        case(SDLK_D):
            if (snk->lastInput == RIGHT)break;
            //puts("Destra");
            appendRight(snk->nextMoves, RIGHT);
            snk->lastInput = RIGHT;
            break;
        case(SDLK_UP):
        case(SDLK_W):
            if (snk->lastInput == UP)break;
            //puts("Su");
            appendRight(snk->nextMoves, UP);
            snk->lastInput = UP;
            break;
        default:
            puts("Il tasto selezionato non ha alcuna funzione, puoi moverti con WASD o le freccette.");
    }
}

char move(struct snake* snk) {                       //Muove nella prossima posizione il serpente, ritorna 0 se ha avuto successo, 1 se � game over, 2 se c'� stato un errore
    if (snk->nextMoves->next != snk->nextMoves) {           //se ci sonno mosse nella Queue nextMoves
        snk->lastMove = popLeft(snk->nextMoves);            //imposta lastMove alla prossima mossa
    }
    int nextR;                                          //dichiara un intero per la nuova riga
    int nextC;                                          //dichiara un intero per la nuova colonna
    switch (snk->lastMove) {                              //A seconda della direzione in lastMove vai ad uno dei casi
    case UP:                                            //caso UP
        nextR = snk->head[0] - 1;                             //nextR = coordinata riga della testa - 1 
        nextC = snk->head[1];                               //nextC = coordinata colonna della testa
        if (nextR >= NROW || nextR < 0) return 1;                //se la coordinata � al di fuori della griglia ritorna 1
        appendRight(snk->lastMoves, UP);                    //aggiungi la mossa alla Queue lastMoves
        break;                                              //break per uscire dallo switch
    case RIGHT:                                         //caso RIGHT
        nextR = snk->head[0];                               //nextR = coordinata riga della testa
        nextC = snk->head[1] + 1;                             //nextC = coordinata colonna della testa + 1
        if (nextC >= NCOL || nextC < 0) return 1;                //se la coordinata � al di fuori della griglia ritorna 1
        appendRight(snk->lastMoves, RIGHT);                 //aggiungi la mossa alla Queue lastMoves
        break;                                              //break per uscire dallo switch
    case DOWN:                                          //caso DOWN
        nextR = snk->head[0] + 1;                             //nextR = coordinata riga della testa + 1
        nextC = snk->head[1];                               //nextC = coordinata colonna della testa
        if (nextR >= NROW || nextR < 0) return 1;                //se la coordinata � al di fuori della griglia ritorna 1
        appendRight(snk->lastMoves, DOWN);                  //aggiungi la mossa alla Queue lastMoves
        break;                                              //break per uscire dallo switch
    case LEFT:                                          //caso LEFT
        nextR = snk->head[0];                               //nextR = coordinata riga della testa
        nextC = snk->head[1] - 1;                             //nextC = coordinata colonna della testa - 1
        if (nextC >= NCOL || nextC < 0) return 1;                //se la coordinata � al di fuori della griglia ritorna 1
        appendRight(snk->lastMoves, LEFT);                  //aggiungi la mossa alla Queue lastMoves
        break;                                              //break per uscire dallo switch
    default:                                            //caso default = Errore
        puts("Direction Error (head)");                     //stampa il tipo di errore
        return 2;                                           //ritorna 2
    }
    char nextType = snk->grid[nextR][nextC];            //assegna alla variabile nextType il tipo(0,1,2) della nuova coordinata
    if (nextType == 1) {                                    //se il tipo del prossimo blocco � 1:
        if (nextR == snk->tail[0] && nextC == snk->tail[1]) nextType = 0;    //se il prossimo blocco � la coda: imposta nextType a 0
        else return 1;                                                  //senn� ritorna 1
    }
    snk->head[0] = nextR;                                 //imposta la coordinata riga della testa alla prossima
    snk->head[1] = nextC;                                 //imposta la coordinata colonna della testa alla prossima
    snk->grid[nextR][nextC] = 1;                        //imposta il blocco successivo ad 1
    if (nextType == 0) {                                    //se il tipo del prossimo blocco � 0:
        snk->grid[snk->tail[0]][snk->tail[1]] = 0;          //imposta il blocco attuale della coda a 0
        color(snk, snk->tail[0], snk->tail[1], snk->backC);
        switch (popLeft(snk->lastMoves)) {                    //a seconda della direzione che dovrebbe prendere la coda (presa dalla queue) vai in uno dei casi                                           //caso
        case UP:                                            //caso UP
            snk->tail[0]--;                                     //sottrai alla coordinata della riga della coda 1
            break;                                              //break per uscire dallo switch
        case RIGHT:                                         //caso RIGHT
            snk->tail[1]++;                                     //aggiungi alla coordinata della colonna della coda 1
            break;                                              //break per uscire dallo switch
        case DOWN:                                          //caso DOWN
            snk->tail[0]++;                                     //aggiungi alla coordinata della riga della coda 1
            break;                                              //break per uscire dallo switch
        case LEFT:                                          //caso LEFT
            snk->tail[1]--;                                     //sottrai alla coordinata della colonna della coda 1
            break;                                              //break per uscire dallo switch
        default:                                            //caso default = Errore
            puts("Direction Error (tail)");                     //stampa il tipo di errore
            return 2;                                           //ritorna 2
        }
    }
    else if (nextType != 2) {                               //se il tipo dell prossimo blocco � diverso da 2 c'� stato un errore
        puts("Grid Type Error");                            //stampa il tipo di errore
        return 2;                                           //ritorna 2
    }
    else {                                               //senn�(nextType==2):
        newApple(snk);                                      //genera una nuova mela
        if(snk->tick>ENDTICK) snk->tick -= snk->tick>MIDDLETICK?HSTEPTICK:LSTEPTICK;
        snk->len++;                                         //aggiungi 1 alla lunghezza del serpente
    }
    color(snk, nextR, nextC, snk->snakeC);
    return 0;                                           //ritorna 0
}

char saveHS(int s) {
    FILE *f = NULL;
    void *buff = calloc(HS_BUFF_SIZE, sizeof(char));

    if (!(f = fopen(HIGHSCORE_FILE, "r"))){
        puts("Error opening in reading mode file");
        return -1;
    }
    if (fread(buff, sizeof(char), HS_BUFF_SIZE, f)<=0) {
        puts("Error reading file");
        return -1;
    }
    //printf("%s\n", buff);
    int hs = atoi((char*)buff);
    if (hs >= s)return 0;
    else{
        //buff = (void*)std::to_string(42);  //-------------------------------------------------------------------aaaaaaaaaaaaaaaaaa devi fare una funzione incasina decasina
        fclose(f);
        f = NULL;
        if (!(f = fopen(HIGHSCORE_FILE, "w"))){
            puts("Error opening file in writing mode");
            return -1;
        }
        if (fwrite(buff, 1, HS_BUFF_SIZE, f) <= 0) {
            puts("Error writing new highscore");
            return -1;
        }
        fclose(f);
        return 1;
    }

}

int main(int argc, char* argv[]) {
    struct snake* snk = (struct snake*) malloc(sizeof(struct snake));   //alloca lo spazio necessario alla struttura snake, e salva in snk il puntatore ad esso
    SDL_Window *win = initWindow();
    if(!win) return 1;
    SDL_Surface* surf = NULL;
    surf = SDL_GetWindowSurface(win);
    initGrid(snk);                                                      //inizializza grid e rectGrid
    initSnake(snk,surf);                                                     //inizializza la struttura snake
    srand(time(0));                                                     //inizializza la funzione di generazione pseudo-randomica dei numeri all'ora attuale
    newApple(snk);                                                      //genera la prima mela
    snk->tick = INITICK;

    SDL_Event ev;
    char running=1;
    SDL_Delay(snk->tick);
    while (running) {
        while (SDL_PollEvent(&ev) != 0) {
            if (ev.type == SDL_EVENT_KEY_DOWN) {
                takeInput(snk, &ev);
            }
            else if (ev.type == SDL_EVENT_QUIT) running = 0;
        }
        
        if (move(snk)) running = 0;
        SDL_UpdateWindowSurface(win);
        SDL_Delay(snk->tick);
    }

    char hs = saveHS(snk->len - 3);
    hs==1?puts("---\tNew Highscore\t---"):hs==-1?puts("Error setting new highscore"):puts("");
    //if (hs == 1) puts("---\tNew Highscore\t---");
    //else if (hs == -1) puts("Error setting new highscore");
    printf("\nScore:\t%d\nTick\t%d\n", snk->len - 3, snk->tick);             //stampa il punteggio
    SDL_Delay(3000);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

