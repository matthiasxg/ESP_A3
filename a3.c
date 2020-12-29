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
#define COMMANDLINE_PARAMETER 1

#define SIZE_MAGIC_NUMBER 7

//strings
#define MAGIC_NUMBER "ESPipes"

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

bool checkConfigFile(char *path);


//-----------------------------------------------------------------------------
///
/// The main program:
/// TODO
///
//
int main(int argc, char *argv[])
{
  if (--argc == COMMANDLINE_PARAMETER)
  {
    char *path = argv[1];
    if (checkConfigFile(path)) {
      printf("Valid Config");
    }
  }
  else
  {
    printf(USAGE_APPLICATION);
  }
  return 0;
}

// TODO
bool checkConfigFile(char *path)
{
  FILE *fp = fopen(path, "rb");
  if (fp)
  {
    char *magic_number = malloc(sizeof(char) * SIZE_MAGIC_NUMBER);
    if (magic_number) {
      fread(magic_number, sizeof(char), SIZE_MAGIC_NUMBER, fp);
      if(strcmp(magic_number, MAGIC_NUMBER) != 0) {
        printf(ERROR_INVALID_FILE, path);
        free(magic_number);
        fclose(fp);
        return false;
      }
      free(magic_number);
    }
    else
    {
      printf(ERROR_OUT_OF_MEMORY);
      fclose(fp);
      return false;
    }
  }
  else
  {
    printf(ERROR_OPEN_FILE, path);
    return false;
  }
  fclose(fp);
  return true;
}
