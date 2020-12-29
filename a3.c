//-----------------------------------------------------------------------------
// a3.c
//
// TODO
//
// Group: 9
//
// Author: 12007662
//-----------------------------------------------------------------------------
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "framework.h"

//constants

//strings

//char

//enum
typedef enum _Direction_
{
  TOP = 0,
  LEFT = 1,
  BOTTOM = 2,
  RIGHT = 3
} Direction;

// TODO Highscore Datentyp

// forward declarations
bool isDirectionOutOfMap(uint8_t width, uint8_t height, uint8_t coord[2], Direction dir);

bool isPipeOpenInDirection(uint8_t **map, uint8_t width, uint8_t height, uint8_t coord[2], Direction dir);

bool shouldPipeConnectInDirection(uint8_t **map, uint8_t width, uint8_t height, uint8_t coord[2], Direction dir);

uint8_t *getAdjacentPipe(uint8_t **map, uint8_t width, uint8_t height, uint8_t coord[2], Direction dir);


//-----------------------------------------------------------------------------
///
/// The main program:
/// TODO
/// int argc, char *argv[]
//
int main()
{

  return 0;
}
