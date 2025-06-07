#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <errno.h> 
#include <cstdlib>
#include <SDL.h>
#include "list.h"

#include <string> 

char** initGrid(int nRow, int nCol);
void frame(char** grid);
//char move(struct snake *snk);
